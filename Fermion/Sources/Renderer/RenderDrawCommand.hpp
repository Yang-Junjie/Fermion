#pragma once

#include "Model/Mesh.hpp"
#include "Model/Material.hpp"
#include "Pipeline.hpp"
#include "VertexArray.hpp"
#include "Math/AABB.hpp"

namespace Fermion {
struct MeshDrawCommand {

    // std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Pipeline> pipeline; 
    std::shared_ptr<VertexArray> vao;
    std::shared_ptr<Material> material;
    // int materialIndex = -1;  
    glm::mat4 transform;

    // GPU draw info
    uint32_t indexCount;
    uint32_t indexOffset;
    int objectID = -1;
    

    // CPU draw info
    AABB aabb;
    bool drawOutline = false; 
};

} // namespace Fermion
