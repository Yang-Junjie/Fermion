#include "fmpch.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Components.hpp"
#include "Scene/ScriptableEntity.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/SceneRenderer.hpp"
#include "Physics/Physics2D.hpp"
#include "Physics/Physics3D.hpp"
#include "Script/ScriptManager.hpp"
#include <glm/glm.hpp>
#include "Core/Log.hpp"

namespace Fermion
{
    Scene::Scene()
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
        if (m_physicsWorld3D)
            m_physicsWorld3D->start(this);
        {
            ScriptManager::onRuntimeStart(this);
            auto view = m_registry.view<ScriptContainerComponent>();
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
            auto view = m_registry.view<ScriptContainerComponent>();
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
        auto &transform = entity.getComponent<TransformComponent>();
        auto &boxSensor2d = entity.getComponent<BoxSensor2DComponent>();

        float halfWidth = boxSensor2d.size.x * transform.scale.x;
        float halfHeight = boxSensor2d.size.y * transform.scale.y;

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

                float halfWidth = bc2d.size.x * transform.scale.x;
                float halfHeight = bc2d.size.y * transform.scale.y;

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
                    cc2d.offset.x * transform.scale.x,
                    cc2d.offset.y * transform.scale.y};

                circle.radius = cc2d.radius * transform.scale.x;

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
                auto cameraView = m_registry.view<TransformComponent, CameraComponent>();
                for (auto entity : cameraView)
                {
                    auto &transform = cameraView.get<TransformComponent>(entity);
                    renderer->drawQuadBillboard(transform.translation, glm::vec2{1.0f, 1.0f}, m_cameraTexture, 1.0f,
                                                glm::vec4(1.0f), (int)entity);
                }
            }
            {
                // 优先渲染带MaterialSlotsComponent的对象（支持多材质）
                auto materialSlotsView = m_registry.view<TransformComponent, MeshComponent, MaterialSlotsComponent>();
                for (auto entity : materialSlotsView)
                {
                    auto &transform = materialSlotsView.get<TransformComponent>(entity);
                    auto &mesh = materialSlotsView.get<MeshComponent>(entity);
                    auto &materialSlots = materialSlotsView.get<MaterialSlotsComponent>(entity);
                    renderer->submitMesh(mesh, materialSlots, transform.getTransform(), (int)entity);
                }

                // 渲染带PBR材质的对象（向后兼容）
                auto pbrView = m_registry.view<TransformComponent, MeshComponent, PBRMaterialComponent>(
                    entt::exclude<MaterialSlotsComponent>);
                for (auto entity : pbrView)
                {
                    auto &transform = pbrView.get<TransformComponent>(entity);
                    auto &mesh = pbrView.get<MeshComponent>(entity);
                    auto &pbrMat = pbrView.get<PBRMaterialComponent>(entity);
                    renderer->submitMesh(mesh, pbrMat, transform.getTransform(), (int)entity);
                }

                // 渲染带Phong材质的对象（向后兼容）
                auto phongView = m_registry.view<TransformComponent, MeshComponent, PhongMaterialComponent>(
                    entt::exclude<MaterialSlotsComponent>);
                for (auto entity : phongView)
                {
                    auto &transform = phongView.get<TransformComponent>(entity);
                    auto &mesh = phongView.get<MeshComponent>(entity);
                    auto &phongMat = phongView.get<PhongMaterialComponent>(entity);
                    renderer->submitMesh(mesh, phongMat, transform.getTransform(), (int)entity);
                }

                // 渲染没有材质组件的对象（使用mesh默认材质）
                auto defaultView = m_registry.view<TransformComponent, MeshComponent>(
                    entt::exclude<PBRMaterialComponent, PhongMaterialComponent, MaterialSlotsComponent>);
                for (auto entity : defaultView)
                {
                    auto &transform = defaultView.get<TransformComponent>(entity);
                    auto &mesh = defaultView.get<MeshComponent>(entity);
                    renderer->submitMesh(mesh, transform.getTransform(), (int)entity);
                }
            }

            // Reset lighting state so removing lights takes effect immediately
            m_environmentLight.directionalLight.color = glm::vec3(0.0f);
            m_environmentLight.directionalLight.intensity = 0.0f;

            // Directional Lights
            {
                auto directionalLights = m_registry.group<DirectionalLightComponent>(entt::get<TransformComponent>);
                for (auto entity : directionalLights)
                {
                    auto &transform = directionalLights.get<TransformComponent>(entity);
                    auto &directionalLight = directionalLights.get<DirectionalLightComponent>(entity);
                    if (directionalLight.mainLight)
                    {
                        m_environmentLight.directionalLight =
                            {
                                .direction = -transform.getForward(),
                                .color = directionalLight.color,
                                .intensity = directionalLight.intensity};
                    }
                    renderer->drawQuadBillboard(transform.translation, glm::vec2{1.0f, 1.0f}, m_lightTexture, 1.0f,
                                                glm::vec4{directionalLight.color, 1.0f}, (int)entity);
                }
            }
            // Point Lights
            {
                auto pointLights = m_registry.group<PointLightComponent>(entt::get<TransformComponent>);
                m_environmentLight.pointLights.clear();
                m_environmentLight.pointLights.reserve(pointLights.size());
                for (auto entity : pointLights)
                {
                    auto &transform = pointLights.get<TransformComponent>(entity);
                    auto &pointLight = pointLights.get<PointLightComponent>(entity);
                    m_environmentLight.pointLights.push_back(
                        {.position = transform.translation,
                         .color = pointLight.color,
                         .intensity = pointLight.intensity,
                         .range = pointLight.range});
                    renderer->drawQuadBillboard(transform.translation, glm::vec2{1.0f, 1.0f}, m_lightTexture, 1.0f,
                                                glm::vec4{pointLight.color, 1.0f}, (int)entity);
                }
            }
            // Spotlights
            {
                auto spotLights = m_registry.group<SpotLightComponent>(entt::get<TransformComponent>);
                m_environmentLight.spotLights.clear();
                m_environmentLight.spotLights.reserve(spotLights.size());
                for (auto entity : spotLights)
                {
                    auto &transform = spotLights.get<TransformComponent>(entity);
                    auto &spotLight = spotLights.get<SpotLightComponent>(entity);

                    float outerRad = glm::radians(spotLight.angle);
                    float innerRad = outerRad * (1.0f - spotLight.softness);

                    innerRad = glm::clamp(innerRad, 0.0f, outerRad - 0.001f);

                    m_environmentLight.spotLights.push_back(
                        {.position = transform.translation,
                         .direction = transform.getForward(),
                         .color = spotLight.color,
                         .intensity = spotLight.intensity,
                         .range = spotLight.range,
                         .innerConeAngle = glm::cos(innerRad),
                         .outerConeAngle = glm::cos(outerRad)});
                    renderer->drawQuadBillboard(transform.translation, glm::vec2{1.0f, 1.0f}, m_lightTexture, 1.0f,
                                                glm::vec4{spotLight.color, 1.0f}, (int)entity);
                }
            }

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
                    renderer->drawCircle(transform.getTransform(), circle.color, circle.thickness, circle.fade,
                                         (int)entity);
                }
            }
            {
                auto group = m_registry.group<>(entt::get<TransformComponent, TextComponent>);
                for (auto entity : group)
                {
                    auto &transform = group.get<TransformComponent>(entity);
                    auto &text = group.get<TextComponent>(entity);
                    renderer->drawString(text.textString, transform.getTransform(), text, (int)entity);
                }
            }
        }
        // Debug draw
        {
            std::shared_ptr<DebugRenderer> debugRenderer = renderer->GetDebugRenderer();
            for (auto &&func : debugRenderer->GetRenderQueue())
                func(renderer);
            debugRenderer->ClearRenderQueue();

            // TODO(Yang) : 升级2DRenderer
            // // X aixs
            // renderer->drawInfiniteLine(glm::vec3(0), glm::vec3(1,0,0),glm::vec4(0.5f,1.0f,0.3f,1.0f));
            // // Y aixs
            // renderer->drawInfiniteLine(glm::vec3(0), glm::vec3(0,1,0),glm::vec4(0.3f,0.5f,1.0f,1.0f));
            // // Z aixs
            // renderer->drawInfiniteLine(glm::vec3(0), glm::vec3(0,0,1),glm::vec4(1.0f,0.3f,0.5f,1.0f));
        }

        renderer->endScene();
    }

    void Scene::onUpdateEditor(std::shared_ptr<SceneRenderer> renderer, Timestep ts, EditorCamera &camera,
                               bool showRenderEntities)
    {
        FM_PROFILE_FUNCTION();
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
                    b2World_Step(m_physicsWorld, ts.getSeconds(), 4);
                    b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_physicsWorld);
                    {
                        auto view = m_registry.view<BoxSensor2DComponent>();
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
            if (m_physicsWorld3D)
                m_physicsWorld3D->step(this, ts);
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
                b2World_Step(m_physicsWorld, ts.getSeconds(), 4);
                b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_physicsWorld);
                {
                    auto view = m_registry.view<BoxSensor2DComponent>();
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
        if (m_physicsWorld3D)
            m_physicsWorld3D->step(this, ts);

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
                if (showRenderEntities)
                {
                    // Reset lighting state so removing lights takes effect immediately
                    m_environmentLight.directionalLight.color = glm::vec3(0.0f);
                    m_environmentLight.directionalLight.intensity = 0.0f;

                    {
                        // 优先渲染带MaterialSlotsComponent的对象（支持多材质）
                        auto materialSlotsView = m_registry.view<TransformComponent, MeshComponent, MaterialSlotsComponent>();
                        for (auto entity : materialSlotsView)
                        {
                            auto &transform = materialSlotsView.get<TransformComponent>(entity);
                            auto &mesh = materialSlotsView.get<MeshComponent>(entity);
                            auto &materialSlots = materialSlotsView.get<MaterialSlotsComponent>(entity);
                            renderer->submitMesh(mesh, materialSlots, transform.getTransform(), (int)entity);
                        }

                        // 渲染带PBR材质的对象（向后兼容）
                        auto pbrView = m_registry.view<TransformComponent, MeshComponent, PBRMaterialComponent>(
                            entt::exclude<MaterialSlotsComponent>);
                        for (auto entity : pbrView)
                        {
                            auto &transform = pbrView.get<TransformComponent>(entity);
                            auto &mesh = pbrView.get<MeshComponent>(entity);
                            auto &pbrMat = pbrView.get<PBRMaterialComponent>(entity);
                            renderer->submitMesh(mesh, pbrMat, transform.getTransform(), (int)entity);
                        }

                        // 渲染带Phong材质的对象（向后兼容）
                        auto phongView = m_registry.view<TransformComponent, MeshComponent, PhongMaterialComponent>(
                            entt::exclude<MaterialSlotsComponent>);
                        for (auto entity : phongView)
                        {
                            auto &transform = phongView.get<TransformComponent>(entity);
                            auto &mesh = phongView.get<MeshComponent>(entity);
                            auto &phongMat = phongView.get<PhongMaterialComponent>(entity);
                            renderer->submitMesh(mesh, phongMat, transform.getTransform(), (int)entity);
                        }

                        // 渲染没有材质组件的对象（使用mesh默认材质）
                        auto defaultView = m_registry.view<TransformComponent, MeshComponent>(
                            entt::exclude<PBRMaterialComponent, PhongMaterialComponent, MaterialSlotsComponent>);
                        for (auto entity : defaultView)
                        {
                            auto &transform = defaultView.get<TransformComponent>(entity);
                            auto &mesh = defaultView.get<MeshComponent>(entity);
                            renderer->submitMesh(mesh, transform.getTransform(), (int)entity);
                        }
                    }
                    // Directional Lights
                    {
                        auto directionalLights = m_registry.group<DirectionalLightComponent>(
                            entt::get<TransformComponent>);
                        for (auto entity : directionalLights)
                        {
                            auto &transform = directionalLights.get<TransformComponent>(entity);
                            auto &directionalLight = directionalLights.get<DirectionalLightComponent>(entity);
                            if (directionalLight.mainLight)
                            {
                                m_environmentLight.directionalLight =
                                    {
                                        .direction = -transform.getForward(),
                                        .color = directionalLight.color,
                                        .intensity = directionalLight.intensity};
                            }
                        }
                    }
                    // Point Lights
                    {
                        auto pointLights = m_registry.group<PointLightComponent>(entt::get<TransformComponent>);
                        m_environmentLight.pointLights.clear();
                        m_environmentLight.pointLights.reserve(pointLights.size());
                        for (auto entity : pointLights)
                        {
                            auto &transform = pointLights.get<TransformComponent>(entity);
                            auto &pointLight = pointLights.get<PointLightComponent>(entity);
                            m_environmentLight.pointLights.push_back(
                                {.position = transform.translation,
                                 .color = pointLight.color,
                                 .intensity = pointLight.intensity,
                                 .range = pointLight.range});
                        }
                    }
                    // Spotlights
                    {
                        auto spotLights = m_registry.group<SpotLightComponent>(entt::get<TransformComponent>);
                        m_environmentLight.spotLights.clear();
                        m_environmentLight.spotLights.reserve(spotLights.size());
                        for (auto entity : spotLights)
                        {
                            auto &transform = spotLights.get<TransformComponent>(entity);
                            auto &spotLight = spotLights.get<SpotLightComponent>(entity);

                            float outerRad = glm::radians(spotLight.angle);
                            float innerRad = outerRad * (1.0f - spotLight.softness);

                            innerRad = glm::clamp(innerRad, 0.0f, outerRad - 0.001f);

                            m_environmentLight.spotLights.push_back(
                                {.position = transform.translation,
                                 .direction = transform.getForward(),
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
                    renderer->drawCircle(transform.getTransform(), circle.color, circle.thickness, circle.fade,
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
        auto view = m_registry.view<ScriptContainerComponent>();
        for (auto e : view)
        {
            Entity entity = {e, this};

            ScriptManager::onUpdateEntity(entity, ts);
        }

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

        m_entityMap[uuid] = entity;
        return entity;
    }

    void Scene::destroyEntity(Entity entity)
    {
        m_entityMap.erase(entity.getUUID());
        m_registry.destroy(entity);
    }

    Entity Scene::duplicateEntity(Entity entity)
    {
        std::string name = entity.getName();
        Entity newEntity = createEntity(name);
        copyComponentIfExists(AllComponents{}, newEntity, entity);
        return newEntity;
    }

    Entity Scene::findEntityByName(std::string_view name)
    {
        auto view = m_registry.view<TagComponent>();
        for (auto entity : view)
        {
            const TagComponent &tc = view.get<TagComponent>(entity);
            if (tc.tag == name)
                return Entity{entity, this};
        }
        return {};
    }

    Entity Scene::getEntityByUUID(UUID uuid)
    {
        if (m_entityMap.find(uuid) != m_entityMap.end())
            return {m_entityMap.at(uuid), this};

        return {};
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
} // namespace Fermion
