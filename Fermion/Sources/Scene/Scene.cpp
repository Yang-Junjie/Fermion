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
        m_physicsWorld2D = std::make_unique<Physics2DWorld>();
        m_physicsWorld3D = std::make_unique<Physics3DWorld>();
    }

    Scene::~Scene()
    {
        if (m_physicsWorld2D)
            m_physicsWorld2D->stop();
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
        if (m_physicsWorld2D)
            m_physicsWorld2D->start(this);
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
        if (m_physicsWorld2D)
            m_physicsWorld2D->stop();
        if (m_physicsWorld3D)
            m_physicsWorld3D->stop(this);
        ScriptManager::onRuntimeStop();
    }

    void Scene::onSimulationStart()
    {
        if (m_physicsWorld2D)
            m_physicsWorld2D->start(this);
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
        if (m_physicsWorld2D)
            m_physicsWorld2D->stop();
        if (m_physicsWorld3D)
            m_physicsWorld3D->stop(this);
        ScriptManager::onRuntimeStop();
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
                        .intensity = directionalLight.intensity};

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
            if (m_physicsWorld2D)
                m_physicsWorld2D->step(this, ts);
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

        // Physics
        if (m_physicsWorld2D)
            m_physicsWorld2D->step(this, ts);
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
                                .intensity = directionalLight.intensity};

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
