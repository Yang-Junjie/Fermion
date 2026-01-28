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

        static AABB TransformAABB(const AABB &aabb, const glm::mat4 &transform)
        {
            glm::vec3 min(FLT_MAX);
            glm::vec3 max(-FLT_MAX);

            const glm::vec3 corners[8] = {
                {aabb.min.x, aabb.min.y, aabb.min.z},
                {aabb.max.x, aabb.min.y, aabb.min.z},
                {aabb.min.x, aabb.max.y, aabb.min.z},
                {aabb.max.x, aabb.max.y, aabb.min.z},
                {aabb.min.x, aabb.min.y, aabb.max.z},
                {aabb.max.x, aabb.min.y, aabb.max.z},
                {aabb.min.x, aabb.max.y, aabb.max.z},
                {aabb.max.x, aabb.max.y, aabb.max.z}};

            for (const auto &corner : corners)
            {
                glm::vec3 world = glm::vec3(transform * glm::vec4(corner, 1.0f));
                min = glm::min(min, world);
                max = glm::max(max, world);
            }

            return {min, max};
        }
    };

}