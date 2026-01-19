#pragma once

#include "Model/Mesh.hpp"
#include "Model/Material.hpp"
#include "Pipeline.hpp"
#include "VertexArray.hpp"
#include "Math/AABB.hpp"
#include "Texture/Texture.hpp"

namespace Fermion {
struct MeshDrawCommand {
    std::shared_ptr<Pipeline> pipeline = nullptr;
    std::shared_ptr<VertexArray> vao = nullptr;
    std::shared_ptr<Material> material = nullptr;
    glm::mat4 transform;

    // GPU draw info
    uint32_t indexCount;
    uint32_t indexOffset;
    int objectID = -1;


    // CPU draw info
    AABB aabb;
    bool transparent = false;
    bool drawOutline = false;
    bool visible = true;
};

struct SkyboxDrawCommand {
    std::shared_ptr<Pipeline> pipeline = nullptr;
    std::shared_ptr<VertexArray> vao = nullptr;
    const TextureCube* cubemap = nullptr;
    glm::mat4 view;
    glm::mat4 projection;
};


} // namespace Fermion
