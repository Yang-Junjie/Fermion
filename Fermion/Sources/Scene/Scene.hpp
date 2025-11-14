#pragma once
#include <entt/entt.hpp>
#include "Core/Timestep.hpp"
#include "Renderer/EditorCamera.hpp"

namespace Fermion
{

    class Entity;
    class Scene
    {
    public:
        Scene();
        ~Scene();
        
        void onUpdateEditor(Timestep ts, EditorCamera &camera);
        void onUpdateRuntime(Timestep ts);
        void onViewportResize(uint32_t width, uint32_t height);

        Entity createEntity(std::string name = std::string());
        void destroyEntity(Entity entity);

        uint32_t getViewportWidth() const 
        {
            return m_viewportWidth;
        }

        uint32_t getViewportHeight()const
        {
            return m_viewportHeight;
        }

        Entity getPrimaryCameraEntity();
    private:
       
      

    private:
        entt::registry m_registry;
        uint32_t m_viewportWidth = 0, m_viewportHeight = 0;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

}