#pragma once
#include <entt/entt.hpp>
#include "Core/Timestep.hpp"
#include "Core/UUID.hpp"
#include "Renderer/Model/Mesh.hpp"
#include "Renderer/Model/Material.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Components.hpp"
#include <box2d/box2d.h>
namespace Fermion
{

    class Entity;
    class SceneRenderer;

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
    struct EnvironmentLight
    {
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

        void onUpdateEditor(std::shared_ptr<SceneRenderer> renderer, Timestep ts, EditorCamera &camera, bool showRenderEntities = true);
        void onUpdateSimulation(std::shared_ptr<SceneRenderer> renderer, Timestep ts, EditorCamera &camera, bool showRenderEntities = true);
        void onUpdateRuntime(std::shared_ptr<SceneRenderer> renderer, Timestep ts, bool showRenderEntities = true);

        void onViewportResize(uint32_t width, uint32_t height);

        Entity createEntity(std::string name = std::string());
        Entity createEntityWithUUID(UUID uuid, std::string name = std::string());
        void destroyEntity(Entity entity);

        Entity duplicateEntity(Entity entity);
        Entity findEntityByName(std::string_view name);
        Entity getEntityByUUID(UUID uuid);

        uint32_t getViewportWidth() const
        {
            return m_viewportWidth;
        }

        uint32_t getViewportHeight() const
        {
            return m_viewportHeight;
        }

        Entity getPrimaryCameraEntity();

        bool isRunning() const { return m_isRunning; }
        bool isPaused() const { return m_isPaused; }
        void setPaused(bool paused) { m_isPaused = paused; }
        void step(int frames = 1);

        template <typename... Components>
        auto getAllEntitiesWith()
        {
            return m_registry.view<Components...>();
        }
        void initPhysicsSensor(Entity entity);
        b2WorldId getPhysicsWorld() const { return m_physicsWorld; }
        bool isPhysicsWorldValid() const { return B2_IS_NON_NULL(m_physicsWorld); }

    private:
        void onPhysics2DStart();
        void onPhysics2DStop();

        void onScriptStart(Timestep ts);
        void onRenderEditor(std::shared_ptr<SceneRenderer> renderer, EditorCamera &camera, bool showRenderEntities = true);
    
    private:
        entt::registry m_registry;
        uint32_t m_viewportWidth = 0, m_viewportHeight = 0;

        bool m_isRunning = false;
        bool m_isPaused = false;
        int m_stepFrames = 0;

        EnvironmentLight m_environmentLight;
        b2WorldId m_physicsWorld = b2_nullWorldId;
        std::unordered_map<UUID, entt::entity> m_entityMap;
        std::unordered_map<UUID, b2BodyId> m_physicsBodyMap;
        friend class Entity;
        friend class SceneRenderer;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

}
