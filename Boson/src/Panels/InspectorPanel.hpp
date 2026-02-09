#pragma once
#include "Scene/Entity.hpp"

#include <optional>
#include <functional>

namespace Fermion
{
    class InspectorPanel
    {
    public:
        InspectorPanel();
        void onImGuiRender();
        void setSelectedEntity(const Entity &entity)
        {
            m_selectedEntity = entity;
        }
        void setContext(const std::shared_ptr<Scene> &scene)
        {
            m_contextScene = scene;
        }

        // Entity picking for joint connections etc.
        bool isEntityPickingActive() const { return m_entityPickingActive; }
        void deliverPickedEntity(Entity pickedEntity);
        void cancelEntityPicking();

    private:
        void drawComponents(Entity entity);
        template <typename T>
        void displayAddComponentEntry(const std::string &entryName);

        Entity m_selectedEntity;
        std::shared_ptr<Scene> m_contextScene;

        // Entity picking state
        bool m_entityPickingActive = false;
        Entity m_pickingTargetEntity;
    };
} // namespace Fermion