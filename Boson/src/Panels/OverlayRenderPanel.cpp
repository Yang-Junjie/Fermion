#include "OverlayRenderPanel.hpp"
#include "Scene/EntityManager.hpp"
#include "Math/Math.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Fermion
{
    void OverlayRenderPanel::render(const Context &ctx) const
    {
        if (!beginOverlayPass(ctx))
            return;

        if (ctx.showPhysicsDebug)
        {
            renderPhysicsColliders(ctx);
            renderJoints(ctx);
        }

        renderSelectedEntityOutline(ctx);
        ctx.viewportRenderer->endOverlay();
    }

    bool OverlayRenderPanel::beginOverlayPass(const Context &ctx) const
    {
        if (ctx.sceneState == 1) // Play
        {
            Entity camera = ctx.activeScene->getEntityManager().getPrimaryCameraEntity();
            if (!camera)
                return false;

            ctx.viewportRenderer->beginOverlay(camera.getComponent<CameraComponent>().camera,
                                               camera.getComponent<TransformComponent>().getTransform());
        }
        else
        {
            ctx.viewportRenderer->beginOverlay(*ctx.editorCamera);
        }

        return true;
    }

    void OverlayRenderPanel::renderPhysicsColliders(const Context &ctx) const
    {
        renderPhysics2DColliders(ctx);
        renderPhysics3DColliders(ctx);
    }
    void OverlayRenderPanel::renderPhysics2DColliders(const Context &ctx) const
    {
        auto getParentTransform = [&](Entity entity)
        {
            Entity parent = ctx.activeScene->getEntityManager().tryGetEntityByUUID(entity.getParentUUID());
            if (parent)
                return ctx.activeScene->getEntityManager().getWorldSpaceTransformMatrix(parent);
            return glm::mat4(1.0f);
        };

        // Box Colliders
        {
            auto view = ctx.activeScene->getAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, ctx.activeScene.get()};
                auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

                glm::vec3 translation = tc.translation + glm::vec3(bc2d.offset, 0.001f);
                glm::vec3 scale = tc.scale * glm::vec3(bc2d.size * 2.0f, 1.0f);

                glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), tc.translation) *
                                           glm::rotate(glm::mat4(1.0f), tc.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                           glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.offset, 0.001f)) * glm::scale(glm::mat4(1.0f), scale);
                glm::mat4 transform = getParentTransform(colliderEntity) * localTransform;

                ctx.viewportRenderer->drawRect(transform, glm::vec4(0, 1, 0, 1));
            }
        }

        // Circle Colliders
        {
            auto view = ctx.activeScene->getAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, ctx.activeScene.get()};
                auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

                glm::vec3 scale = tc.scale * glm::vec3(cc2d.radius * 2.0f, cc2d.radius * 2.0f, 1.0f);

                glm::mat4 localTransform =
                    glm::translate(glm::mat4(1.0f), tc.translation) * glm::rotate(glm::mat4(1.0f), tc.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(cc2d.offset, 0.001f)) * glm::scale(glm::mat4(1.0f), scale);
                glm::mat4 transform = getParentTransform(colliderEntity) * localTransform;

                ctx.viewportRenderer->drawCircle(transform, glm::vec4(0, 1, 0, 1), 0.1f);
            }
        }
        // Box Sensor
        {
            auto view = ctx.activeScene->getAllEntitiesWith<TransformComponent, BoxSensor2DComponent>();
            for (auto entity : view)
            {
                Entity sensor = {entity, ctx.activeScene.get()};
                auto [tc, bs2d] = view.get<TransformComponent, BoxSensor2DComponent>(entity);
                glm::vec3 translation = tc.translation + glm::vec3(bs2d.offset, 0.001f);
                glm::vec3 scale = tc.scale * glm::vec3(bs2d.size * 2.0f, 1.0f);

                glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), tc.translation) *
                                           glm::rotate(glm::mat4(1.0f), tc.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                           glm::translate(glm::mat4(1.0f), glm::vec3(bs2d.offset, 0.001f)) * glm::scale(glm::mat4(1.0f), scale);
                glm::mat4 transform = getParentTransform(sensor) * localTransform;

                ctx.viewportRenderer->drawRect(transform, glm::vec4(0, 1, 1, 1));
            }
        }
    }
    void OverlayRenderPanel::renderPhysics3DColliders(const Context &ctx) const
    {
        constexpr float kTwoPi = 6.28318530718f;
        constexpr int kCircleSegments = 24;
        constexpr float kMinRadius = 0.001f;
        constexpr float kMinHalfHeight = 0.001f;
        const glm::vec4 collider3DColor{0.0f, 1.0f, 0.0f, 1.0f};

        auto getParentTransform = [&](Entity entity)
        {
            Entity parent = ctx.activeScene->getEntityManager().tryGetEntityByUUID(entity.getParentUUID());
            if (parent)
                return ctx.activeScene->getEntityManager().getWorldSpaceTransformMatrix(parent);
            return glm::mat4(1.0f);
        };

        auto getParentRotationScale = [&](Entity entity, glm::mat3 &rotation, glm::vec3 &scale)
        {
            glm::mat4 parentTransform = getParentTransform(entity);
            glm::mat3 basis = glm::mat3(parentTransform);

            scale = {
                glm::length(basis[0]),
                glm::length(basis[1]),
                glm::length(basis[2])};

            rotation = glm::mat3(1.0f);
            if (scale.x > 0.0f)
                rotation[0] = basis[0] / scale.x;
            if (scale.y > 0.0f)
                rotation[1] = basis[1] / scale.y;
            if (scale.z > 0.0f)
                rotation[2] = basis[2] / scale.z;
        };

        auto drawArc = [&](const glm::vec3 &center, const glm::vec3 &axisA, const glm::vec3 &axisB, float radius,
                           float startAngle, float endAngle, int segments, const glm::vec4 &color)
        {
            if (radius <= 0.0f || segments < 1)
                return;

            glm::vec3 u = glm::normalize(axisA);
            glm::vec3 v = glm::normalize(axisB);
            float step = (endAngle - startAngle) / static_cast<float>(segments);
            glm::vec3 prev = center + (u * std::cos(startAngle) + v * std::sin(startAngle)) * radius;

            for (int i = 1; i <= segments; ++i)
            {
                float angle = startAngle + step * static_cast<float>(i);
                glm::vec3 point = center + (u * std::cos(angle) + v * std::sin(angle)) * radius;
                ctx.viewportRenderer->drawLine(prev, point, color);
                prev = point;
            }
        };
        auto drawCircle = [&](const glm::vec3 &center, const glm::vec3 &axisA, const glm::vec3 &axisB, float radius,
                              const glm::vec4 &color)
        {
            drawArc(center, axisA, axisB, radius, 0.0f, kTwoPi, kCircleSegments, color);
        };

        auto drawBox = [&](const glm::mat4 &transform, const glm::vec4 &color)
        {
            const glm::vec3 localCorners[8] = {
                {-0.5f, -0.5f, -0.5f},
                {0.5f, -0.5f, -0.5f},
                {0.5f, 0.5f, -0.5f},
                {-0.5f, 0.5f, -0.5f},
                {-0.5f, -0.5f, 0.5f},
                {0.5f, -0.5f, 0.5f},
                {0.5f, 0.5f, 0.5f},
                {-0.5f, 0.5f, 0.5f}};

            glm::vec3 corners[8];
            for (int i = 0; i < 8; ++i)
            {
                corners[i] = transform * glm::vec4(localCorners[i], 1.0f);
            }

            const int edges[12][2] = {
                {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

            for (const auto &edge : edges)
            {
                ctx.viewportRenderer->drawLine(corners[edge[0]], corners[edge[1]], color);
            }
        };

        // 3D Box Colliders
        {
            auto view = ctx.activeScene->getAllEntitiesWith<TransformComponent, BoxCollider3DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, ctx.activeScene.get()};
                auto [tc, bc3d] = view.get<TransformComponent, BoxCollider3DComponent>(entity);

                glm::mat4 rotation = glm::toMat4(glm::quat(tc.rotation));
                glm::vec3 scale = tc.scale * (bc3d.size * 2.0f);
                glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), tc.translation) *
                                           rotation *
                                           glm::translate(glm::mat4(1.0f), bc3d.offset) *
                                           glm::scale(glm::mat4(1.0f), scale);
                glm::mat4 transform = getParentTransform(colliderEntity) * localTransform;

                drawBox(transform, collider3DColor);
            }
        }
        // 3D Sphere Colliders
        {
            auto view = ctx.activeScene->getAllEntitiesWith<TransformComponent, CircleCollider3DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, ctx.activeScene.get()};
                auto [tc, cc3d] = view.get<TransformComponent, CircleCollider3DComponent>(entity);

                glm::mat3 parentRotation;
                glm::vec3 parentScale;
                getParentRotationScale(colliderEntity, parentRotation, parentScale);

                glm::mat3 localRotation = glm::toMat3(glm::quat(tc.rotation));
                glm::mat3 worldRotation = parentRotation * localRotation;
                glm::vec3 worldScale = parentScale * tc.scale;

                glm::vec3 localCenter = tc.translation + (localRotation * cc3d.offset);
                glm::vec3 center = getParentTransform(colliderEntity) * glm::vec4(localCenter, 1.0f);
                float maxScale = std::max(worldScale.x, std::max(worldScale.y, worldScale.z));
                float radius = cc3d.radius * maxScale;
                if (radius <= 0.0f)
                    continue;

                glm::vec3 right = worldRotation * glm::vec3(1.0f, 0.0f, 0.0f);
                glm::vec3 up = worldRotation * glm::vec3(0.0f, 1.0f, 0.0f);
                glm::vec3 forward = worldRotation * glm::vec3(0.0f, 0.0f, 1.0f);

                drawCircle(center, right, up, radius, collider3DColor);
                drawCircle(center, right, forward, radius, collider3DColor);
                drawCircle(center, up, forward, radius, collider3DColor);
            }
        }
        // 3D Capsule Colliders
        {
            auto view = ctx.activeScene->getAllEntitiesWith<TransformComponent, CapsuleCollider3DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, ctx.activeScene.get()};
                auto [tc, cap3d] = view.get<TransformComponent, CapsuleCollider3DComponent>(entity);

                glm::mat3 parentRotation;
                glm::vec3 parentScale;
                getParentRotationScale(colliderEntity, parentRotation, parentScale);

                glm::mat3 localRotation = glm::toMat3(glm::quat(tc.rotation));
                glm::mat3 worldRotation = parentRotation * localRotation;
                glm::vec3 worldScale = parentScale * tc.scale;

                glm::vec3 localCenter = tc.translation + (localRotation * cap3d.offset);
                glm::vec3 center = getParentTransform(colliderEntity) * glm::vec4(localCenter, 1.0f);

                float radiusScale = std::max(worldScale.x, worldScale.z);
                float scaledRadius = cap3d.radius * radiusScale;
                float scaledHeight = cap3d.height * worldScale.y;

                if (scaledRadius < kMinRadius)
                    scaledRadius = kMinRadius;

                float halfHeight = scaledHeight * 0.5f;
                float halfCylinderHeight = halfHeight - scaledRadius;
                if (halfCylinderHeight < kMinHalfHeight)
                    halfCylinderHeight = kMinHalfHeight;

                glm::vec3 right = worldRotation * glm::vec3(1.0f, 0.0f, 0.0f);
                glm::vec3 up = worldRotation * glm::vec3(0.0f, 1.0f, 0.0f);
                glm::vec3 forward = worldRotation * glm::vec3(0.0f, 0.0f, 1.0f);

                glm::vec3 topCenter = center + up * halfCylinderHeight;
                glm::vec3 bottomCenter = center - up * halfCylinderHeight;

                drawCircle(topCenter, right, forward, scaledRadius, collider3DColor);
                drawCircle(bottomCenter, right, forward, scaledRadius, collider3DColor);

                ctx.viewportRenderer->drawLine(topCenter + right * scaledRadius, bottomCenter + right * scaledRadius,
                                             collider3DColor);
                ctx.viewportRenderer->drawLine(topCenter - right * scaledRadius, bottomCenter - right * scaledRadius,
                                             collider3DColor);
                ctx.viewportRenderer->drawLine(topCenter + forward * scaledRadius, bottomCenter + forward * scaledRadius,
                                             collider3DColor);
                ctx.viewportRenderer->drawLine(topCenter - forward * scaledRadius, bottomCenter - forward * scaledRadius,
                                             collider3DColor);

                const int arcSegments = kCircleSegments / 2;
                drawArc(topCenter, right, up, scaledRadius, 0.0f, kTwoPi * 0.5f, arcSegments, collider3DColor);
                drawArc(bottomCenter, right, up, scaledRadius, kTwoPi * 0.5f, kTwoPi, arcSegments, collider3DColor);
                drawArc(topCenter, forward, up, scaledRadius, 0.0f, kTwoPi * 0.5f, arcSegments, collider3DColor);
                drawArc(bottomCenter, forward, up, scaledRadius, kTwoPi * 0.5f, kTwoPi, arcSegments, collider3DColor);
            }
        }
        // 3D Mesh Colliders
        {
            auto view = ctx.activeScene->getAllEntitiesWith<TransformComponent, MeshCollider3DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, ctx.activeScene.get()};
                auto [tc, mc3d] = view.get<TransformComponent, MeshCollider3DComponent>(entity);

                if (static_cast<uint64_t>(mc3d.meshHandle) == 0)
                    continue;

                auto editorAssets = Project::getEditorAssetManager();
                auto mesh = editorAssets->getAsset<Mesh>(mc3d.meshHandle);
                if (!mesh)
                    continue;

                const auto &vertices = mesh->getVertices();
                const auto &indices = mesh->getIndices();

                if (vertices.empty() || indices.empty())
                    continue;

                glm::mat4 rotation = glm::toMat4(glm::quat(tc.rotation));
                glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), tc.translation) *
                                           rotation *
                                           glm::translate(glm::mat4(1.0f), mc3d.offset) *
                                           glm::scale(glm::mat4(1.0f), tc.scale);
                glm::mat4 transform = getParentTransform(colliderEntity) * localTransform;

                for (size_t i = 0; i < indices.size(); i += 3)
                {
                    if (i + 2 >= indices.size())
                        break;

                    uint32_t idx0 = indices[i];
                    uint32_t idx1 = indices[i + 1];
                    uint32_t idx2 = indices[i + 2];

                    if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size())
                        continue;

                    glm::vec3 v0 = transform * glm::vec4(vertices[idx0].Position, 1.0f);
                    glm::vec3 v1 = transform * glm::vec4(vertices[idx1].Position, 1.0f);
                    glm::vec3 v2 = transform * glm::vec4(vertices[idx2].Position, 1.0f);

                    ctx.viewportRenderer->drawLine(v0, v1, collider3DColor);
                    ctx.viewportRenderer->drawLine(v1, v2, collider3DColor);
                    ctx.viewportRenderer->drawLine(v2, v0, collider3DColor);
                }
            }
        }
    }
    void OverlayRenderPanel::renderJoints(const Context &ctx) const
    {
        renderJoint2Ds(ctx);
    }
    void OverlayRenderPanel::renderJoint2Ds(const Context &ctx) const
    {
        const glm::vec4 jointColor{1.0f, 0.6f, 0.0f, 1.0f};
        constexpr float anchorRadius = 0.05f;

        auto getAnchorWorldPos = [&](Entity entity, const glm::vec2 &localAnchor) -> glm::vec3
        {
            auto &tc = entity.getComponent<TransformComponent>();
            glm::mat4 parentTransform(1.0f);
            Entity parent = ctx.activeScene->getEntityManager().tryGetEntityByUUID(entity.getParentUUID());
            if (parent)
                parentTransform = ctx.activeScene->getEntityManager().getWorldSpaceTransformMatrix(parent);

            glm::mat4 bodyTransform = parentTransform *
                                      glm::translate(glm::mat4(1.0f), tc.translation) *
                                      glm::rotate(glm::mat4(1.0f), tc.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            return glm::vec3(bodyTransform * glm::vec4(localAnchor, 0.002f, 1.0f));
        };

        auto drawAnchorCircle = [&](const glm::vec3 &worldPos)
        {
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), worldPos) *
                                  glm::scale(glm::mat4(1.0f), glm::vec3(anchorRadius * 2.0f, anchorRadius * 2.0f, 1.0f));
            ctx.viewportRenderer->drawCircle(transform, jointColor, 0.3f);
        };

        // RevoluteJoint
        {
            auto view = ctx.activeScene->getAllEntitiesWith<TransformComponent, RevoluteJoint2DComponent>();
            for (auto e : view)
            {
                Entity entity{e, ctx.activeScene.get()};
                auto &rj = entity.getComponent<RevoluteJoint2DComponent>();

                drawAnchorCircle(getAnchorWorldPos(entity, rj.localAnchorA));

                if (static_cast<uint64_t>(rj.connectedBodyID) != 0)
                {
                    Entity connectedEntity = ctx.activeScene->getEntityManager().tryGetEntityByUUID(rj.connectedBodyID);
                    if (connectedEntity && connectedEntity.hasComponent<TransformComponent>())
                        drawAnchorCircle(getAnchorWorldPos(connectedEntity, rj.localAnchorB));
                }
            }
        }

        // DistanceJoint
        {
            auto view = ctx.activeScene->getAllEntitiesWith<TransformComponent, DistanceJoint2DComponent>();
            for (auto e : view)
            {
                Entity entity{e, ctx.activeScene.get()};
                auto &dj = entity.getComponent<DistanceJoint2DComponent>();

                glm::vec3 anchorA = getAnchorWorldPos(entity, dj.localAnchorA);
                drawAnchorCircle(anchorA);

                if (static_cast<uint64_t>(dj.connectedBodyID) != 0)
                {
                    Entity connectedEntity = ctx.activeScene->getEntityManager().tryGetEntityByUUID(dj.connectedBodyID);
                    if (connectedEntity && connectedEntity.hasComponent<TransformComponent>())
                    {
                        glm::vec3 anchorB = getAnchorWorldPos(connectedEntity, dj.localAnchorB);
                        drawAnchorCircle(anchorB);
                        ctx.viewportRenderer->drawLine(anchorA, anchorB, jointColor);
                    }
                }
            }
        }
    }
    void OverlayRenderPanel::renderSelectedEntityOutline(const Context &ctx) const
    {
        if (Entity selectedEntity = ctx.selectedEntity; selectedEntity)
        {
            glm::mat4 worldTransform = ctx.activeScene->getEntityManager().getWorldSpaceTransformMatrix(selectedEntity);
            if (selectedEntity.hasComponent<MeshComponent>())
            {
                if (ctx.viewportRenderer->getSceneInfo().renderMode == SceneRenderer::RenderMode::Forward)
                {
                    ctx.viewportRenderer->submitMesh(selectedEntity.getComponent<MeshComponent>(),
                                                     worldTransform, (int)selectedEntity, true);
                }
            }
            else
            {
                ctx.viewportRenderer->drawRect(worldTransform, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
        }
    }
} // namespace Fermion
