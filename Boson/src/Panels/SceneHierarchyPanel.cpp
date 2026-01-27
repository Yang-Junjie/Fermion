#include "fmpch.hpp"
#include "SceneHierarchyPanel.hpp"
#include "Scene/Scene.hpp"
#include "Scene/EntityManager.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <entt/entt.hpp>

namespace Fermion
{
    SceneHierarchyPanel::SceneHierarchyPanel()
    {
    }

    SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &scene) : m_contextScene(scene)
    {
    }
    void SceneHierarchyPanel::setContext(const std::shared_ptr<Scene> &scene)
    {
        m_contextScene = scene;
        m_selectedEntity = {};
        m_inspectorPanel.setSelectedEntity({});
    }
    void SceneHierarchyPanel::onImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

        if (m_contextScene)
        {
            // static char buffer[256];
            // memset(buffer, 0, sizeof(buffer));
            // ImGui::InputTextWithHint("##Search", "Search...", buffer, sizeof(buffer));
            // ImGui::Separator();

            auto view = m_contextScene->getRegistry().view<RelationshipComponent>();
            for (auto entityID : view)
            {
                Entity entity{entityID, m_contextScene.get()};

                if (!entity.getParent())
                {
                    drawEntityNode(entity);
                }
            }
            if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
            {
                m_selectedEntity = {};
                m_inspectorPanel.setSelectedEntity({});
            }

            if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                {
                    m_contextScene->createEntity("Empty Entity");
                }
                ImGui::EndPopup();
            }

            if (m_editingEnabled && ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_ENTITY"))
                {
                    uint64_t droppedId = *(const uint64_t *)payload->Data;
                    Entity droppedEntity = m_contextScene->getEntityManager().tryGetEntityByUUID(UUID(droppedId));

                    if (droppedEntity && droppedEntity.getParent())
                    {
                        m_contextScene->getEntityManager().convertToWorldSpace(droppedEntity);
                        droppedEntity.setParent(Entity{}); 
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }

        ImGui::End();


        m_inspectorPanel.onImGuiRender();
    }

    void SceneHierarchyPanel::setSelectedEntity(Entity entity)
    {
        m_selectedEntity = entity;
        m_inspectorPanel.setSelectedEntity(entity);
    }
    void SceneHierarchyPanel::setEditingEnabled(bool enabled)
    {
        // m_editingEnabled = enabled;
        // if (!m_editingEnabled)
        // {
        // 	m_selectedEntity = {};
        // 	m_inspectorPanel.setSelectedEntity({});
        // }
    }

    void SceneHierarchyPanel::drawEntityNode(Entity entity)
    {

        const char *label = entity.hasComponent<TagComponent>()
                                ? entity.getComponent<TagComponent>().tag.c_str()
                                : "Unnamed Entity";

        const bool hasChildren = !entity.getChildren().empty();
        ImGuiTreeNodeFlags flags = (m_selectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0);
        flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        bool opened = ImGui::TreeNodeEx((void *)(uint64_t)entity.getUUID(), flags, "%s", label);

        if (ImGui::IsItemClicked())
        {
            m_selectedEntity = entity;
            m_inspectorPanel.setSelectedEntity(entity);
        }

        if (m_editingEnabled && ImGui::BeginDragDropSource())
        {
            uint64_t entityId = (uint64_t)entity.getUUID();
            ImGui::SetDragDropPayload("FERMION_ENTITY", &entityId, sizeof(uint64_t));
            ImGui::Text("%s", label);
            ImGui::EndDragDropSource();
        }

        if (m_editingEnabled && ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_ENTITY"))
            {
                uint64_t droppedId = *(const uint64_t *)payload->Data;
                Entity droppedEntity = m_contextScene->getEntityManager().tryGetEntityByUUID(UUID(droppedId));

                if (droppedEntity && droppedEntity != entity &&
                    droppedEntity.getParent() != entity && !isDescendant(entity, droppedEntity))
                {
                    m_contextScene->getEntityManager().convertToWorldSpace(droppedEntity);
                    droppedEntity.setParent(entity);
                    m_contextScene->getEntityManager().convertToLocalSpace(droppedEntity);
                }
            }
            ImGui::EndDragDropTarget();
        }

        // 6. 右键菜单
        bool entityDeleted = false;
        if (m_editingEnabled && ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Create Child"))
                m_contextScene->createChildEntity(entity, "Empty Entity");

            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened && hasChildren)
        {
            for (UUID childId : entity.getChildren())
            {
                Entity child = m_contextScene->getEntityManager().tryGetEntityByUUID(childId);
                if (child)
                {
                    drawEntityNode(child);
                }
            }
            ImGui::TreePop();
        }
        if (entityDeleted)
        {
            m_contextScene->getEntityManager().destroyEntity(entity);
            if (m_selectedEntity == entity)
            {
                m_selectedEntity = {};
                m_inspectorPanel.setSelectedEntity({});
            }
        }
    }

    bool SceneHierarchyPanel::isDescendant(Entity entity, Entity potentialAncestor) const
    {
        if (!m_contextScene || !entity || !potentialAncestor)
        {
            return false;
        }

        Entity current = entity;
        while (current)
        {
            UUID parentId = current.getParentUUID();
            if (static_cast<uint64_t>(parentId) == 0)
                break;

            Entity parent = m_contextScene->getEntityManager().tryGetEntityByUUID(parentId);
            if (!parent)
                break;

            if (parent == potentialAncestor)
                return true;

            current = parent;
        }

        return false;
    }

} // namespace Fermion
