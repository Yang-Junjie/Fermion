#include "ModelSourceImporter.hpp"
#include "Core/UUID.hpp"
#include "Asset/AssetManager.hpp"
#include "Asset/AssetSerializer.hpp"
#include "Renderer/Model/MaterialSerializer.hpp"
#include "Renderer/Model/MeshSerializer.hpp"
#include "Renderer/Model/ModelSerializer.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <fstream>
#include <optional>
#include <string_view>

namespace Fermion
{
    namespace
    {
        std::filesystem::path make_output_path(const std::filesystem::path &sourcePath,
                                              const std::string &suffix,
                                              const std::string &extension)
        {
            auto out = sourcePath.parent_path() / (sourcePath.stem().string() + suffix);
            out.replace_extension(extension);
            return out;
        }

        glm::vec4 aiColorToVec4(const aiColor4D &c)
        {
            return {c.r, c.g, c.b, c.a};
        }

        glm::vec3 aiColorToVec3(const aiColor4D &c)
        {
            return {c.r, c.g, c.b};
        }

        void process_mesh(aiMesh *mesh, const aiScene *scene, std::vector<Vertex> &vertices,
                          std::vector<uint32_t> &indices, std::vector<SubMesh> &subMeshes,
                          const std::string &meshName)
        {
            if (!mesh)
                return;

            const uint32_t vertexStart = static_cast<uint32_t>(vertices.size());
            const uint32_t indexStart = static_cast<uint32_t>(indices.size());

            glm::vec4 defaultColor(1.0f);

            vertices.reserve(vertices.size() + mesh->mNumVertices);
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
            {
                Vertex vertex{};
                vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
                vertex.Normal = mesh->HasNormals()
                                    ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
                                    : glm::vec3(0.0f, 1.0f, 0.0f);

                if (mesh->HasVertexColors(0))
                {
                    const auto &c = mesh->mColors[0][i];
                    vertex.Color = {c.r, c.g, c.b, c.a};
                }
                else
                {
                    vertex.Color = defaultColor;
                }

                if (mesh->HasTextureCoords(0))
                {
                    vertex.TexCoord = {mesh->mTextureCoords[0][i].x, 1.0f - mesh->mTextureCoords[0][i].y};
                }
                else
                {
                    vertex.TexCoord = {0.0f, 0.0f};
                }

                vertices.push_back(vertex);
            }

            for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
            {
                const aiFace &face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; ++j)
                    indices.push_back(face.mIndices[j] + vertexStart);
            }

            SubMesh submesh;
            submesh.IndexOffset = indexStart;
            submesh.IndexCount = static_cast<uint32_t>(indices.size()) - indexStart;
            
            if (scene && mesh->mMaterialIndex < scene->mNumMaterials)
            {
                submesh.MaterialSlotIndex = mesh->mMaterialIndex;

                aiString matName;
                scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, matName);
                
                Log::Info(std::format("SubMesh [{}]: vertices={}, indices={}, materialSlot={}, materialName='{}'",
                         subMeshes.size(), mesh->mNumVertices, submesh.IndexCount,
                         submesh.MaterialSlotIndex, matName.C_Str()));
            }
            else
            {
                Log::Warn(std::format("SubMesh [{}]: Mesh '{}' has invalid material index {} (max: {}), using slot 0",
                         subMeshes.size(), meshName, mesh->mMaterialIndex,
                         scene ? scene->mNumMaterials : 0));
                submesh.MaterialSlotIndex = 0;
            }
            
            subMeshes.push_back(submesh);
        }

        void process_node(aiNode *node, const aiScene *scene, std::vector<Vertex> &vertices,
                          std::vector<uint32_t> &indices, std::vector<SubMesh> &subMeshes)
        {
            if (!node || !scene)
                return;

            for (unsigned int i = 0; i < node->mNumMeshes; ++i)
            {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                std::string meshName = mesh->mName.C_Str();
                if (meshName.empty())
                    meshName = std::format("mesh_{}", node->mMeshes[i]);
                    
                process_mesh(mesh, scene, vertices, indices, subMeshes, meshName);
            }

            for (unsigned int i = 0; i < node->mNumChildren; ++i)
                process_node(node->mChildren[i], scene, vertices, indices, subMeshes);
        }

        static std::optional<std::filesystem::path> write_embedded_texture_if_needed(
            const std::filesystem::path &modelSourcePath, const aiScene *scene, const aiString &texturePath)
        {
            if (!scene)
                return std::nullopt;

            const std::string_view pathView(texturePath.C_Str());
            if (pathView.empty() || pathView[0] != '*')
                return std::nullopt;

            uint32_t embeddedIndex = 0;
            try
            {
                embeddedIndex = static_cast<uint32_t>(std::stoul(std::string(pathView.substr(1))));
            }
            catch (...)
            {
                Log::Warn(std::format("  Embedded texture has invalid index '{}'", texturePath.C_Str()));
                return std::nullopt;
            }

            if (embeddedIndex >= scene->mNumTextures)
            {
                Log::Warn(std::format("  Embedded texture index {} out of range (max: {})",
                                      embeddedIndex, scene->mNumTextures));
                return std::nullopt;
            }

            const aiTexture *tex = scene->mTextures[embeddedIndex];
            if (!tex)
                return std::nullopt;


            if (tex->mHeight != 0)
            {
                Log::Warn(std::format("  Embedded texture {} is uncompressed ({}x{}), skipping",
                                      embeddedIndex, tex->mWidth, tex->mHeight));
                return std::nullopt;
            }

            std::string ext = "bin";
            if (tex->achFormatHint[0] != '\0')
                ext = tex->achFormatHint;

            auto outPath = modelSourcePath.parent_path() /
                           std::format("{}_embedded_{}.{}", modelSourcePath.stem().string(), embeddedIndex, ext);

            if (!std::filesystem::exists(outPath))
            {
                std::ofstream out(outPath, std::ios::binary);
                if (!out.is_open())
                {
                    Log::Warn(std::format("  Failed to write embedded texture to {}", outPath.string()));
                    return std::nullopt;
                }

                out.write(reinterpret_cast<const char *>(tex->pcData), static_cast<std::streamsize>(tex->mWidth));
            }

            return outPath;
        }

        static std::filesystem::path resolve_texture_path(const std::filesystem::path &modelSourcePath,
                                                         const aiScene *scene,
                                                         const aiString &texturePath)
        {
            if (auto embedded = write_embedded_texture_if_needed(modelSourcePath, scene, texturePath); embedded)
                return *embedded;

            std::filesystem::path p(texturePath.C_Str());
            if (p.empty())
                return {};

            if (p.is_absolute())
                return p;

            return modelSourcePath.parent_path() / p;
        }

        static AssetHandle import_texture_asset(const std::filesystem::path &modelSourcePath,
                                                const aiScene *scene,
                                                const aiString &texturePath)
        {
            const auto resolved = resolve_texture_path(modelSourcePath, scene, texturePath);
            if (resolved.empty())
                return AssetHandle(0);

            if (!std::filesystem::exists(resolved))
            {
                Log::Warn(std::format("  Texture file not found: {}", resolved.string()));
                return AssetHandle(0);
            }

            return AssetManager::importAsset(resolved);
        }

        Material make_material_from_assimp(const std::filesystem::path &modelSourcePath,
                                           const aiScene *scene,
                                           aiMaterial *aiMat,
                                           const std::string &fallbackName)
        {
            Material material;
            material.setMaterialType(MaterialType::PBR);
            material.setName(fallbackName);

            if (!aiMat)
                return material;

            aiString name;
            if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS && name.length > 0)
                material.setName(name.C_Str());

            aiColor4D baseColor;
            bool hasBaseColor = false;
            if (aiGetMaterialColor(aiMat, AI_MATKEY_BASE_COLOR, &baseColor) == AI_SUCCESS)
            {
                material.setAlbedo(aiColorToVec3(baseColor));
                material.setDiffuseColor(aiColorToVec4(baseColor));
                hasBaseColor = true;
                Log::Info(std::format("  BASE_COLOR: [{:.3f}, {:.3f}, {:.3f}]", baseColor.r, baseColor.g, baseColor.b));
            }
            else if (aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &baseColor) == AI_SUCCESS)
            {
                material.setAlbedo(aiColorToVec3(baseColor));
                material.setDiffuseColor(aiColorToVec4(baseColor));
                hasBaseColor = true;
                Log::Info(std::format("  DIFFUSE_COLOR: [{:.3f}, {:.3f}, {:.3f}]", baseColor.r, baseColor.g, baseColor.b));
            }

            aiColor4D ambientColor;
            if (aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_AMBIENT, &ambientColor) == AI_SUCCESS)
            {
                material.setAmbientColor(aiColorToVec4(ambientColor));
                Log::Info(std::format("  AMBIENT_COLOR: [{:.3f}, {:.3f}, {:.3f}]", ambientColor.r, ambientColor.g, ambientColor.b));
            }

            float metallic = 0.0f;
            aiReturn metallicResult = aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
            if (metallicResult == AI_SUCCESS)
            {
                material.setMetallic(metallic);
                Log::Info(std::format("  METALLIC_FACTOR: {:.3f}", metallic));
            }
            else
            {
                Log::Warn(std::format("  METALLIC_FACTOR not found, using default 0.0"));
            }

            float roughness = 0.5f;
            aiReturn roughnessResult = aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
            if (roughnessResult == AI_SUCCESS)
            {
                material.setRoughness(roughness);
                Log::Info(std::format("  ROUGHNESS_FACTOR: {:.3f}", roughness));
            }
            else
            {
                material.setRoughness(0.5f);
                Log::Warn(std::format("  ROUGHNESS_FACTOR not found, using default 0.5"));
            }

            float ao = 1.0f;
            material.setAO(ao);

            // 提取纹理贴图
            aiString texturePath;
            
            if (aiMat->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath) == AI_SUCCESS ||
                aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
            {
                Log::Info(std::format("  Found Albedo texture: {}", texturePath.C_Str()));
                material.setAlbedoMap(import_texture_asset(modelSourcePath, scene, texturePath));
            }
            
            if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS ||
                aiMat->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS)
            {
                Log::Info(std::format("  Found Normal texture: {}", texturePath.C_Str()));
                material.setNormalMap(import_texture_asset(modelSourcePath, scene, texturePath));
            }
            
            if (aiMat->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS)
            {
                Log::Info(std::format("  Found Metallic texture: {}", texturePath.C_Str()));
                material.setMetallicMap(import_texture_asset(modelSourcePath, scene, texturePath));
            }
            
            if (aiMat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texturePath) == AI_SUCCESS)
            {
                Log::Info(std::format("  Found Roughness texture: {}", texturePath.C_Str()));
                material.setRoughnessMap(import_texture_asset(modelSourcePath, scene, texturePath));
            }
            
            if (aiMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath) == AI_SUCCESS ||
                aiMat->GetTexture(aiTextureType_LIGHTMAP, 0, &texturePath) == AI_SUCCESS)
            {
                Log::Info(std::format("  Found AO texture: {}", texturePath.C_Str()));
                material.setAOMap(import_texture_asset(modelSourcePath, scene, texturePath));
            }

            return material;
        }

        AssetHandle make_handle_non_zero()
        {
            AssetHandle handle = UUID();
            if (static_cast<uint64_t>(handle) == 0)
                handle = UUID(1);
            return handle;
        }
    } // namespace

    AssetMetadata ModelSourceImporter::importAsset(const std::filesystem::path &assetPath)
    {
        if (assetPath.empty() || !std::filesystem::exists(assetPath))
        {
            Log::Error("ModelSourceImporter: Invalid asset path");  
            return {};
        }

        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(assetPath.string(),
                                                 aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

        if (!scene || !scene->mRootNode)
        {
            Log::Error(std::format("ModelSourceImporter: Failed to load model from {}: {}",
                      assetPath.string(), importer.GetErrorString()));

            return {};
        }

        const auto meshPath = make_output_path(assetPath, "", ".fmesh");
        const auto modelPath = make_output_path(assetPath, "", ".fmodel");

        Log::Info(std::format("ModelSourceImporter: Processing '{}': {} meshes, {} materials",
                 assetPath.filename().string(), scene->mNumMeshes, scene->mNumMaterials));

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<SubMesh> subMeshes;
        process_node(scene->mRootNode, scene, vertices, indices, subMeshes);

        if (vertices.empty() || indices.empty())
        {
            Log::Warn(std::format("ModelSourceImporter: Model {} has no valid geometry", assetPath.string()));
            return {};
        }
        
        Log::Info(std::format("ModelSourceImporter: Generated {} SubMeshes with {} total vertices, {} indices",
                 subMeshes.size(), vertices.size(), indices.size()));

        AssetMetadata meshMeta;
        meshMeta.Handle = make_handle_non_zero();
        meshMeta.Type = AssetType::Mesh;
        meshMeta.FilePath = meshPath;
        meshMeta.Name = assetPath.stem().string();

        Mesh mesh(std::move(vertices), std::move(indices), std::move(subMeshes));
        mesh.handle = meshMeta.Handle;

        MeshSerializeOptions meshOptions;
        meshOptions.nameOverride = meshMeta.Name;
        if (!MeshSerializer::serialize(meshPath, mesh, meshOptions))
        {
            Log::Error(std::format("ModelSourceImporter: Failed to serialize mesh to {}", meshPath.string()));
            return {};
        }

        writeMetadata(meshMeta);

        std::vector<AssetHandle> materialHandles;
        materialHandles.resize(scene->mNumMaterials, AssetHandle(0));

        MaterialSerializeOptions materialOptions;
        materialOptions.includePBRMaps = true;

        for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
        {
            aiMaterial *aiMat = scene->mMaterials[i];
            const std::string fallbackName = std::string("Material_") + std::to_string(i);

            Material material = make_material_from_assimp(assetPath, scene, aiMat, fallbackName);

            AssetMetadata matMeta;
            matMeta.Handle = make_handle_non_zero();
            matMeta.Type = AssetType::Material;
            matMeta.Name = material.getName().empty() ? fallbackName : material.getName();
            matMeta.FilePath = make_output_path(assetPath, std::string("_slot") + std::to_string(i), ".fmat");

            Log::Info(std::format("Material slot {}: name='{}', file='{}'",
                     i, matMeta.Name, matMeta.FilePath.filename().string()));

            material.handle = matMeta.Handle;
            if (!MaterialSerializer::serialize(matMeta.FilePath, material, materialOptions))
            {
                Log::Error(std::format("ModelSourceImporter: Failed to serialize material slot {} to {}",
                          i, matMeta.FilePath.string()));

                return {};
            }

            writeMetadata(matMeta);
            materialHandles[i] = matMeta.Handle;
        }
        
        Log::Info(std::format("ModelSourceImporter: Created {} material files", materialHandles.size()));

        m_Metadata = ModelSerializer::serialize(modelPath, meshMeta.Handle, materialHandles);
        if (m_Metadata.Type == AssetType::None)
        {
            Log::Error(std::format("ModelSourceImporter: Failed to serialize model to {}", modelPath.string()));
            return {};
        }

        return m_Metadata;
    }
} // namespace Fermion
