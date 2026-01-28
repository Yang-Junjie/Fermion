#pragma once

#include <glm/glm.hpp>
#include "AABB.hpp"
namespace Fermion::Math
{

    bool decomposeTransform(const glm::mat4 &transform, glm::vec3 &translation, glm::vec3 &rotation, glm::vec3 &scale);
    std::array<glm::vec4, 6> ExtractFrustumPlanes(const glm::mat4 &viewProjection);
    bool IsAABBInsideFrustum(const std::array<glm::vec4, 6> &planes, const AABB &aabb);
}
