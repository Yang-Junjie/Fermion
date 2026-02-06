#include "fmpch.hpp"
#include "Physics/Physics3DShapes.hpp"
#include "Physics/Physics3DTypes.hpp"
#include "Scene/Components.hpp"
#include "Core/Log.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/Model/Mesh.hpp"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>

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
    JPH::ShapeRefC CreateMeshShape(const TransformComponent &transform,
                                   const MeshCollider3DComponent *collider)
    {
        if (!collider || collider->meshHandle == AssetHandle(0))
        {
            Log::Error("Failed to create MeshShape: Invalid mesh handle");
            return nullptr;
        }

        auto mesh = AssetManager::getAsset<Mesh>(collider->meshHandle);
        if (!mesh)
        {
            Log::Error("Failed to create MeshShape: Could not load mesh asset");
            return nullptr;
        }

        const auto &vertices = mesh->getVertices();
        const auto &indices = mesh->getIndices();

        if (vertices.empty() || indices.empty())
        {
            Log::Error("Failed to create MeshShape: Mesh has no vertices or indices");
            return nullptr;
        }

        if (collider->convex)
        {
            // Create convex hull shape
            JPH::Array<JPH::Vec3> points;
            points.reserve(vertices.size());

            for (const auto &vertex : vertices)
            {
                glm::vec3 scaledPos = vertex.Position * transform.scale;
                points.push_back(Physics3DUtils::ToJoltVec3(scaledPos));
            }

            JPH::ConvexHullShapeSettings convexSettings(points);
            JPH::ShapeSettings::ShapeResult result = convexSettings.Create();
            if (result.HasError())
            {
                Log::Error(std::format("Failed to create ConvexHullShape: {}", result.GetError()));
                return nullptr;
            }
            return result.Get();
        }
        else
        {
            // Create triangle mesh shape
            JPH::TriangleList triangles;
            triangles.reserve(indices.size() / 3);

            for (size_t i = 0; i < indices.size(); i += 3)
            {
                if (i + 2 < indices.size())
                {
                    uint32_t idx0 = indices[i];
                    uint32_t idx1 = indices[i + 1];
                    uint32_t idx2 = indices[i + 2];

                    if (idx0 < vertices.size() && idx1 < vertices.size() && idx2 < vertices.size())
                    {
                        glm::vec3 v0 = vertices[idx0].Position * transform.scale;
                        glm::vec3 v1 = vertices[idx1].Position * transform.scale;
                        glm::vec3 v2 = vertices[idx2].Position * transform.scale;

                        JPH::Triangle triangle(
                            Physics3DUtils::ToJoltVec3(v0),
                            Physics3DUtils::ToJoltVec3(v1),
                            Physics3DUtils::ToJoltVec3(v2));
                        triangles.push_back(triangle);
                    }
                }
            }

            if (triangles.empty())
            {
                Log::Error("Failed to create MeshShape: No valid triangles");
                return nullptr;
            }

            JPH::MeshShapeSettings meshSettings(triangles);
            JPH::ShapeSettings::ShapeResult result = meshSettings.Create();
            if (result.HasError())
            {
                Log::Error(std::format("Failed to create MeshShape: {}", result.GetError()));
                return nullptr;
            }
            return result.Get();
        }
    }
} // namespace Fermion::Physics3DShapes
