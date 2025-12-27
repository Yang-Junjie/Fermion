#pragma once

#include "Model/Mesh.hpp"
#include "Model/Material.hpp"

namespace Fermion {
struct MeshDrawCommand {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    glm::mat4 transform;

    int objectID = -1;
    bool drawOutline = false;
};

} // namespace Fermion
