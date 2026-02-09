#pragma once
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include "InspectorPanel.hpp"

namespace Fermion {
    class SceneHierarchyPanel {
    public:
        SceneHierarchyPanel();

        SceneHierarchyPanel(const std::shared_ptr<Scene> &scene);

        void setContext(const std::shared_ptr<Scene> &scene);

        void onImGuiRender();

        Entity getSelectedEntity() const {
            return m_selectedEntity;
        }

        void setSelectedEntity(Entity entity);

        void setEditingEnabled(bool enabled);

        // Forward entity picking API from InspectorPanel
        bool isEntityPickingActive() const { return m_inspectorPanel.isEntityPickingActive(); }
        void deliverPickedEntity(Entity entity) { m_inspectorPanel.deliverPickedEntity(entity); }
        void cancelEntityPicking() { m_inspectorPanel.cancelEntityPicking(); }

    private:
        void drawEntityNode(Entity entity);
        bool isDescendant(Entity entity, Entity potentialAncestor) const;

        // void drawComponents(Entity entity);
        // template <typename T>
        // void displayAddComponentEntry(const std::string &entryName);

    private:
        InspectorPanel m_inspectorPanel;
        std::shared_ptr<Scene> m_contextScene;
        Entity m_selectedEntity;
        bool m_editingEnabled = true;
    };
} // namespace Fermion
