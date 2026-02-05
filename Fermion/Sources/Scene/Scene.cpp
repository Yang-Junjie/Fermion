#include "fmpch.hpp"
#include "Scene/Scene.hpp"
#include "Scene/EntityManager.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Components.hpp"
#include "Renderer/Renderers/Renderer2D.hpp"
#include "Renderer/Renderers/SceneRenderer.hpp"
#include "Physics/Physics2D.hpp"
#include "Physics/Physics3D.hpp"
#include "Script/ScriptManager.hpp"
#include <glm/glm.hpp>
#include "Core/Log.hpp"

namespace Fermion
{
    Scene::Scene() : m_entityManager(std::make_unique<EntityManager>(this))
    {
        m_lightTexture = Texture2D::create("../Boson/Resources/icons/light.png");
        m_cameraTexture = Texture2D::create("../Boson/Resources/icons/Camera.png");
        m_physicsWorld3D = std::make_unique<Physics3DWorld>();
    }

    Scene::~Scene()
    {
        if (m_physicsWorld3D)
            m_physicsWorld3D->stop(this);
    }

    entt::registry &Scene::getRegistry()
    {
        return m_entityManager->getRegistry();
    }

    const entt::registry &Scene::getRegistry() const
    {
        return m_entityManager->getRegistry();
    }

    EntityManager &Scene::getEntityManager()
    {
        return *m_entityManager;
    }

    const EntityManager &Scene::getEntityManager() const
    {
        return *m_entityManager;
    }

    template <typename... Component>
    static void copyComponent(entt::registry &dst, entt::registry &src,
                              const std::unordered_map<UUID, entt::entity> &enttMap)
    {
        ([&]()
         {
            auto view = src.view<Component>();
            for (auto srcEntity: view) {
                entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

                auto &srcComponent = src.get<Component>(srcEntity);
                dst.emplace_or_replace<Component>(dstEntity, srcComponent);
            } }(), ...);
    }

    template <typename... Component>
    static void copyComponent(ComponentGroup<Component...>, entt::registry &dst, entt::registry &src,
                              const std::unordered_map<UUID, entt::entity> &enttMap)
    {
        copyComponent<Component...>(dst, src, enttMap);
    }

    std::shared_ptr<Scene> Scene::copy(std::shared_ptr<Scene> other)
    {
        std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

        newScene->m_viewportWidth = other->m_viewportWidth;
        newScene->m_viewportHeight = other->m_viewportHeight;

        auto &srcSceneRegistry = other->getRegistry();
        auto &dstSceneRegistry = newScene->getRegistry();
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
        if (m_physicsWorld3D)
            m_physicsWorld3D->start(this);
        {
            ScriptManager::onRuntimeStart(this);
            auto view = getRegistry().view<ScriptContainerComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                ScriptManager::onCreateEntity(entity);
            }
        }
    }

    void Scene::onRuntimeStop()
    {
        m_isRunning = false;
        onPhysics2DStop();
        if (m_physicsWorld3D)
            m_physicsWorld3D->stop(this);
        ScriptManager::onRuntimeStop();
    }

    void Scene::onSimulationStart()
    {
        onPhysics2DStart();
        if (m_physicsWorld3D)
            m_physicsWorld3D->start(this);
        {
            ScriptManager::onRuntimeStart(this);
            auto view = getRegistry().view<ScriptContainerComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                ScriptManager::onCreateEntity(entity);
            }
        }
    }

    void Scene::onSimulationStop()
    {
        onPhysics2DStop();
        if (m_physicsWorld3D)
            m_physicsWorld3D->stop(this);
        ScriptManager::onRuntimeStop();
    }

    void Scene::initPhysicsSensor(Entity entity)
    {
        TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(entity);
        auto &boxSensor2d = entity.getComponent<BoxSensor2DComponent>();

        float halfWidth = boxSensor2d.size.x * worldTransform.scale.x;
        float halfHeight = boxSensor2d.size.y * worldTransform.scale.y;

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.isSensor = true;
        shapeDef.enableSensorEvents = true;

        b2Polygon box = b2MakeOffsetBox(
            halfWidth, halfHeight,
            b2Vec2{boxSensor2d.offset.x, boxSensor2d.offset.y},
            b2Rot_identity);

        b2ShapeId shapeId = b2CreatePolygonShape(m_physicsBodyMap[entity.getUUID()], &shapeDef, &box);
        boxSensor2d.runtimeFixture = (void *)(uintptr_t)b2StoreShapeId(shapeId);
    }

    void Scene::onPhysics2DStart()
    {
        FM_PROFILE_FUNCTION();
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0.0f, -9.8f};
        m_physicsWorld = b2CreateWorld(&worldDef);

        auto view = getRegistry().view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity{e, this};
            TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(entity);
            auto &rb2d = entity.getComponent<Rigidbody2DComponent>();

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.type);
            bodyDef.position = {worldTransform.translation.x, worldTransform.translation.y};
            bodyDef.rotation = b2MakeRot(worldTransform.rotation.z);

            b2BodyId bodyId = b2CreateBody(m_physicsWorld, &bodyDef);
            m_physicsBodyMap[entity.getUUID()] = bodyId;

            if (rb2d.fixedRotation)
            {
                b2MotionLocks locks = {false, false, true};
                b2Body_SetMotionLocks(bodyId, locks);
            }

            rb2d.runtimeBody = (void *)(uintptr_t)b2StoreBodyId(bodyId);

            if (entity.hasComponent<BoxCollider2DComponent>())
            {
                auto &bc2d = entity.getComponent<BoxCollider2DComponent>();

                float halfWidth = bc2d.size.x * worldTransform.scale.x;
                float halfHeight = bc2d.size.y * worldTransform.scale.y;

                b2ShapeDef shapeDef = b2DefaultShapeDef();
                // TODO : material component
                shapeDef.density = bc2d.density;
                shapeDef.material.friction = bc2d.friction;
                shapeDef.material.restitution = bc2d.restitution;
                shapeDef.enableSensorEvents = true;

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
                shapeDef.enableSensorEvents = true;

                b2Circle circle;
                // offset 也按缩放来算，和可视 Transform 一致
                circle.center = b2Vec2{
                    cc2d.offset.x * worldTransform.scale.x,
                    cc2d.offset.y * worldTransform.scale.y};

                circle.radius = cc2d.radius * worldTransform.scale.x;

                b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
                cc2d.runtimeFixture = (void *)(uintptr_t)b2StoreShapeId(shapeId);
            }
            if (entity.hasComponent<BoxSensor2DComponent>())
            {
                initPhysicsSensor(entity);
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

    void Scene::onRenderEditor(std::shared_ptr<SceneRenderer> renderer, EditorCamera &camera, bool showRenderEntities)
    {
        FM_PROFILE_FUNCTION();
        renderer->beginScene(camera);
        if (showRenderEntities)
        {

            {
                auto cameraView = getRegistry().view<TransformComponent, CameraComponent>();
                for (auto entity : cameraView)
                {
                    Entity sceneEntity{entity, this};
                    TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(sceneEntity);
                    renderer->drawQuadBillboard(worldTransform.translation, glm::vec2{1.0f, 1.0f}, m_cameraTexture, 1.0f,
                                                glm::vec4(1.0f), (int)entity);
                }
            }
            {
                auto defaultView = getRegistry().view<TransformComponent, MeshComponent>();
                for (auto entity : defaultView)
                {
                    auto &mesh = defaultView.get<MeshComponent>(entity);
                    Entity sceneEntity{entity, this};
                    glm::mat4 worldTransform = m_entityManager->getWorldSpaceTransformMatrix(sceneEntity);

                    // Check if this entity also has an AnimatorComponent
                    if (sceneEntity.hasComponent<AnimatorComponent>())
                    {
                        auto &animator = sceneEntity.getComponent<AnimatorComponent>();
                        renderer->submitSkinnedMesh(mesh, animator, worldTransform, (int)entity);
                    }
                    else
                    {
                        renderer->submitMesh(mesh, worldTransform, (int)entity);
                    }
                }
            }

            // Reset lighting state so removing lights takes effect immediately
            m_environmentLight.directionalLights.clear();

            // Directional Lights
            {
                auto directionalLights = getRegistry().group<DirectionalLightComponent>(entt::get<TransformComponent>);
                m_environmentLight.directionalLights.reserve(directionalLights.size());

                DirectionalLight mainDirLight;
                bool hasMainLight = false;

                for (auto entity : directionalLights)
                {
                    auto &directionalLight = directionalLights.get<DirectionalLightComponent>(entity);
                    Entity sceneEntity{entity, this};
                    TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(sceneEntity);

                    DirectionalLight light = {
                        .direction = -worldTransform.getForward(),
                        .color = directionalLight.color,
                        .intensity = directionalLight.intensity
                    };

                    if (directionalLight.mainLight)
                    {
                        mainDirLight = light;
                        hasMainLight = true;
                    }
                    else
                    {
                        m_environmentLight.directionalLights.push_back(light);
                    }

                    renderer->drawQuadBillboard(worldTransform.translation, glm::vec2{1.0f, 1.0f}, m_lightTexture, 1.0f,
                                                glm::vec4{directionalLight.color, 1.0f}, (int)entity);
                }

                // Insert main light at the beginning if it exists
                if (hasMainLight)
                {
                    m_environmentLight.directionalLights.insert(m_environmentLight.directionalLights.begin(), mainDirLight);
                }
            }
            // Point Lights
            {
                auto pointLights = getRegistry().group<PointLightComponent>(entt::get<TransformComponent>);
                m_environmentLight.pointLights.clear();
                m_environmentLight.pointLights.reserve(pointLights.size());
                for (auto entity : pointLights)
                {
                    auto &pointLight = pointLights.get<PointLightComponent>(entity);
                    Entity sceneEntity{entity, this};
                    TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(sceneEntity);
                    m_environmentLight.pointLights.push_back(
                        {.position = worldTransform.translation,
                         .color = pointLight.color,
                         .intensity = pointLight.intensity,
                         .range = pointLight.range});
                    renderer->drawQuadBillboard(worldTransform.translation, glm::vec2{1.0f, 1.0f}, m_lightTexture, 1.0f,
                                                glm::vec4{pointLight.color, 1.0f}, (int)entity);
                }
            }
            // Spotlights
            {
                auto spotLights = getRegistry().group<SpotLightComponent>(entt::get<TransformComponent>);
                m_environmentLight.spotLights.clear();
                m_environmentLight.spotLights.reserve(spotLights.size());
                for (auto entity : spotLights)
                {
                    auto &spotLight = spotLights.get<SpotLightComponent>(entity);
                    Entity sceneEntity{entity, this};
                    TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(sceneEntity);

                    float outerRad = glm::radians(spotLight.angle);
                    float innerRad = outerRad * (1.0f - spotLight.softness);

                    innerRad = glm::clamp(innerRad, 0.0f, outerRad - 0.001f);

                    m_environmentLight.spotLights.push_back(
                        {.position = worldTransform.translation,
                         .direction = worldTransform.getForward(),
                         .color = spotLight.color,
                         .intensity = spotLight.intensity,
                         .range = spotLight.range,
                         .innerConeAngle = glm::cos(innerRad),
                         .outerConeAngle = glm::cos(outerRad)});
                    renderer->drawQuadBillboard(worldTransform.translation, glm::vec2{1.0f, 1.0f}, m_lightTexture, 1.0f,
                                                glm::vec4{spotLight.color, 1.0f}, (int)entity);
                }
            }

            {
                auto group = getRegistry().group<>(entt::get<TransformComponent, SpriteRendererComponent>);
                for (auto entity : group)
                {
                    auto &sprite = group.get<SpriteRendererComponent>(entity);
                    Entity sceneEntity{entity, this};
                    glm::mat4 worldTransform = m_entityManager->getWorldSpaceTransformMatrix(sceneEntity);
                    renderer->drawSprite(worldTransform, sprite, (int)entity);
                }
            }
            {
                auto group = getRegistry().group<>(entt::get<TransformComponent, CircleRendererComponent>);
                for (auto entity : group)
                {
                    auto &circle = group.get<CircleRendererComponent>(entity);
                    Entity sceneEntity{entity, this};
                    glm::mat4 worldTransform = m_entityManager->getWorldSpaceTransformMatrix(sceneEntity);
                    renderer->drawCircle(worldTransform, circle.color, circle.thickness, circle.fade,
                                         (int)entity);
                }
            }
            {
                auto group = getRegistry().group<>(entt::get<TransformComponent, TextComponent>);
                for (auto entity : group)
                {
                    auto &text = group.get<TextComponent>(entity);
                    Entity sceneEntity{entity, this};
                    glm::mat4 worldTransform = m_entityManager->getWorldSpaceTransformMatrix(sceneEntity);
                    renderer->drawString(text.textString, worldTransform, text, (int)entity);
                }
            }
        }
        // Debug draw
        {
            std::shared_ptr<DebugRenderer> debugRenderer = renderer->GetDebugRenderer();
            for (auto &&func : debugRenderer->GetRenderQueue())
                func(renderer);
            debugRenderer->ClearRenderQueue();
        }

        renderer->endScene();
    }

    void Scene::onUpdateEditor(std::shared_ptr<SceneRenderer> renderer, Timestep ts, EditorCamera &camera,
                               bool showRenderEntities)
    {
        FM_PROFILE_FUNCTION();

        // Update all animators
        {
            auto view = getRegistry().view<AnimatorComponent>();
            for (auto e : view)
            {
                auto &animator = view.get<AnimatorComponent>(e);
                if (animator.runtimeAnimator)
                {
                    animator.runtimeAnimator->update(ts);
                }
            }
        }

        onRenderEditor(renderer, camera, showRenderEntities);
    }

    void Scene::onUpdateSimulation(std::shared_ptr<SceneRenderer> renderer, Timestep ts, EditorCamera &camera,
                                   bool showRenderEntities)
    {
        FM_PROFILE_FUNCTION();

        if (!m_isPaused || m_stepFrames-- > 0)
        {
            onScriptStart(ts);
            {
                if (B2_IS_NON_NULL(m_physicsWorld))
                {
                    {
                        auto view = getRegistry().view<Rigidbody2DComponent>();
                        for (auto e : view)
                        {
                            Entity entity{e, this};
                            auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
                            if (rb2d.type != Rigidbody2DComponent::BodyType::Kinematic)
                                continue;

                            if (!rb2d.runtimeBody)
                                continue;

                            uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
                            b2BodyId bodyId = b2LoadBodyId(storedId);
                            if (!b2Body_IsValid(bodyId))
                                continue;

                            TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(entity);
                            b2Body_SetTransform(bodyId,
                                                {worldTransform.translation.x, worldTransform.translation.y},
                                                b2MakeRot(worldTransform.rotation.z));
                        }
                    }

                    b2World_Step(m_physicsWorld, ts.getSeconds(), 4);
                    b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_physicsWorld);
                    {
                        auto view = getRegistry().view<BoxSensor2DComponent>();
                        for (auto e : view)
                        {
                            Entity entity{e, this};
                            auto &bs2d = entity.getComponent<BoxSensor2DComponent>();

                            if (!bs2d.runtimeFixture)
                                continue;

                            uint64_t storedShapeId = (uint64_t)(uintptr_t)bs2d.runtimeFixture;
                            b2ShapeId myShapeId = b2LoadShapeId(storedShapeId);

                            if (!b2Shape_IsValid(myShapeId))
                                continue;

                            bs2d.sensorBegin = false;
                            for (int i = 0; i < sensorEvents.beginCount; ++i)
                            {
                                b2SensorBeginTouchEvent *begin = sensorEvents.beginEvents + i;
                                if (B2_ID_EQUALS(begin->sensorShapeId, myShapeId))
                                {
                                    bs2d.sensorBegin = true;
                                    break;
                                }
                            }
                            bs2d.sensorEnd = false;
                            for (int i = 0; i < sensorEvents.endCount; ++i)
                            {
                                b2SensorEndTouchEvent *end = sensorEvents.endEvents + i;
                                if (b2Shape_IsValid(end->sensorShapeId) &&
                                    B2_ID_EQUALS(end->sensorShapeId, myShapeId))
                                {
                                    bs2d.sensorEnd = true;
                                    break;
                                }
                            }
                        }
                    }
                    auto view = getRegistry().view<Rigidbody2DComponent>();
                    for (auto e : view)
                    {
                        Entity entity{e, this};
                        auto &rb2d = entity.getComponent<Rigidbody2DComponent>();

                        if (!rb2d.runtimeBody)
                            continue;

                        uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
                        b2BodyId bodyId = b2LoadBodyId(storedId);
                        if (!b2Body_IsValid(bodyId))
                            continue;

                        b2Transform xf = b2Body_GetTransform(bodyId);

                        float angle = atan2f(xf.q.s, xf.q.c);
                        TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(entity);
                        worldTransform.translation.x = xf.p.x;
                        worldTransform.translation.y = xf.p.y;
                        worldTransform.rotation.z = angle;
                        auto &transform = entity.getComponent<TransformComponent>();
                        transform.setTransform(worldTransform.getTransform());
                        m_entityManager->convertToLocalSpace(entity);
                    }
                }
            }
            if (m_physicsWorld3D)
                m_physicsWorld3D->step(this, ts);
        }

        // Update all animators
        {
            auto view = getRegistry().view<AnimatorComponent>();
            for (auto e : view)
            {
                auto &animator = view.get<AnimatorComponent>(e);
                if (animator.runtimeAnimator)
                {
                    animator.runtimeAnimator->update(ts);
                }
            }
        }

        onRenderEditor(renderer, camera, showRenderEntities);
    }

    void Scene::onUpdateRuntime(std::shared_ptr<SceneRenderer> renderer, Timestep ts, bool showRenderEntities)
    {
        FM_PROFILE_FUNCTION();

        onScriptStart(ts);

        // Physics2D
        {
            if (B2_IS_NON_NULL(m_physicsWorld))
            {
                {
                    auto view = getRegistry().view<Rigidbody2DComponent>();
                    for (auto e : view)
                    {
                        Entity entity{e, this};
                        auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
                        if (rb2d.type != Rigidbody2DComponent::BodyType::Kinematic)
                            continue;

                        if (!rb2d.runtimeBody)
                            continue;

                        uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
                        b2BodyId bodyId = b2LoadBodyId(storedId);
                        if (!b2Body_IsValid(bodyId))
                            continue;

                        TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(entity);
                        b2Body_SetTransform(bodyId,
                                            {worldTransform.translation.x, worldTransform.translation.y},
                                            b2MakeRot(worldTransform.rotation.z));
                    }
                }

                b2World_Step(m_physicsWorld, ts.getSeconds(), 4);
                b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_physicsWorld);
                {
                    auto view = getRegistry().view<BoxSensor2DComponent>();
                    for (auto e : view)
                    {
                        Entity entity{e, this};
                        auto &bs2d = entity.getComponent<BoxSensor2DComponent>();

                        if (!bs2d.runtimeFixture)
                            continue;

                        uint64_t storedShapeId = (uint64_t)(uintptr_t)bs2d.runtimeFixture;
                        b2ShapeId myShapeId = b2LoadShapeId(storedShapeId);

                        if (!b2Shape_IsValid(myShapeId))
                            continue;

                        bs2d.sensorBegin = false;
                        for (int i = 0; i < sensorEvents.beginCount; ++i)
                        {
                            b2SensorBeginTouchEvent *begin = sensorEvents.beginEvents + i;
                            if (B2_ID_EQUALS(begin->sensorShapeId, myShapeId))
                            {
                                bs2d.sensorBegin = true;
                                break;
                            }
                        }
                        bs2d.sensorEnd = false;
                        for (int i = 0; i < sensorEvents.endCount; ++i)
                        {
                            b2SensorEndTouchEvent *end = sensorEvents.endEvents + i;
                            if (b2Shape_IsValid(end->sensorShapeId) && B2_ID_EQUALS(end->sensorShapeId, myShapeId))
                            {
                                bs2d.sensorEnd = true;
                                break;
                            }
                        }
                    }
                }

                auto view = getRegistry().view<Rigidbody2DComponent>();
                for (auto e : view)
                {
                    Entity entity{e, this};
                    auto &rb2d = entity.getComponent<Rigidbody2DComponent>();

                    if (!rb2d.runtimeBody)
                        continue;

                    uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
                    b2BodyId bodyId = b2LoadBodyId(storedId);
                    if (!b2Body_IsValid(bodyId))
                        continue;

                    b2Transform xf = b2Body_GetTransform(bodyId);

                    float angle = atan2f(xf.q.s, xf.q.c);
                    TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(entity);
                    worldTransform.translation.x = xf.p.x;
                    worldTransform.translation.y = xf.p.y;
                    worldTransform.rotation.z = angle;
                    auto &transform = entity.getComponent<TransformComponent>();
                    transform.setTransform(worldTransform.getTransform());
                    m_entityManager->convertToLocalSpace(entity);
                }
            }
        }
        if (m_physicsWorld3D)
            m_physicsWorld3D->step(this, ts);

        // Update all animators
        {
            auto animView = getRegistry().view<AnimatorComponent>();
            for (auto e : animView)
            {
                auto &animator = animView.get<AnimatorComponent>(e);
                if (animator.runtimeAnimator)
                {
                    animator.runtimeAnimator->update(ts);
                }
            }
        }

        {
            Camera *mainCamera = nullptr;
            glm::mat4 cameraTransform;

            {
                auto view = getRegistry().view<CameraComponent, TransformComponent>();
                for (auto entity : view)
                {
                    auto &camera = view.get<CameraComponent>(entity);
                    if (camera.primary)
                    {
                        mainCamera = &camera.camera;
                        cameraTransform = m_entityManager->getWorldSpaceTransformMatrix(Entity{entity, this});
                        break;
                    }
                }
            }

            if (mainCamera)
            {
                renderer->beginScene(*mainCamera, cameraTransform);
                if (showRenderEntities)
                {
                    // Reset lighting state so removing lights takes effect immediately
                    m_environmentLight.directionalLights.clear();

                    {
                        auto defaultView = getRegistry().view<TransformComponent, MeshComponent>();
                        for (auto entity : defaultView)
                        {
                            auto &mesh = defaultView.get<MeshComponent>(entity);
                            Entity sceneEntity{entity, this};
                            glm::mat4 worldTransform = m_entityManager->getWorldSpaceTransformMatrix(sceneEntity);

                            // Check if this entity also has an AnimatorComponent
                            if (sceneEntity.hasComponent<AnimatorComponent>())
                            {
                                auto &animator = sceneEntity.getComponent<AnimatorComponent>();
                                renderer->submitSkinnedMesh(mesh, animator, worldTransform, (int)entity);
                            }
                            else
                            {
                                renderer->submitMesh(mesh, worldTransform, (int)entity);
                            }
                        }
                    }
                    // Directional Lights
                    {
                        auto directionalLights = getRegistry().group<DirectionalLightComponent>(
                            entt::get<TransformComponent>);
                        m_environmentLight.directionalLights.reserve(directionalLights.size());

                        DirectionalLight mainDirLight;
                        bool hasMainLight = false;

                        for (auto entity : directionalLights)
                        {
                            auto &directionalLight = directionalLights.get<DirectionalLightComponent>(entity);
                            Entity sceneEntity{entity, this};
                            TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(sceneEntity);

                            DirectionalLight light = {
                                .direction = -worldTransform.getForward(),
                                .color = directionalLight.color,
                                .intensity = directionalLight.intensity
                            };

                            if (directionalLight.mainLight)
                            {
                                mainDirLight = light;
                                hasMainLight = true;
                            }
                            else
                            {
                                m_environmentLight.directionalLights.push_back(light);
                            }
                        }

                        // Insert main light at the beginning if it exists
                        if (hasMainLight)
                        {
                            m_environmentLight.directionalLights.insert(m_environmentLight.directionalLights.begin(), mainDirLight);
                        }
                    }
                    // Point Lights
                    {
                        auto pointLights = getRegistry().group<PointLightComponent>(entt::get<TransformComponent>);
                        m_environmentLight.pointLights.clear();
                        m_environmentLight.pointLights.reserve(pointLights.size());
                        for (auto entity : pointLights)
                        {
                            auto &pointLight = pointLights.get<PointLightComponent>(entity);
                            Entity sceneEntity{entity, this};
                            TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(sceneEntity);
                            m_environmentLight.pointLights.push_back(
                                {.position = worldTransform.translation,
                                 .color = pointLight.color,
                                 .intensity = pointLight.intensity,
                                 .range = pointLight.range});
                        }
                    }
                    // Spotlights
                    {
                        auto spotLights = getRegistry().group<SpotLightComponent>(entt::get<TransformComponent>);
                        m_environmentLight.spotLights.clear();
                        m_environmentLight.spotLights.reserve(spotLights.size());
                        for (auto entity : spotLights)
                        {
                            auto &spotLight = spotLights.get<SpotLightComponent>(entity);
                            Entity sceneEntity{entity, this};
                            TransformComponent worldTransform = m_entityManager->getWorldSpaceTransform(sceneEntity);

                            float outerRad = glm::radians(spotLight.angle);
                            float innerRad = outerRad * (1.0f - spotLight.softness);

                            innerRad = glm::clamp(innerRad, 0.0f, outerRad - 0.001f);

                            m_environmentLight.spotLights.push_back(
                                {.position = worldTransform.translation,
                                 .direction = worldTransform.getForward(),
                                 .color = spotLight.color,
                                 .intensity = spotLight.intensity,
                                 .range = spotLight.range,
                                 .innerConeAngle = glm::cos(innerRad),
                                 .outerConeAngle = glm::cos(outerRad)});
                        }
                    }
                }
            }

            {
                auto group = getRegistry().group<>(entt::get<TransformComponent, SpriteRendererComponent>);
                for (auto entity : group)
                {
                    auto &sprite = group.get<SpriteRendererComponent>(entity);

                    Entity sceneEntity{entity, this};
                    glm::mat4 worldTransform = m_entityManager->getWorldSpaceTransformMatrix(sceneEntity);
                    renderer->drawSprite(worldTransform, sprite, (int)entity);
                }
            }
            {
                auto group = getRegistry().group<>(entt::get<TransformComponent, CircleRendererComponent>);
                for (auto entity : group)
                {
                    auto &circle = group.get<CircleRendererComponent>(entity);
                    Entity sceneEntity{entity, this};
                    glm::mat4 worldTransform = m_entityManager->getWorldSpaceTransformMatrix(sceneEntity);
                    renderer->drawCircle(worldTransform, circle.color, circle.thickness, circle.fade,
                                         (int)entity);
                }
            }
        }

        // Debug draw
        {
            std::shared_ptr<DebugRenderer> debugRenderer = renderer->GetDebugRenderer();
            for (auto &&func : debugRenderer->GetRenderQueue())
                func(renderer);
            debugRenderer->ClearRenderQueue();
        }
        // renderer->DrawCube(glm::mat4(1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        renderer->endScene();
    }

    void Scene::onScriptStart(Timestep ts)
    {
        auto view = getRegistry().view<ScriptContainerComponent>();
        for (auto e : view)
        {
            Entity entity = {e, this};

            ScriptManager::onUpdateEntity(entity, ts);
        }

        // getRegistry().view<NativeScriptComponent>().each(
        //     [=](auto entity, auto &nsc)
        //     {
        //         if (!nsc.instance)
        //         {
        //             nsc.instance = nsc.instantiateScript();
        //             nsc.instance->m_entity = Entity{entity, this};
        //             nsc.instance->onCreate();
        //         }
        //         nsc.instance->onUpdate(ts);
        //     });
    }

    void Scene::onViewportResize(uint32_t width, uint32_t height)
    {
        m_viewportWidth = width;
        m_viewportHeight = height;

        auto view = getRegistry().view<CameraComponent>();
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
        return m_entityManager->createEntity(std::move(name));
    }

    Entity Scene::createChildEntity(Entity parent, std::string name)
    {
        return m_entityManager->createChildEntity(parent, std::move(name));
    }
    Entity Scene::createEntityWithUUID(UUID uuid, std::string name)
    {
        return m_entityManager->createEntityWithUUID(uuid, std::move(name));
    }

    void Scene::step(int frames)
    {
        m_stepFrames = frames;
    }
} // namespace Fermion
