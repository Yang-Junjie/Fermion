#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>
#include "Core/Timestep.hpp"
#include "Core/UUID.hpp"
#include "Renderer/Model/Mesh.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Components.hpp"

namespace Fermion
{
    class Entity;
    class EntityManager;
    class SceneRenderer;
    class Physics2DWorld;
    class Physics3DWorld;

    struct SceneEnvironmentSettings
    {
        bool showSkybox = true;
        bool enableShadows = true;
        float ambientIntensity = 0.1f;

        uint32_t shadowMapSize = 2048;
        float shadowBias = 0.01f;
        float shadowSoftness = 1.0f;
        float normalMapStrength = 1.0f;
        float toksvigStrength = 1.0f;

        bool useIBL = true;
    };

    struct PointLight
    {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;
        float range = 10.0f;
    };

    struct SpotLight
    {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 direction{0.0f, -1.0f, 0.0f};

        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;

        float range = 10.0f;

        float innerConeAngle = glm::radians(15.0f);
        float outerConeAngle = glm::radians(25.0f);
    };

    struct DirectionalLight
    {
        glm::vec3 direction{0.0f, -1.0f, 0.0f};
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;
    };

    struct EnvironmentLight
    {
        std::vector<DirectionalLight> directionalLights;
        std::vector<PointLight> pointLights;
        std::vector<SpotLight> spotLights;
    };

    class Scene
    {
    public:
        Scene();

        ~Scene();

        static std::shared_ptr<Scene> copy(std::shared_ptr<Scene> other);

        void onRuntimeStart();
        void onRuntimeStop();
        void onSimulationStart();
        void onSimulationStop();

        void onUpdateEditor(std::shared_ptr<SceneRenderer> renderer,
                            Timestep ts, EditorCamera &camera,
                            bool showRenderEntities = true);

        void onUpdateSimulation(std::shared_ptr<SceneRenderer> renderer,
                                Timestep ts, EditorCamera &camera,
                                bool showRenderEntities = true);

        void onUpdateRuntime(std::shared_ptr<SceneRenderer> renderer,
                             Timestep ts,
                             bool showRenderEntities = true);

        void onViewportResize(uint32_t width, uint32_t height);

        Entity createEntity(std::string name = std::string());
        Entity createChildEntity(Entity parent, std::string name = std::string());
        Entity createEntityWithUUID(UUID uuid, std::string name = std::string());

        uint32_t getViewportWidth() const { return m_viewportWidth; }
        uint32_t getViewportHeight() const { return m_viewportHeight; }

        bool isRunning() const { return m_isRunning; }
        bool isPaused() const { return m_isPaused; }
        void setPaused(bool paused) { m_isPaused = paused; }

        void step(int frames = 1);

        EntityManager &getEntityManager();
        const EntityManager &getEntityManager() const;

        entt::registry &getRegistry();
        const entt::registry &getRegistry() const;

        template <typename... Components>
        auto getAllEntitiesWith()
        {
            return getRegistry().view<Components...>();
        }

        Physics2DWorld *getPhysicsWorld2D() const
        {
            return m_physicsWorld2D.get();
        }

        Physics3DWorld *getPhysicsWorld3D() const
        {
            return m_physicsWorld3D.get();
        }

        SceneEnvironmentSettings &getEnvironmentSettings() { return m_environmentSettings; }
        const SceneEnvironmentSettings &getEnvironmentSettings() const { return m_environmentSettings; }

    private:
        void onScriptStart(Timestep ts);

        void onRenderEditor(std::shared_ptr<SceneRenderer> renderer, EditorCamera &camera,
                            bool showRenderEntities = true);

    private:
        std::unique_ptr<EntityManager> m_entityManager;

        uint32_t m_viewportWidth = 0, m_viewportHeight = 0;

        bool m_isRunning = false;
        bool m_isPaused = false;
        int m_stepFrames = 0;

        EnvironmentLight m_environmentLight;
        SceneEnvironmentSettings m_environmentSettings;

        bool m_hasDirectionalLight = false;
        std::shared_ptr<Texture2D> m_lightTexture = nullptr, m_cameraTexture = nullptr;

        std::unique_ptr<Physics2DWorld> m_physicsWorld2D;
        std::unique_ptr<Physics3DWorld> m_physicsWorld3D;

        friend class Entity;
        friend class SceneRenderer;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class Physics2DWorld;
        friend class Physics3DWorld;
    };
} // namespace Fermion
