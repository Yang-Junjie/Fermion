#include "fmpch.hpp"
#include "Physics/Physics3DTypes.hpp"
#include "Scene/Components.hpp"

#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Math/Quat.h>

namespace Fermion::Physics3DUtils
{
    JPH::EMotionType ToJoltMotionType(Rigidbody3DComponent::BodyType type)
    {
        switch (type)
        {
        case Rigidbody3DComponent::BodyType::Static:
            return JPH::EMotionType::Static;
        case Rigidbody3DComponent::BodyType::Kinematic:
            return JPH::EMotionType::Kinematic;
        case Rigidbody3DComponent::BodyType::Dynamic:
        default:
            return JPH::EMotionType::Dynamic;
        }
    }

    JPH::Vec3 ToJoltVec3(const glm::vec3 &value)
    {
        return {value.x, value.y, value.z};
    }

    JPH::Quat ToJoltQuat(const glm::quat &value)
    {
        return {value.x, value.y, value.z, value.w};
    }

    glm::vec3 ToGlmVec3(const JPH::Vec3 &value)
    {
        return {value.GetX(), value.GetY(), value.GetZ()};
    }

    glm::quat ToGlmQuat(const JPH::Quat &value)
    {
        return {value.GetW(), value.GetX(), value.GetY(), value.GetZ()};
    }
} // namespace Fermion::Physics3DUtils
