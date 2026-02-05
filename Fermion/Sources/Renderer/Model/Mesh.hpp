#pragma once
#include "fmpch.hpp"
#include "Asset/Asset.hpp"
#include "Math/AABB.hpp"
#include "Animation/Skeleton.hpp"
#include "Animation/AnimationClip.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace Fermion
{
    class VertexArray;
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec4 Color = glm::vec4(1.0f);
        glm::vec2 TexCoord = glm::vec2(0.0f);
    };

    struct VertexBoneData
    {
        glm::ivec4 BoneIDs = glm::ivec4(-1);
        glm::vec4 BoneWeights = glm::vec4(0.0f);
    };

    struct SubMesh
    {
        uint32_t MaterialSlotIndex = 0; // 材质槽位索引
        uint32_t IndexOffset = 0;
        uint32_t IndexCount = 0;
    };

    enum MemoryMeshType : uint16_t
    {
        None = 0,
        Cube = 1,
        Sphere = 2,
        Cylinder = 3,
        Capsule = 4,
        Cone = 5
    };

    struct MemoryMeshKey
    {
        MemoryMeshType Type;
        glm::vec3 Size;
        float Radius;
        float Height;
    };

    class Mesh : public Asset
    {
    public:
        Mesh(const std::string &path) : m_ModelPath(path)
        {
            loadMesh(path);
            setupMesh();
            calculateBoundingBox();
        }

        Mesh(std::vector<Vertex> vertices,
             std::vector<uint32_t> indices,
             std::vector<SubMesh> subMeshes = {});

        ~Mesh() = default;

        std::shared_ptr<VertexArray> getVertexArray() const;

        const std::vector<Vertex> &getVertices() const
        {
            return m_vertices;
        }

        const std::vector<uint32_t> &getIndices() const
        {
            return m_indices;
        }

        const std::vector<SubMesh> &getSubMeshes() const
        {
            return m_SubMeshes;
        }

        const std::string &getPath() const
        {
            return m_ModelPath;
        }
        void debugMeshLog() const;

        void calculateBoundingBox();
        const AABB &getBoundingBox() const
        {
            return m_BoundingBox;
        }

        bool isSkinned() const { return m_isSkinned; }

        void setBoneData(std::vector<VertexBoneData> boneData)
        {
            m_boneData = std::move(boneData);
            m_isSkinned = !m_boneData.empty();
            // Re-setup mesh to include bone data in VAO
            if (m_isSkinned)
            {
                setupMesh();
            }
        }

        const std::vector<VertexBoneData> &getBoneData() const
        {
            return m_boneData;
        }

        std::shared_ptr<Skeleton> getSkeleton() const
        {
            return m_skeleton;
        }

        const std::vector<std::shared_ptr<AnimationClip>> &getAnimations() const
        {
            return m_animations;
        }

    private:
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        std::vector<SubMesh> m_SubMeshes;

        std::string m_ModelPath;
        AABB m_BoundingBox;

        std::shared_ptr<VertexArray> m_VAO = nullptr;

        // Skinning data
        bool m_isSkinned = false;
        std::vector<VertexBoneData> m_boneData;
        std::shared_ptr<Skeleton> m_skeleton;
        std::vector<std::shared_ptr<AnimationClip>> m_animations;

        void loadMesh(const std::string &path);

        void processNode(aiNode *node, const aiScene *scene);
        void processMesh(aiMesh *mesh, const aiScene *scene);

        void processBones(aiMesh *mesh, uint32_t vertexStart);
        void processAnimations(const aiScene *scene);

        void buildSkeletonHierarchy(const aiScene *scene);

        void setupMesh();
    };
} // namespace Fermion
