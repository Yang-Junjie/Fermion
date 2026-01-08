#include "Mesh.hpp"

namespace Fermion {
    Mesh::Mesh(std::vector<Vertex> vertices,
               std::vector<uint32_t> indices,
               std::vector<SubMesh> subMeshes) : m_vertices(std::move(vertices)),
                                                 m_indices(std::move(indices)),
                                                 m_SubMeshes(std::move(subMeshes)) {
        m_ModelPath = "<MemoryMesh>";

        if (m_Materials.empty()) {
            auto defaultMaterial = std::make_shared<Material>();
            defaultMaterial->setDiffuseColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
            defaultMaterial->setAmbientColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
            m_Materials.push_back(defaultMaterial);
        }

        if (m_SubMeshes.empty() && !m_indices.empty()) {
            SubMesh sub;
            sub.MaterialIndex = 0;
            sub.IndexOffset = 0;
            sub.IndexCount = static_cast<uint32_t>(m_indices.size());
            m_SubMeshes.push_back(sub);
        }

        setupMesh();
        calculateBoundingBox();
    }

    std::shared_ptr<Material> Mesh::cloneMaterial(uint32_t index) const
    {
        if (index >= m_Materials.size() || !m_Materials[index])
            return std::make_shared<Material>();

        return m_Materials[index]->clone();
    }

    void Mesh::debugMeshLog() const
    {
        Log::Info("-------------------Mesh Info----------------------");
        Log::Info("Mesh: " + m_ModelPath);
        for(size_t i = 0; i < m_vertices.size(); i++){
            Log::Info("-------------------Vertex Info----------------------");
            auto& vertex = m_vertices[i];
            Log::Info(std::format("Vertex: {}",i));
            Log::Info(std::format("Position: {},{},{}",vertex.Position.x,vertex.Position.y,vertex.Position.z));
            Log::Info(std::format("Normal: {},{},{}",vertex.Normal.x,vertex.Normal.y,vertex.Normal.z));
            Log::Info(std::format("Color: {},{},{},{}",vertex.Color.r,vertex.Color.g,vertex.Color.b,vertex.Color.a));
            Log::Info(std::format("TexCoord: {},{}",vertex.TexCoord.x,vertex.TexCoord.y));
            Log::Info("-----------------------------------------------------");
        }
        Log::Info("---------------------------------------------------");
    }

    void Mesh::calculateBoundingBox()
    {
        m_BoundingBox.min = { FLT_MAX, FLT_MAX, FLT_MAX };
		m_BoundingBox.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

        for (auto &vertex : m_vertices) {
            m_BoundingBox.min.x = std::min(m_BoundingBox.min.x, vertex.Position.x);
            m_BoundingBox.min.y = std::min(m_BoundingBox.min.y, vertex.Position.y);
            m_BoundingBox.min.z = std::min(m_BoundingBox.min.z, vertex.Position.z);
            m_BoundingBox.max.x = std::max(m_BoundingBox.max.x, vertex.Position.x);
            m_BoundingBox.max.y = std::max(m_BoundingBox.max.y, vertex.Position.y);
            m_BoundingBox.max.z = std::max(m_BoundingBox.max.z, vertex.Position.z);
        }
    }

    void Mesh::loadMesh(const std::string &path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path,
                                                 aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

        if (!scene || !scene->mRootNode) {
            Log::Error("Failed to load mesh: " + path);
            return;
        }

        processNode(scene->mRootNode, scene);
    }

    void Mesh::processNode(aiNode *node, const aiScene *scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene);
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    void Mesh::processMesh(aiMesh *mesh, const aiScene *scene) {
        if (!mesh)
            return;

        uint32_t vertexStart = (uint32_t) m_vertices.size();
        uint32_t indexStart = (uint32_t) m_indices.size();

        std::shared_ptr<Material> material = std::make_shared<Material>();
        glm::vec4 Kd(1.0f);
        glm::vec4 Ka(0.0f);

        if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials) {
            aiMaterial *aiMat = scene->mMaterials[mesh->mMaterialIndex];

            aiColor4D diffColor(1, 1, 1, 1);
            if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffColor) == AI_SUCCESS)
                Kd = glm::vec4(diffColor.r, diffColor.g, diffColor.b, diffColor.a);

            aiColor4D ambientColor(0, 0, 0, 1);
            if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS)
                Ka = glm::vec4(ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a);

            material->setDiffuseColor(Kd);
            material->setAmbientColor(Ka);

            // 加载贴图
            if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString texPath;
                if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS && texPath.length > 0) {
                    std::filesystem::path modelDir = std::filesystem::path(m_ModelPath).parent_path();
                    std::filesystem::path fullTexPath = modelDir / texPath.C_Str();

                    auto tex = Texture2D::create(fullTexPath.string());
                    if (tex && tex->isLoaded())
                        material->setTexture(std::move(tex));
                }
            }
        }

        m_Materials.push_back(material);

        // 顶点
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex v;
            v.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            v.Normal = mesh->HasNormals()
                           ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
                           : glm::vec3(0, 1, 0);
            v.Color = material->hasTexture() ? glm::vec4(1.0f) : Kd;
            v.TexCoord = mesh->HasTextureCoords(0)
                             ? glm::vec2(mesh->mTextureCoords[0][i].x, 1.0f - mesh->mTextureCoords[0][i].y)
                             : glm::vec2(0.0f);

            m_vertices.push_back(v);
        }

        // 索引
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace &face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                m_indices.push_back(face.mIndices[j] + vertexStart);
        }

        // 子网格
        SubMesh submesh;
        submesh.MaterialIndex = (unsigned int) (m_Materials.size() - 1);
        submesh.IndexOffset = indexStart;
        submesh.IndexCount = (uint32_t) m_indices.size() - indexStart;
        m_SubMeshes.push_back(submesh);
    }

    void Mesh::setupMesh() {
        m_VAO = VertexArray::create();

        auto vbo = VertexBuffer::create(
            reinterpret_cast<float *>(m_vertices.data()),
            (uint32_t) (m_vertices.size() * sizeof(Vertex)));

        vbo->setLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Float2, "a_TexCoords"}
        });

        auto ibo = IndexBuffer::create(m_indices.data(), (uint32_t) m_indices.size());

        m_VAO->addVertexBuffer(vbo);
        m_VAO->setIndexBuffer(ibo);
    }
} // namespace Fermion
