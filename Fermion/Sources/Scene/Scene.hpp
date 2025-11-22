#pragma once
#include <entt/entt.hpp>
#include "Core/Timestep.hpp"
#include "Core/UUID.hpp"
#include "Renderer/EditorCamera.hpp"
#include <box2d/box2d.h>
namespace Fermion
{

    class Entity;
    class SceneRenderer;
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

        void onUpdateEditor(std::shared_ptr<SceneRenderer> renderer, Timestep ts, EditorCamera &camera);
        void onUpdateSimulation(std::shared_ptr<SceneRenderer> renderer, Timestep ts, EditorCamera &camera);
        void onUpdateRuntime(std::shared_ptr<SceneRenderer> renderer, Timestep ts);

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

    private:
        void onPhysics2DStart();
        void onPhysics2DStop();
        void onRenderEditor(std::shared_ptr<SceneRenderer> renderer, EditorCamera &camera);

    private:
        entt::registry m_registry;
        uint32_t m_viewportWidth = 0, m_viewportHeight = 0;

        bool m_isRunning = false;
        bool m_isPaused = false;
        int m_stepFrames = 0;

        b2WorldId m_physicsWorld = b2_nullWorldId;
        std::unordered_map<UUID, entt::entity> m_entityMap;
        friend class Entity;
        friend class SceneRenderer;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

}
