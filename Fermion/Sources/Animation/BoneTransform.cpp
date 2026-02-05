#include "fmpch.hpp"
#include "BoneTransform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Fermion
{
    glm::mat4 BoneTransform::toMatrix() const
    {
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotationMatrix = glm::toMat4(rotation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    BoneTransform BoneTransform::lerp(const BoneTransform& a, const BoneTransform& b, float t)
    {
        BoneTransform result;
        result.position = glm::mix(a.position, b.position, t);
        result.rotation = glm::slerp(a.rotation, b.rotation, t);
        result.scale = glm::mix(a.scale, b.scale, t);
        return result;
    }
} // namespace Fermion
