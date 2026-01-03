#pragma once
#include <glm/glm.hpp>

namespace Fermion
{
    struct AABB
    {
        glm::vec3 min;
        glm::vec3 max;
        AABB() : min(0.0f), max(0.0f) {}

        AABB(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}

        bool operator==(const AABB &other) const
        {
            return min == other.min && max == other.max;
        }
        glm::vec3 size() { return max - min; }
        glm::vec3 center() { return (max + min) * 0.5f; }
    };

}