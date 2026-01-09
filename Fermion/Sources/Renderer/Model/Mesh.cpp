#include "Mesh.hpp"

namespace Fermion {
    Mesh::Mesh(std::vector<Vertex> vertices,
               std::vector<uint32_t> indices,
               std::vector<SubMesh> subMeshes) : m_vertices(std::move(vertices)),
                                                 m_indices(std::move(indices)),
                                                 m_SubMeshes(std::move(subMeshes)) {
        m_ModelPath = "<MemoryMesh>";

        if (m_SubMeshes.empty() && !m_indices.empty()) {
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

        // 默认顶点颜色
        glm::vec4 defaultColor(1.0f);

        // 顶点
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
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

        // 索引
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace &face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                m_indices.push_back(face.mIndices[j] + vertexStart);
        }

        // 子网格 - 只存储材质槽位索引
        SubMesh submesh;
        submesh.MaterialSlotIndex = mesh->mMaterialIndex;  // 使用assimp的材质索引作为槽位索引
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
