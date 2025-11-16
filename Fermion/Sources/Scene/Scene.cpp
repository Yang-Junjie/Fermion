#include "fmpch.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Components.hpp"
#include "Scene/ScriptableEntity.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/SceneRenderer.hpp"
#include "Physics/Physics2D.hpp"
#include <glm/glm.hpp>
#include "Scene.hpp"
#include "Core/Log.hpp"

namespace Fermion
{
    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    template <typename... Component>
    static void copyComponent(entt::registry &dst, entt::registry &src, const std::unordered_map<UUID, entt::entity> &enttMap)
    {
        ([&]()
         {
			auto view = src.view<Component>();
			for (auto srcEntity : view)
			{
				entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

				auto& srcComponent = src.get<Component>(srcEntity);
				dst.emplace_or_replace<Component>(dstEntity, srcComponent);
			} }(), ...);
    }

    template <typename... Component>
    static void copyComponent(ComponentGroup<Component...>, entt::registry &dst, entt::registry &src, const std::unordered_map<UUID, entt::entity> &enttMap)
    {
        copyComponent<Component...>(dst, src, enttMap);
    }

    template <typename... Component>
    static void copyComponentIfExists(Entity dst, Entity src)
    {
        ([&]()
         {
			if (src.hasComponent<Component>())
				dst.addOrReplaceComponent<Component>(src.getComponent<Component>()); }(), ...);
    }
    template <typename... Component>
    static void copyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
    {
        copyComponentIfExists<Component...>(dst, src);
    }
    std::shared_ptr<Scene> Scene::copy(std::shared_ptr<Scene> other)
    {
        std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

        newScene->m_viewportWidth = other->m_viewportWidth;
        newScene->m_viewportHeight = other->m_viewportHeight;

        auto &srcSceneRegistry = other->m_registry;
        auto &dstSceneRegistry = newScene->m_registry;
        std::unordered_map<UUID, entt::entity> enttMap;

        auto idView = srcSceneRegistry.view<IDComponent>();
        for (auto e : idView)
        {
            UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
            const auto &name = srcSceneRegistry.get<TagComponent>(e).tag;
            Entity newEntity = newScene->createEntityWithUUID(uuid, name);
            enttMap[uuid] = (entt::entity)newEntity;
        }

        copyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

        return newScene;
    }

    void Scene::onRuntimeStart()
    {
        m_isRunning = true;
        onPhysics2DStart();
    }

    void Scene::onRuntimeStop()
    {
        m_isRunning = false;
        onPhysics2DStop();
    }

    void Scene::onSimulationStart()
    {
        onPhysics2DStart();
    }

    void Scene::onSimulationStop()
    {
        onPhysics2DStop();
    }
    void Scene::onPhysics2DStart()
    {
        // Create Box2D world
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0.0f, -9.8f};
        m_physicsWorld = b2CreateWorld(&worldDef);

        // Create bodies and shapes for all rigidbodies
        auto view = m_registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity{e, this};
            auto &transform = entity.getComponent<TransformComponent>();
            auto &rb2d = entity.getComponent<Rigidbody2DComponent>();

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.type);
            bodyDef.position = {transform.translation.x, transform.translation.y};
            bodyDef.rotation = b2MakeRot(transform.rotation.z);

            b2BodyId bodyId = b2CreateBody(m_physicsWorld, &bodyDef);

            if (rb2d.fixedRotation)
            {
                b2MotionLocks locks = {false, false, true};
                b2Body_SetMotionLocks(bodyId, locks);
            }

            // Store runtime handle (serialized as uint64_t)
            rb2d.runtimeBody = (void *)(uintptr_t)b2StoreBodyId(bodyId);

            if (entity.hasComponent<BoxCollider2DComponent>())
            {
                auto &bc2d = entity.getComponent<BoxCollider2DComponent>();

                float halfWidth = bc2d.size.x * transform.scale.x;
                float halfHeight = bc2d.size.y * transform.scale.y;

                b2ShapeDef shapeDef = b2DefaultShapeDef();
                shapeDef.density = bc2d.density;
                shapeDef.material.friction = bc2d.friction;
                shapeDef.material.restitution = bc2d.restitution;

                b2Polygon box = b2MakeOffsetBox(
                    halfWidth, halfHeight,
                    b2Vec2{bc2d.offset.x, bc2d.offset.y},
                    b2Rot_identity);

                b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
                bc2d.runtimeFixture = (void *)(uintptr_t)b2StoreShapeId(shapeId);
            }

            if (entity.hasComponent<CircleCollider2DComponent>())
            {
                auto &cc2d = entity.getComponent<CircleCollider2DComponent>();

                b2ShapeDef shapeDef = b2DefaultShapeDef();
                shapeDef.density = cc2d.density;
                shapeDef.material.friction = cc2d.friction;
                shapeDef.material.restitution = cc2d.restitution;

                b2Circle circle;
                // offset 也按缩放来算，和可视 Transform 一致
                circle.center = b2Vec2{
                    cc2d.offset.x * transform.scale.x,
                    cc2d.offset.y * transform.scale.y};

                circle.radius = cc2d.radius * transform.scale.x; // 或者用 max(scale.x, scale.y)

                b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
                cc2d.runtimeFixture = (void *)(uintptr_t)b2StoreShapeId(shapeId);
            }
        }
    }
    void Scene::onPhysics2DStop()
    {
        if (B2_IS_NON_NULL(m_physicsWorld))
        {
            b2DestroyWorld(m_physicsWorld);
            m_physicsWorld = b2_nullWorldId;
        }
    }

    void Scene::onRenderEditor(std::shared_ptr<SceneRenderer> renderer, EditorCamera &camera)
    {
        renderer->beginScene(camera);
        {
            auto group = m_registry.group<>(entt::get<TransformComponent, SpriteRendererComponent>);
            for (auto entity : group)
            {
                auto &transform = group.get<TransformComponent>(entity);
                auto &sprite = group.get<SpriteRendererComponent>(entity);
                renderer->drawSprite(transform.getTransform(), sprite, (int)entity);
            }
        }
        {
            auto group = m_registry.group<>(entt::get<TransformComponent, CircleRendererComponent>);
            for (auto entity : group)
            {
                auto &transform = group.get<TransformComponent>(entity);
                auto &circle = group.get<CircleRendererComponent>(entity);
                renderer->drawCircle(transform.getTransform(), circle.color, circle.thickness, circle.fade, (int)entity);
            }
        }

        renderer->endScene();
    }

    void Scene::onUpdateEditor(std::shared_ptr<SceneRenderer> renderer, Timestep ts, EditorCamera &camera)
    {
        onRenderEditor(renderer, camera);
    }

    void Scene::onUpdateSimulation(std::shared_ptr<SceneRenderer> renderer, Timestep ts, EditorCamera &camera)
    {
        if (!m_isPaused || m_stepFrames-- > 0)
            if (B2_IS_NON_NULL(m_physicsWorld))
            {
                b2World_Step(m_physicsWorld, ts.getSeconds(), 4);

                auto view = m_registry.view<Rigidbody2DComponent>();
                for (auto e : view)
                {
                    Entity entity{e, this};
                    auto &transform = entity.getComponent<TransformComponent>();
                    auto &rb2d = entity.getComponent<Rigidbody2DComponent>();

                    if (!rb2d.runtimeBody)
                        continue;

                    uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
                    b2BodyId bodyId = b2LoadBodyId(storedId);
                    if (!b2Body_IsValid(bodyId))
                        continue;

                    b2Transform xf = b2Body_GetTransform(bodyId);

                    transform.translation.x = xf.p.x;
                    transform.translation.y = xf.p.y;

                    float angle = atan2f(xf.q.s, xf.q.c);
                    transform.rotation.z = angle;
                }
            }
        onRenderEditor(renderer, camera);
    }

    void Scene::onUpdateRuntime(std::shared_ptr<SceneRenderer> renderer, Timestep ts)
    {
        // Scripts
        {
            m_registry.view<NativeScriptComponent>().each(
                [=](auto entity, auto &nsc)
                {
                    if (!nsc.instance)
                    {
                        nsc.instance = nsc.instantiateScript();
                        nsc.instance->m_entity = Entity{entity, this};
                        nsc.instance->onCreate();
                    }
                    nsc.instance->onUpdate(ts);
                });
        }
        // Physics2D
        {
            if (B2_IS_NON_NULL(m_physicsWorld))
            {
                b2World_Step(m_physicsWorld, ts.getSeconds(), 4);

                auto view = m_registry.view<Rigidbody2DComponent>();
                for (auto e : view)
                {
                    Entity entity{e, this};
                    auto &transform = entity.getComponent<TransformComponent>();
                    auto &rb2d = entity.getComponent<Rigidbody2DComponent>();

                    if (!rb2d.runtimeBody)
                        continue;

                    uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
                    b2BodyId bodyId = b2LoadBodyId(storedId);
                    if (!b2Body_IsValid(bodyId))
                        continue;

                    b2Transform xf = b2Body_GetTransform(bodyId);

                    transform.translation.x = xf.p.x;
                    transform.translation.y = xf.p.y;

                    float angle = atan2f(xf.q.s, xf.q.c);
                    transform.rotation.z = angle;
                }
            }
        }
        // Renderer2D
        {
            Camera *mainCamera = nullptr;
            glm::mat4 cameraTransform;

            {
                auto view = m_registry.view<CameraComponent, TransformComponent>();
                for (auto entity : view)
                {
                    auto &camera = view.get<CameraComponent>(entity);
                    auto &transform = view.get<TransformComponent>(entity);
                    if (camera.primary)
                    {
                        mainCamera = &camera.camera;
                        cameraTransform = transform.getTransform();
                        break;
                    }
                }
            }

            if (mainCamera)
            {
                renderer->beginScene(*mainCamera, cameraTransform);
                {
                    auto group = m_registry.group<>(entt::get<TransformComponent, SpriteRendererComponent>);
                    for (auto entity : group)
                    {
                        auto &transform = group.get<TransformComponent>(entity);
                        auto &sprite = group.get<SpriteRendererComponent>(entity);

                        renderer->drawSprite(transform.getTransform(), sprite, (int)entity);
                    }
                }
                {
                    auto group = m_registry.group<>(entt::get<TransformComponent, CircleRendererComponent>);
                    for (auto entity : group)
                    {
                        auto &transform = group.get<TransformComponent>(entity);
                        auto &circle = group.get<CircleRendererComponent>(entity);
                        renderer->drawCircle(transform.getTransform(), circle.color, circle.thickness, circle.fade, (int)entity);
                    }
                }

                renderer->endScene();
            }
        }
    }

    void Scene::onViewportResize(uint32_t width, uint32_t height)
    {
        m_viewportWidth = width;
        m_viewportHeight = height;

        auto view = m_registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto &cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.fixedAspectRatio)
            {
                cameraComponent.camera.setViewportSize(width, height);
            }
        }
    }
    Entity Scene::createEntity(std::string name)
    {
        return createEntityWithUUID(UUID(), name);
    }
    Entity Scene::createEntityWithUUID(UUID uuid, std::string name)
    {
        Entity entity{m_registry.create(), this};
        entity.addComponent<IDComponent>(uuid);
        entity.addComponent<TransformComponent>();
        entity.addComponent<TagComponent>(name.empty() ? std::string("unknown") : name);
        return entity;
    }
    void Scene::destroyEntity(Entity entity)
    {
        m_registry.destroy(entity);
    }

    Entity Scene::duplicateEntity(Entity entity)
    {
        std::string name = entity.getName();
        Entity newEntity = createEntity(name);
        copyComponentIfExists(AllComponents{}, newEntity, entity);
        return newEntity;
    }

    Entity Scene::getPrimaryCameraEntity()
    {
        auto view = m_registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto &camera = view.get<CameraComponent>(entity);
            if (camera.primary)
            {
                return Entity{entity, this};
            }
        }
        return {};
    }

    void Scene::step(int frames)
    {
        m_stepFrames = frames;
    }
}
