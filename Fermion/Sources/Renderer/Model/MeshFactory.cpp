#include "MeshFactory.hpp"
namespace Fermion {
AssetHandle MeshFactory::CreateBox(const glm::vec3 &size) {
    // std::vector<Vertex> vertices;
    // std::vector<uint32_t> indices;

    // const glm::vec3 h = size * 0.5f;

    // // ========== Front (+Z)
    // vertices.push_back({{-h.x, -h.y, h.z}, {0, 0, 1}});
    // vertices.push_back({{h.x, -h.y, h.z}, {0, 0, 1}});
    // vertices.push_back({{h.x, h.y, h.z}, {0, 0, 1}});
    // vertices.push_back({{-h.x, h.y, h.z}, {0, 0, 1}});

    // // ========== Back (-Z)
    // vertices.push_back({{h.x, -h.y, -h.z}, {0, 0, -1}});
    // vertices.push_back({{-h.x, -h.y, -h.z}, {0, 0, -1}});
    // vertices.push_back({{-h.x, h.y, -h.z}, {0, 0, -1}});
    // vertices.push_back({{h.x, h.y, -h.z}, {0, 0, -1}});

    // // ========== Left (-X)
    // vertices.push_back({{-h.x, -h.y, -h.z}, {-1, 0, 0}});
    // vertices.push_back({{-h.x, -h.y, h.z}, {-1, 0, 0}});
    // vertices.push_back({{-h.x, h.y, h.z}, {-1, 0, 0}});
    // vertices.push_back({{-h.x, h.y, -h.z}, {-1, 0, 0}});

    // // ========== Right (+X)
    // vertices.push_back({{h.x, -h.y, h.z}, {1, 0, 0}});
    // vertices.push_back({{h.x, -h.y, -h.z}, {1, 0, 0}});
    // vertices.push_back({{h.x, h.y, -h.z}, {1, 0, 0}});
    // vertices.push_back({{h.x, h.y, h.z}, {1, 0, 0}});

    // // ========== Top (+Y)
    // vertices.push_back({{-h.x, h.y, h.z}, {0, 1, 0}});
    // vertices.push_back({{h.x, h.y, h.z}, {0, 1, 0}});
    // vertices.push_back({{h.x, h.y, -h.z}, {0, 1, 0}});
    // vertices.push_back({{-h.x, h.y, -h.z}, {0, 1, 0}});

    // // ========== Bottom (-Y)
    // vertices.push_back({{-h.x, -h.y, -h.z}, {0, -1, 0}});
    // vertices.push_back({{h.x, -h.y, -h.z}, {0, -1, 0}});
    // vertices.push_back({{h.x, -h.y, h.z}, {0, -1, 0}});
    // vertices.push_back({{-h.x, -h.y, h.z}, {0, -1, 0}});

    // // ===== 索引（6 面 × 2 三角形）
    // for (uint32_t i = 0; i < 6; i++)
    // {
    //     uint32_t base = i * 4;
    //     indices.push_back(base + 0);
    //     indices.push_back(base + 1);
    //     indices.push_back(base + 2);

    //     indices.push_back(base + 2);
    //     indices.push_back(base + 3);
    //     indices.push_back(base + 0);
    // }

    // AssetHandle meshSource =

    return {};
}

} // namespace Fermion