#include "MeshFactory.hpp"
#include "Asset/AssetManager.hpp"
#include "Mesh.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>

namespace Fermion
{
    AssetHandle MeshFactory::CreateBox(const glm::vec3 &size)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        const glm::vec3 h = size * 0.5f;

        // ========== Front (+Z)
        vertices.push_back({{-h.x, -h.y, h.z}, {0, 0, 1}, {1, 1, 1, 1}, {0, 0}});
        vertices.push_back({{h.x, -h.y, h.z}, {0, 0, 1}, {1, 1, 1, 1}, {1, 0}});
        vertices.push_back({{h.x, h.y, h.z}, {0, 0, 1}, {1, 1, 1, 1}, {1, 1}});
        vertices.push_back({{-h.x, h.y, h.z}, {0, 0, 1}, {1, 1, 1, 1}, {0, 1}});

        // ========== Back (-Z)
        vertices.push_back({{h.x, -h.y, -h.z}, {0, 0, -1}, {1, 1, 1, 1}, {0, 0}});
        vertices.push_back({{-h.x, -h.y, -h.z}, {0, 0, -1}, {1, 1, 1, 1}, {1, 0}});
        vertices.push_back({{-h.x, h.y, -h.z}, {0, 0, -1}, {1, 1, 1, 1}, {1, 1}});
        vertices.push_back({{h.x, h.y, -h.z}, {0, 0, -1}, {1, 1, 1, 1}, {0, 1}});

        // ========== Left (-X)
        vertices.push_back({{-h.x, -h.y, -h.z}, {-1, 0, 0}, {1, 1, 1, 1}, {0, 0}});
        vertices.push_back({{-h.x, -h.y, h.z}, {-1, 0, 0}, {1, 1, 1, 1}, {1, 0}});
        vertices.push_back({{-h.x, h.y, h.z}, {-1, 0, 0}, {1, 1, 1, 1}, {1, 1}});
        vertices.push_back({{-h.x, h.y, -h.z}, {-1, 0, 0}, {1, 1, 1, 1}, {0, 1}});

        // ========== Right (+X)
        vertices.push_back({{h.x, -h.y, h.z}, {1, 0, 0}, {1, 1, 1, 1}, {0, 0}});
        vertices.push_back({{h.x, -h.y, -h.z}, {1, 0, 0}, {1, 1, 1, 1}, {1, 0}});
        vertices.push_back({{h.x, h.y, -h.z}, {1, 0, 0}, {1, 1, 1, 1}, {1, 1}});
        vertices.push_back({{h.x, h.y, h.z}, {1, 0, 0}, {1, 1, 1, 1}, {0, 1}});

        // ========== Top (+Y)
        vertices.push_back({{-h.x, h.y, h.z}, {0, 1, 0}, {1, 1, 1, 1}, {0, 0}});
        vertices.push_back({{h.x, h.y, h.z}, {0, 1, 0}, {1, 1, 1, 1}, {1, 0}});
        vertices.push_back({{h.x, h.y, -h.z}, {0, 1, 0}, {1, 1, 1, 1}, {1, 1}});
        vertices.push_back({{-h.x, h.y, -h.z}, {0, 1, 0}, {1, 1, 1, 1}, {0, 1}});

        // ========== Bottom (-Y)
        vertices.push_back({{-h.x, -h.y, -h.z}, {0, -1, 0}, {1, 1, 1, 1}, {0, 0}});
        vertices.push_back({{h.x, -h.y, -h.z}, {0, -1, 0}, {1, 1, 1, 1}, {1, 0}});
        vertices.push_back({{h.x, -h.y, h.z}, {0, -1, 0}, {1, 1, 1, 1}, {1, 1}});
        vertices.push_back({{-h.x, -h.y, h.z}, {0, -1, 0}, {1, 1, 1, 1}, {0, 1}});

        // ===== 索引（6 面 × 2 三角形）
        for (uint32_t i = 0; i < 6; i++)
        {
            uint32_t base = i * 4;
            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);

            indices.push_back(base + 2);
            indices.push_back(base + 3);
            indices.push_back(base + 0);
        }
        SubMesh sub;
        sub.MaterialIndex = 0;
        sub.IndexOffset = 0;
        sub.IndexCount = (uint32_t)indices.size();

        // ===== 构造 Mesh
        auto mesh = std::make_shared<Mesh>(
            std::move(vertices),
            std::move(indices),
            std::vector<SubMesh>{sub});
        return AssetManager::addMemoryOnlyAsset(mesh);
    }

    AssetHandle MeshFactory::CreateSphere(
        float radius,
        uint32_t latitudeSegments,
        uint32_t longitudeSegments)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t y = 0; y <= latitudeSegments; y++)
        {
            float v = (float)y / (float)latitudeSegments;
            float theta = v * glm::pi<float>();

            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            for (uint32_t x = 0; x <= longitudeSegments; x++)
            {
                float u = (float)x / (float)longitudeSegments;
                float phi = u * glm::two_pi<float>();

                float sinPhi = sin(phi);
                float cosPhi = cos(phi);

                glm::vec3 normal = {
                    cosPhi * sinTheta,
                    cosTheta,
                    sinPhi * sinTheta};

                glm::vec3 position = normal * radius;

                vertices.push_back({position,
                                    glm::normalize(normal),
                                    {1, 1, 1, 1},
                                    {u, v}});
            }
        }

        // ===== 索引 =====
        uint32_t stride = longitudeSegments + 1;

        for (uint32_t y = 0; y < latitudeSegments; y++)
        {
            for (uint32_t x = 0; x < longitudeSegments; x++)
            {
                uint32_t i0 = y * stride + x;
                uint32_t i1 = i0 + stride;
                uint32_t i2 = i1 + 1;
                uint32_t i3 = i0 + 1;

                // 维持 CCW 顺序，让法线朝外
                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);

                indices.push_back(i2);
                indices.push_back(i0);
                indices.push_back(i3);
            }
        }

        SubMesh sub;
        sub.MaterialIndex = 0;
        sub.IndexOffset = 0;
        sub.IndexCount = (uint32_t)indices.size();

        auto mesh = std::make_shared<Mesh>(
            std::move(vertices),
            std::move(indices),
            std::vector<SubMesh>{sub});

        return AssetManager::addMemoryOnlyAsset(mesh);
    }

    AssetHandle MeshFactory::CreateCylinder(
        float radius,
        float height,
        uint32_t radialSegments)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float halfH = height * 0.5f;

        // ======================
        // 侧面
        // ======================
        uint32_t sideVertexOffset = 0;

        for (uint32_t i = 0; i <= radialSegments; i++)
        {
            float u = (float)i / (float)radialSegments;
            float angle = u * glm::two_pi<float>();

            float x = cos(angle);
            float z = sin(angle);

            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));

            // bottom
            vertices.push_back({{x * radius, -halfH, z * radius},
                                normal,
                                {1, 1, 1, 1},
                                {u, 0.0f}});

            // top
            vertices.push_back({{x * radius, halfH, z * radius},
                                normal,
                                {1, 1, 1, 1},
                                {u, 1.0f}});
        }

        // side indices
        for (uint32_t i = 0; i < radialSegments; i++)
        {
            uint32_t i0 = sideVertexOffset + i * 2;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + 2;
            uint32_t i3 = i0 + 3;

            // CCW（观察者在外侧）
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i3);
        }

        // ======================
        // 顶盖 (+Y)
        // ======================
        uint32_t topCenterIndex = (uint32_t)vertices.size();
        vertices.push_back({{0.0f, halfH, 0.0f}, {0, 1, 0}});

        uint32_t topRingStart = (uint32_t)vertices.size();

        for (uint32_t i = 0; i <= radialSegments; i++)
        {
            float angle = (float)i / radialSegments * glm::two_pi<float>();
            float x = cos(angle);
            float z = sin(angle);

            vertices.push_back({{x * radius, halfH, z * radius},
                                {0, 1, 0},
                                {1, 1, 1, 1},
                                {x * 0.5f + 0.5f, z * 0.5f + 0.5f}});
        }

        for (uint32_t i = 0; i < radialSegments; i++)
        {
            indices.push_back(topCenterIndex);
            indices.push_back(topRingStart + i + 1);
            indices.push_back(topRingStart + i);
        }

        // ======================
        //  底盖 (-Y)
        // ======================
        uint32_t bottomCenterIndex = (uint32_t)vertices.size();
        vertices.push_back({{0.0f, -halfH, 0.0f}, {0, -1, 0}});

        uint32_t bottomRingStart = (uint32_t)vertices.size();

        for (uint32_t i = 0; i <= radialSegments; i++)
        {
            float angle = (float)i / radialSegments * glm::two_pi<float>();
            float x = cos(angle);
            float z = sin(angle);

            vertices.push_back({{x * radius, -halfH, z * radius},
                                {0, -1, 0},
                                {1, 1, 1, 1},
                                {x * 0.5f + 0.5f, z * 0.5f + 0.5f}});
        }

        for (uint32_t i = 0; i < radialSegments; i++)
        {
            indices.push_back(bottomCenterIndex);
            indices.push_back(bottomRingStart + i);
            indices.push_back(bottomRingStart + i + 1);
        }

        // ======================
        // SubMesh
        // ======================
        SubMesh sub;
        sub.MaterialIndex = 0;
        sub.IndexOffset = 0;
        sub.IndexCount = (uint32_t)indices.size();

        auto mesh = std::make_shared<Mesh>(
            std::move(vertices),
            std::move(indices),
            std::vector<SubMesh>{sub});

        return AssetManager::addMemoryOnlyAsset(mesh);
    }

    AssetHandle MeshFactory::CreateCapsule(
        float radius,
        float height,
        uint32_t radialSegments,
        uint32_t hemisphereSegments)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float halfHeight = height * 0.5f;
        float cylinderHalf = halfHeight - radius;

        uint32_t indexOffset = 0;

        // ======================
        // 圆柱侧面
        // ======================
        uint32_t sideStart = (uint32_t)vertices.size();

        for (uint32_t i = 0; i <= radialSegments; i++)
        {
            float u = (float)i / radialSegments;
            float angle = u * glm::two_pi<float>();

            float x = cos(angle);
            float z = sin(angle);

            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));

            vertices.push_back({{x * radius, -cylinderHalf, z * radius},
                                normal,
                                {1, 1, 1, 1},
                                {u, 0.0f}});

            vertices.push_back({{x * radius, cylinderHalf, z * radius},
                                normal,
                                {1, 1, 1, 1},
                                {u, 1.0f}});
        }

        for (uint32_t i = 0; i < radialSegments; i++)
        {
            uint32_t i0 = sideStart + i * 2;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + 2;
            uint32_t i3 = i0 + 3;

            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i3);
        }

        // ======================
        // 上半球 (+Y)
        // ======================
        uint32_t topStart = (uint32_t)vertices.size();

        for (uint32_t y = 0; y <= hemisphereSegments; y++)
        {
            float v = (float)y / hemisphereSegments;
            float theta = v * glm::half_pi<float>();

            float sinT = sin(theta);
            float cosT = cos(theta);

            for (uint32_t x = 0; x <= radialSegments; x++)
            {
                float u = (float)x / radialSegments;
                float phi = u * glm::two_pi<float>();

                float sinP = sin(phi);
                float cosP = cos(phi);

                glm::vec3 normal = {
                    cosP * sinT,
                    cosT,
                    sinP * sinT};

                glm::vec3 position =
                    normal * radius +
                    glm::vec3(0.0f, cylinderHalf, 0.0f);

                vertices.push_back({position,
                                    glm::normalize(normal),
                                    {1, 1, 1, 1},
                                    {u, v}});
            }
        }

        uint32_t stride = radialSegments + 1;

        for (uint32_t y = 0; y < hemisphereSegments; y++)
        {
            for (uint32_t x = 0; x < radialSegments; x++)
            {
                uint32_t i0 = topStart + y * stride + x;
                uint32_t i1 = i0 + stride;
                uint32_t i2 = i1 + 1;
                uint32_t i3 = i0 + 1;

                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);

                indices.push_back(i2);
                indices.push_back(i0);
                indices.push_back(i3);
            }
        }

        // ======================
        // 下半球 (-Y)
        // ======================
        uint32_t bottomStart = (uint32_t)vertices.size();

        for (uint32_t y = 0; y <= hemisphereSegments; y++)
        {
            float v = (float)y / hemisphereSegments;
            float theta = v * glm::half_pi<float>();

            float sinT = sin(theta);
            float cosT = cos(theta);

            for (uint32_t x = 0; x <= radialSegments; x++)
            {
                float u = (float)x / radialSegments;
                float phi = u * glm::two_pi<float>();

                float sinP = sin(phi);
                float cosP = cos(phi);

                glm::vec3 normal = {
                    cosP * sinT,
                    -cosT,
                    sinP * sinT};

                glm::vec3 position =
                    normal * radius -
                    glm::vec3(0.0f, cylinderHalf, 0.0f);

                vertices.push_back({position,
                                    glm::normalize(normal),
                                    {1, 1, 1, 1},
                                    {u, v}});
            }
        }

        for (uint32_t y = 0; y < hemisphereSegments; y++)
        {
            for (uint32_t x = 0; x < radialSegments; x++)
            {
                uint32_t i0 = bottomStart + y * stride + x;
                uint32_t i1 = i0 + stride;
                uint32_t i2 = i1 + 1;
                uint32_t i3 = i0 + 1;

                // 注意顺序反转，保证 CCW
                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);

                indices.push_back(i2);
                indices.push_back(i3);
                indices.push_back(i0);
            }
        }

        // ======================
        // SubMesh
        // ======================
        SubMesh sub;
        sub.MaterialIndex = 0;
        sub.IndexOffset = 0;
        sub.IndexCount = (uint32_t)indices.size();

        auto mesh = std::make_shared<Mesh>(
            std::move(vertices),
            std::move(indices),
            std::vector<SubMesh>{sub});

        return AssetManager::addMemoryOnlyAsset(mesh);
    }

    AssetHandle MeshFactory::CreateCone(
        float radius,
        float height,
        uint32_t radialSegments)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float halfH = height * 0.5f;

        // ======================
        // 侧面
        // ======================
        uint32_t sideStart = (uint32_t)vertices.size();

        // 顶点
        uint32_t apexIndex = sideStart;
        vertices.push_back({{0.0f, halfH, 0.0f},
                            {0.0f, 1.0f, 0.0f},
                            {1, 1, 1, 1},
                            {0.5f, 1.0f}});

        // 侧面环
        for (uint32_t i = 0; i <= radialSegments; i++)
        {
            float u = (float)i / radialSegments;
            float angle = u * glm::two_pi<float>();

            float x = cos(angle);
            float z = sin(angle);

            // 计算平滑法线
            glm::vec3 dir = glm::normalize(glm::vec3(x, radius / height, z));

            vertices.push_back({{x * radius, -halfH, z * radius},
                                dir,
                                {1, 1, 1, 1},
                                {u, 0.0f}});
        }

        // 索引（侧面）
        for (uint32_t i = 0; i < radialSegments; i++)
        {
            indices.push_back(apexIndex);
            indices.push_back(sideStart + i + 2);
            indices.push_back(sideStart + i + 1);
        }

        // ======================
        // 底盖 (-Y)
        // ======================
        uint32_t bottomCenterIndex = (uint32_t)vertices.size();
        vertices.push_back({{0.0f, -halfH, 0.0f},
                            {0.0f, -1.0f, 0.0f}});

        uint32_t bottomRingStart = (uint32_t)vertices.size();

        for (uint32_t i = 0; i <= radialSegments; i++)
        {
            float angle = (float)i / radialSegments * glm::two_pi<float>();
            float x = cos(angle);
            float z = sin(angle);

            vertices.push_back({{x * radius, -halfH, z * radius},
                                {0.0f, -1.0f, 0.0f},
                                {1, 1, 1, 1},
                                {x * 0.5f + 0.5f, z * 0.5f + 0.5f}});
        }

        for (uint32_t i = 0; i < radialSegments; i++)
        {
            // 保持 CCW（从外部下方观察）
            indices.push_back(bottomCenterIndex);
            indices.push_back(bottomRingStart + i);
            indices.push_back(bottomRingStart + i + 1);
        }

        // ======================
        // SubMesh
        // ======================
        SubMesh sub;
        sub.MaterialIndex = 0;
        sub.IndexOffset = 0;
        sub.IndexCount = (uint32_t)indices.size();

        auto mesh = std::make_shared<Mesh>(
            std::move(vertices),
            std::move(indices),
            std::vector<SubMesh>{sub});

        return AssetManager::addMemoryOnlyAsset(mesh);
    }
    AssetHandle MeshFactory::createMemoryMesh(MemoryMeshType type)
    {
        switch (type)
        {
        case MemoryMeshType::Cube:
            return MeshFactory::CreateBox(glm::vec3(1));
        case MemoryMeshType::Sphere:
            return MeshFactory::CreateSphere(0.5f);
        case MemoryMeshType::Cylinder:
            return MeshFactory::CreateCylinder(0.5f, 1.0f, 32);
        case MemoryMeshType::Capsule:
            return MeshFactory::CreateCapsule(0.5f, 1.5f, 32, 8);
        case MemoryMeshType::Cone:
            return MeshFactory::CreateCone(0.5f, 1.0f, 32);
        default:
            return {};
        }
    }
} // namespace Fermion
