#include "fmpch.hpp"
#include "SceneHierarchyPanel.hpp"
#include "Scene/Scene.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <entt/entt.hpp>

namespace Fermion {
SceneHierarchyPanel::SceneHierarchyPanel() {
}

SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &scene) : m_contextScene(scene) {
}
void SceneHierarchyPanel::setContext(const std::shared_ptr<Scene> &scene) {
    m_contextScene = scene;
    m_selectedEntity = {};
    m_inspectorPanel.setSelectedEntity({});
}
void SceneHierarchyPanel::onImGuiRender() {
    ImGui::Begin("Scene Hierarchy");

    if (m_contextScene) {
        auto view = m_contextScene->m_registry.view<TagComponent>();
        for (auto entityID : view) {
            Entity entity{entityID, m_contextScene.get()};
            drawEntityNode(entity);
        }
    }

    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
        m_selectedEntity = {};
        m_inspectorPanel.setSelectedEntity(Entity());
    }
    if (ImGui::BeginPopupContextWindow("WindowContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::MenuItem("Create Empty Entity")) {
            m_contextScene->createEntity("Empty Entity");
        }
        ImGui::EndPopup();
    }

    ImGui::End();

    m_inspectorPanel.onImGuiRender();
}

void SceneHierarchyPanel::setSelectedEntity(Entity entity) {
    m_selectedEntity = entity;
    m_inspectorPanel.setSelectedEntity(entity);
}
void SceneHierarchyPanel::setEditingEnabled(bool enabled) {
    // m_editingEnabled = enabled;
    // if (!m_editingEnabled)
    // {
    // 	m_selectedEntity = {};
    // 	m_inspectorPanel.setSelectedEntity({});
    // }
}

void SceneHierarchyPanel::drawEntityNode(Entity entity) {
    auto &tag = entity.getComponent<TagComponent>().tag;

    ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
    bool opend = ImGui::TreeNodeEx((void *)(uint64_t)(uint32_t)entity, flags, tag.c_str());
    if (m_editingEnabled && ImGui::IsItemClicked()) {
        m_selectedEntity = entity;
        m_inspectorPanel.setSelectedEntity(entity);
    }

    bool enetityDeleted = false;
    if (m_editingEnabled && ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete Entity")) {
            enetityDeleted = true;
        }
        ImGui::EndPopup();
    }
    if (opend) {
        ImGui::TreePop();
    }

    if (enetityDeleted) {
        m_contextScene->destroyEntity(entity);
        if (m_selectedEntity == entity) {
            m_selectedEntity = {};
            m_inspectorPanel.setSelectedEntity(Entity());
        }
    }
}

} // namespace Fermion
