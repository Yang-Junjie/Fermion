#pragma once
#include "Scene/Entity.hpp"

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

    private:
        void drawComponents(Entity entity);
        template <typename T>
        void displayAddComponentEntry(const std::string &entryName);
        Entity m_selectedEntity;
    };
} // namespace Fermion