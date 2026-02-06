#include "fmpch.hpp"
#include "Physics/Physics3DShapes.hpp"
#include "Physics/Physics3DTypes.hpp"
#include "Scene/Components.hpp"
#include "Core/Log.hpp"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

#include <glm/glm.hpp>
#include <format>

namespace Fermion::Physics3DShapes
{
    JPH::ShapeRefC CreateBoxShape(const TransformComponent &transform,
                                  const BoxCollider3DComponent *collider)
    {
        glm::vec3 halfExtents = collider ? collider->size : glm::vec3{0.5f};
        halfExtents *= transform.scale;
        constexpr float cMinHalfExtent = 0.001f;
        halfExtents = glm::max(halfExtents, glm::vec3{cMinHalfExtent});

        JPH::BoxShapeSettings boxSettings(Physics3DUtils::ToJoltVec3(halfExtents));
        JPH::ShapeSettings::ShapeResult result = boxSettings.Create();
        if (result.HasError())
        {
            Log::Error(std::format("Failed to create BoxShape: {}", result.GetError()));
            return nullptr;
        }
        return result.Get();
    }

    JPH::ShapeRefC CreateSphereShape(const TransformComponent &transform,
                                     const CircleCollider3DComponent *collider)
    {
        float radius = collider ? collider->radius : 0.5f;
        // Use the maximum scale component for uniform sphere scaling
        float maxScale = glm::max(glm::max(transform.scale.x, transform.scale.y), transform.scale.z);
        radius *= maxScale;
        constexpr float cMinRadius = 0.001f;
        radius = glm::max(radius, cMinRadius);

        JPH::SphereShapeSettings sphereSettings(radius);
        JPH::ShapeSettings::ShapeResult result = sphereSettings.Create();
        if (result.HasError())
        {
            Log::Error(std::format("Failed to create SphereShape: {}", result.GetError()));
            return nullptr;
        }
        return result.Get();
    }

    JPH::ShapeRefC CreateCapsuleShape(const TransformComponent &transform,
                                      const CapsuleCollider3DComponent *collider)
    {
        float radius = collider ? collider->radius : 0.5f;
        float height = collider ? collider->height : 1.5f;

        float radiusScale = glm::max(transform.scale.x, transform.scale.z);
        float heightScale = transform.scale.y;

        float scaledRadius = radius * radiusScale;
        float scaledHeight = height * heightScale;

        constexpr float cMinRadius = 0.001f;
        constexpr float cMinHalfHeight = 0.001f;
        scaledRadius = glm::max(scaledRadius, cMinRadius);

        float halfHeight = scaledHeight * 0.5f;
        float halfCylinderHeight = halfHeight - scaledRadius;
        halfCylinderHeight = glm::max(halfCylinderHeight, cMinHalfHeight);

        JPH::CapsuleShapeSettings capsuleSettings(halfCylinderHeight, scaledRadius);
        JPH::ShapeSettings::ShapeResult result = capsuleSettings.Create();
        if (result.HasError())
        {
            Log::Error(std::format("Failed to create CapsuleShape: {}", result.GetError()));
            return nullptr;
        }
        return result.Get();
    }
} // namespace Fermion::Physics3DShapes
