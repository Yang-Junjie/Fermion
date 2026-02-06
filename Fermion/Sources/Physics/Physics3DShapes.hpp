#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

namespace Fermion
{
    struct TransformComponent;
    struct BoxCollider3DComponent;
    struct CircleCollider3DComponent;
    struct CapsuleCollider3DComponent;

    namespace Physics3DShapes
    {

        JPH::ShapeRefC CreateBoxShape(const TransformComponent &transform,
                                      const BoxCollider3DComponent *collider);

        JPH::ShapeRefC CreateSphereShape(const TransformComponent &transform,
                                         const CircleCollider3DComponent *collider);

        JPH::ShapeRefC CreateCapsuleShape(const TransformComponent &transform,
                                          const CapsuleCollider3DComponent *collider);
    } // namespace Physics3DShapes
} // namespace Fermion
