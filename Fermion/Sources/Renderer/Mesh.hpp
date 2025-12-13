#pragma once
#include "fmpch.hpp"
#include "Buffer.hpp"
#include "VertexArray.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Texture.hpp"
#include "Asset/Asset.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Fermion
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec4 Color = glm::vec4(1.0f);
        glm::vec2 TexCoord = glm::vec2(0.0f);
    };

    struct SubMesh
    {
        uint32_t MaterialIndex = 0;
        uint32_t IndexOffset = 0;
        uint32_t IndexCount = 0;
    };

    class Mesh : public Asset
    {
    public:
        Mesh(const std::string &path)
        {
            loadMesh(path);
            setupMesh();
        }

        ~Mesh() = default;

        std::shared_ptr<VertexArray> getVertexArray() const { return m_VAO; }
        const std::vector<Vertex> &getVertices() const { return m_vertices; }
        const std::vector<uint32_t> &getIndices() const { return m_indices; }
        const std::vector<std::shared_ptr<Material>> &getMaterials() const { return m_Materials; }
        const std::vector<SubMesh> &getSubMeshes() const { return m_SubMeshes; }
        const std::string& getPath() const {return m_ModelPath;}
    private:
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        std::vector<std::shared_ptr<Material>> m_Materials;
        std::vector<SubMesh> m_SubMeshes;
        std::string m_ModelPath;

        std::shared_ptr<VertexArray> m_VAO = nullptr;

        void loadMesh(const std::string &path);
        void processNode(aiNode *node, const aiScene *scene);
        void processMesh(aiMesh *mesh, const aiScene *scene);
        void setupMesh();
    };

    
}
