#pragma once
#include "Scene/Entity.hpp"
#include "Renderer/Texture.hpp"
#include <memory>

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

        std::shared_ptr<Texture2D> m_spriteComponentDefaultTexture;
        Entity m_selectedEntity;
        
    };
}