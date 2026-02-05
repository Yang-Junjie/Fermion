#include "Mesh.hpp"
#include "../VertexArray.hpp"
#include <assimp/postprocess.h>
namespace Fermion
{
    Mesh::Mesh(std::vector<Vertex> vertices,
               std::vector<uint32_t> indices,
               std::vector<SubMesh> subMeshes) : m_vertices(std::move(vertices)),
                                                 m_indices(std::move(indices)),
                                                 m_SubMeshes(std::move(subMeshes))
    {
        m_ModelPath = "<MemoryMesh>";

        if (m_SubMeshes.empty() && !m_indices.empty())
        {
            SubMesh sub;
            sub.IndexOffset = 0;
            sub.IndexCount = static_cast<uint32_t>(m_indices.size());
            m_SubMeshes.push_back(sub);
        }

        setupMesh();
        calculateBoundingBox();
    }

    void Mesh::debugMeshLog() const
    {
        Log::Info("-------------------Mesh Info----------------------");
        Log::Info("Mesh: " + m_ModelPath);
        for (size_t i = 0; i < m_vertices.size(); i++)
        {
            Log::Info("-------------------Vertex Info----------------------");
            auto &vertex = m_vertices[i];
            Log::Info(std::format("Vertex: {}", i));
            Log::Info(std::format("Position: {},{},{}", vertex.Position.x, vertex.Position.y, vertex.Position.z));
            Log::Info(std::format("Normal: {},{},{}", vertex.Normal.x, vertex.Normal.y, vertex.Normal.z));
            Log::Info(std::format("Color: {},{},{},{}", vertex.Color.r, vertex.Color.g, vertex.Color.b, vertex.Color.a));
            Log::Info(std::format("TexCoord: {},{}", vertex.TexCoord.x, vertex.TexCoord.y));
            Log::Info("-----------------------------------------------------");
        }
        Log::Info("---------------------------------------------------");
    }

    void Mesh::calculateBoundingBox()
    {
        m_BoundingBox.min = {FLT_MAX, FLT_MAX, FLT_MAX};
        m_BoundingBox.max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        for (auto &vertex : m_vertices)
        {
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
        // Note: Do NOT use aiProcess_PreTransformVertices as it removes bone information
        const aiScene *scene = importer.ReadFile(path,
                                                 aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_LimitBoneWeights);

        if (!scene || !scene->mRootNode)
        {
            Log::Error("Failed to load mesh: " + path);
            return;
        }

        // Check if any mesh has bones
        bool hasBones = false;
        for (unsigned int i = 0; i < scene->mNumMeshes; i++)
        {
            if (scene->mMeshes[i]->HasBones())
            {
                hasBones = true;
                break;
            }
        }

        if (hasBones)
        {
            m_isSkinned = true;
            m_skeleton = std::make_shared<Skeleton>();
        }

        processNode(scene->mRootNode, scene);

        if (m_isSkinned)
        {
            buildSkeletonHierarchy(scene);
            processAnimations(scene);

            // Bind all animations to the skeleton
            for (auto &clip : m_animations)
            {
                clip->bindSkeleton(m_skeleton);
            }
        }
    }

    std::shared_ptr<VertexArray> Mesh::getVertexArray() const
    {
        return m_VAO;
    }

    void Mesh::processNode(aiNode *node, const aiScene *scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene);
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    void Mesh::processMesh(aiMesh *mesh, const aiScene *scene)
    {
        if (!mesh)
            return;

        uint32_t vertexStart = (uint32_t)m_vertices.size();
        uint32_t indexStart = (uint32_t)m_indices.size();

        // 默认顶点颜色
        glm::vec4 defaultColor(1.0f);

        // 顶点
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex v;
            v.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            v.Normal = mesh->HasNormals()
                           ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
                           : glm::vec3(0, 1, 0);
            v.Color = defaultColor;
            v.TexCoord = mesh->HasTextureCoords(0)
                             ? glm::vec2(mesh->mTextureCoords[0][i].x, 1.0f - mesh->mTextureCoords[0][i].y)
                             : glm::vec2(0.0f);

            m_vertices.push_back(v);
        }

        // Resize bone data to match vertices if skinned
        if (m_isSkinned)
        {
            m_boneData.resize(m_vertices.size());
        }

        // Process bones for this mesh
        if (mesh->HasBones())
        {
            processBones(mesh, vertexStart);
        }

        // 索引
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            const aiFace &face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                m_indices.push_back(face.mIndices[j] + vertexStart);
        }

        // 子网格 - 只存储材质槽位索引
        SubMesh submesh;
        submesh.MaterialSlotIndex = mesh->mMaterialIndex; // 使用assimp的材质索引作为槽位索引
        submesh.IndexOffset = indexStart;
        submesh.IndexCount = (uint32_t)m_indices.size() - indexStart;
        m_SubMeshes.push_back(submesh);
    }

    void Mesh::processBones(aiMesh *mesh, uint32_t vertexStart)
    {
        if (!m_skeleton)
            return;

        for (unsigned int boneIdx = 0; boneIdx < mesh->mNumBones; boneIdx++)
        {
            aiBone *bone = mesh->mBones[boneIdx];
            std::string boneName = bone->mName.C_Str();

            // Get or create bone in skeleton
            int32_t boneIndex = m_skeleton->findBoneIndex(boneName);
            if (boneIndex < 0)
            {
                // Convert Assimp offset matrix to glm
                const auto &m = bone->mOffsetMatrix;
                glm::mat4 offsetMatrix = glm::mat4(
                    m.a1, m.b1, m.c1, m.d1,
                    m.a2, m.b2, m.c2, m.d2,
                    m.a3, m.b3, m.c3, m.d3,
                    m.a4, m.b4, m.c4, m.d4);

                boneIndex = m_skeleton->addBone(boneName, -1, offsetMatrix);
            }

            if (boneIndex < 0)
                continue;

            // Assign bone weights to vertices
            for (unsigned int weightIdx = 0; weightIdx < bone->mNumWeights; weightIdx++)
            {
                uint32_t vertexId = bone->mWeights[weightIdx].mVertexId + vertexStart;
                float weight = bone->mWeights[weightIdx].mWeight;

                if (vertexId >= m_boneData.size())
                    continue;

                auto &boneData = m_boneData[vertexId];

                // Find an empty slot in the bone data
                for (int i = 0; i < 4; i++)
                {
                    if (boneData.BoneIDs[i] < 0)
                    {
                        boneData.BoneIDs[i] = boneIndex;
                        boneData.BoneWeights[i] = weight;
                        break;
                    }
                }
            }
        }
    }

    void Mesh::buildSkeletonHierarchy(const aiScene *scene)
    {
        if (!m_skeleton || !scene->mRootNode)
            return;

        // Build a map of node names to their parent node names
        std::function<void(aiNode *, int32_t)> resolveParents = [&](aiNode *node, int32_t depth)
        {
            std::string nodeName = node->mName.C_Str();
            int32_t boneIndex = m_skeleton->findBoneIndex(nodeName);

            if (boneIndex >= 0)
            {
                // Find parent bone by walking up the node tree
                aiNode *parent = node->mParent;
                while (parent)
                {
                    std::string parentName = parent->mName.C_Str();
                    int32_t parentIndex = m_skeleton->findBoneIndex(parentName);
                    if (parentIndex >= 0)
                    {
                        auto &bone = m_skeleton->getBone(boneIndex);
                        bone.parentIndex = parentIndex;
                        break;
                    }
                    parent = parent->mParent;
                }

                // Set the local bind pose from the node transformation
                const auto &m = node->mTransformation;
                glm::mat4 localTransform = glm::mat4(
                    m.a1, m.b1, m.c1, m.d1,
                    m.a2, m.b2, m.c2, m.d2,
                    m.a3, m.b3, m.c3, m.d3,
                    m.a4, m.b4, m.c4, m.d4);

                auto &bone = m_skeleton->getBone(boneIndex);
                // Decompose the local transform into TRS
                glm::vec3 pos = glm::vec3(localTransform[3]);
                glm::vec3 scale;
                scale.x = glm::length(glm::vec3(localTransform[0]));
                scale.y = glm::length(glm::vec3(localTransform[1]));
                scale.z = glm::length(glm::vec3(localTransform[2]));

                glm::mat3 rotMat(
                    glm::vec3(localTransform[0]) / scale.x,
                    glm::vec3(localTransform[1]) / scale.y,
                    glm::vec3(localTransform[2]) / scale.z);
                glm::quat rot = glm::quat_cast(rotMat);

                bone.localBindPose.position = pos;
                bone.localBindPose.rotation = rot;
                bone.localBindPose.scale = scale;
            }

            for (unsigned int i = 0; i < node->mNumChildren; i++)
            {
                resolveParents(node->mChildren[i], depth + 1);
            }
        };

        resolveParents(scene->mRootNode, 0);
    }

    void Mesh::processAnimations(const aiScene *scene)
    {
        if (!scene || !m_skeleton)
            return;

        for (unsigned int animIdx = 0; animIdx < scene->mNumAnimations; animIdx++)
        {
            aiAnimation *anim = scene->mAnimations[animIdx];

            float ticksPerSecond = (anim->mTicksPerSecond > 0.0f)
                                       ? static_cast<float>(anim->mTicksPerSecond)
                                       : 25.0f;

            auto clip = std::make_shared<AnimationClip>(
                anim->mName.C_Str(),
                static_cast<float>(anim->mDuration),
                ticksPerSecond);

            for (unsigned int chanIdx = 0; chanIdx < anim->mNumChannels; chanIdx++)
            {
                aiNodeAnim *nodeAnim = anim->mChannels[chanIdx];

                BoneChannel channel;
                channel.boneName = nodeAnim->mNodeName.C_Str();

                // Position keys
                channel.positionKeys.reserve(nodeAnim->mNumPositionKeys);
                for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++)
                {
                    auto &key = nodeAnim->mPositionKeys[k];
                    channel.positionKeys.emplace_back(
                        static_cast<float>(key.mTime),
                        glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z));
                }

                // Rotation keys
                channel.rotationKeys.reserve(nodeAnim->mNumRotationKeys);
                for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; k++)
                {
                    auto &key = nodeAnim->mRotationKeys[k];
                    channel.rotationKeys.emplace_back(
                        static_cast<float>(key.mTime),
                        glm::quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
                }

                // Scale keys
                channel.scaleKeys.reserve(nodeAnim->mNumScalingKeys);
                for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; k++)
                {
                    auto &key = nodeAnim->mScalingKeys[k];
                    channel.scaleKeys.emplace_back(
                        static_cast<float>(key.mTime),
                        glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z));
                }

                clip->addChannel(std::move(channel));
            }

            m_animations.push_back(clip);
        }
    }

    void Mesh::setupMesh()
    {
        m_VAO = VertexArray::create();

        auto vbo = VertexBuffer::create(
            reinterpret_cast<float *>(m_vertices.data()),
            (uint32_t)(m_vertices.size() * sizeof(Vertex)));

        vbo->setLayout({{ShaderDataType::Float3, "a_Position"},
                        {ShaderDataType::Float3, "a_Normal"},
                        {ShaderDataType::Float4, "a_Color"},
                        {ShaderDataType::Float2, "a_TexCoords"}});

        auto ibo = IndexBuffer::create(m_indices.data(), (uint32_t)m_indices.size());

        m_VAO->addVertexBuffer(vbo);

        // Add bone data VBO for skinned meshes
        if (m_isSkinned && !m_boneData.empty())
        {
            auto boneVBO = VertexBuffer::create(
                reinterpret_cast<float *>(m_boneData.data()),
                (uint32_t)(m_boneData.size() * sizeof(VertexBoneData)));

            boneVBO->setLayout({{ShaderDataType::Int4, "a_BoneIDs"},
                                {ShaderDataType::Float4, "a_BoneWeights"}});

            m_VAO->addVertexBuffer(boneVBO);
        }

        m_VAO->setIndexBuffer(ibo);
    }
} // namespace Fermion
