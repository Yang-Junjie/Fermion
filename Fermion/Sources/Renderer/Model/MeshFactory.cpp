#include "MeshFactory.hpp"
#include "Asset/AssetManager.hpp"
#include "Mesh.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
namespace Fermion
{
AssetHandle MeshFactory::CreateBox(const glm::vec3 &size) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    const glm::vec3 h = size * 0.5f;

    // ========== Front (+Z)
    vertices.push_back({{-h.x, -h.y, h.z}, {0, 0, 1}});
    vertices.push_back({{h.x, -h.y, h.z}, {0, 0, 1}});
    vertices.push_back({{h.x, h.y, h.z}, {0, 0, 1}});
    vertices.push_back({{-h.x, h.y, h.z}, {0, 0, 1}});

    // ========== Back (-Z)
    vertices.push_back({{h.x, -h.y, -h.z}, {0, 0, -1}});
    vertices.push_back({{-h.x, -h.y, -h.z}, {0, 0, -1}});
    vertices.push_back({{-h.x, h.y, -h.z}, {0, 0, -1}});
    vertices.push_back({{h.x, h.y, -h.z}, {0, 0, -1}});

    // ========== Left (-X)
    vertices.push_back({{-h.x, -h.y, -h.z}, {-1, 0, 0}});
    vertices.push_back({{-h.x, -h.y, h.z}, {-1, 0, 0}});
    vertices.push_back({{-h.x, h.y, h.z}, {-1, 0, 0}});
    vertices.push_back({{-h.x, h.y, -h.z}, {-1, 0, 0}});

    // ========== Right (+X)
    vertices.push_back({{h.x, -h.y, h.z}, {1, 0, 0}});
    vertices.push_back({{h.x, -h.y, -h.z}, {1, 0, 0}});
    vertices.push_back({{h.x, h.y, -h.z}, {1, 0, 0}});
    vertices.push_back({{h.x, h.y, h.z}, {1, 0, 0}});

    // ========== Top (+Y)
    vertices.push_back({{-h.x, h.y, h.z}, {0, 1, 0}});
    vertices.push_back({{h.x, h.y, h.z}, {0, 1, 0}});
    vertices.push_back({{h.x, h.y, -h.z}, {0, 1, 0}});
    vertices.push_back({{-h.x, h.y, -h.z}, {0, 1, 0}});

    // ========== Bottom (-Y)
    vertices.push_back({{-h.x, -h.y, -h.z}, {0, -1, 0}});
    vertices.push_back({{h.x, -h.y, -h.z}, {0, -1, 0}});
    vertices.push_back({{h.x, -h.y, h.z}, {0, -1, 0}});
    vertices.push_back({{-h.x, -h.y, h.z}, {0, -1, 0}});

    // ===== 索引（6 面 × 2 三角形）
    for (uint32_t i = 0; i < 6; i++) {
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
    uint32_t longitudeSegments) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t y = 0; y <= latitudeSegments; y++) {
        float v = (float)y / (float)latitudeSegments;
        float theta = v * glm::pi<float>();

        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (uint32_t x = 0; x <= longitudeSegments; x++) {
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
                                glm::normalize(normal)});
        }
    }

    // ===== 索引 =====
    uint32_t stride = longitudeSegments + 1;

    for (uint32_t y = 0; y < latitudeSegments; y++) {
        for (uint32_t x = 0; x < longitudeSegments; x++) {
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
} // namespace Fermion
