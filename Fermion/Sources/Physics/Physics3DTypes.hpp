#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Physics/Body/MotionType.h>

#include "Scene/Components.hpp"

namespace Fermion
{
    namespace Physics3DUtils
    {
        JPH::EMotionType ToJoltMotionType(Rigidbody3DComponent::BodyType type);

        JPH::Vec3 ToJoltVec3(const glm::vec3 &value);

        JPH::Quat ToJoltQuat(const glm::quat &value);

        glm::vec3 ToGlmVec3(const JPH::Vec3 &value);

        glm::quat ToGlmQuat(const JPH::Quat &value);
    } 
} // namespace Fermion
