#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Fermion
{
    /**
     * @brief Represents a bone transformation with position, rotation, and scale
     */
    struct BoneTransform
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f);

        BoneTransform() = default;

        BoneTransform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl)
            : position(pos), rotation(rot), scale(scl)
        {
        }

        /**
         * @brief Convert this transform to a 4x4 transformation matrix
         * @return The transformation matrix (TRS order)
         */
        glm::mat4 toMatrix() const;

        /**
         * @brief Linearly interpolate between two bone transforms
         * @param a First transform
         * @param b Second transform
         * @param t Interpolation factor [0, 1]
         * @return Interpolated transform
         */
        static BoneTransform lerp(const BoneTransform& a, const BoneTransform& b, float t);
    };
} // namespace Fermion
