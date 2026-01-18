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
            auto view = m_contextScene->getRegistry().view<RelationshipComponent>();
            for (auto entityID : view)
            {
                Entity entity{entityID, m_contextScene.get()};
                Entity parent = m_contextScene->getEntityManager().tryGetEntityByUUID(entity.getParentUUID());
                if (!parent)
                {
                    drawEntityNode(entity);
                }
            }
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
        {
            m_selectedEntity = {};
            m_inspectorPanel.setSelectedEntity(Entity());
        }

        if (ImGui::BeginPopupContextWindow("WindowContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (m_contextScene && ImGui::MenuItem("Create Empty Entity"))
            {
                m_contextScene->createEntity("Empty Entity");
            }
            ImGui::EndPopup();
        }

        if (m_editingEnabled && m_contextScene && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        {
            ImGuiWindow *window = ImGui::GetCurrentWindow();
            ImRect dropRect(window->InnerRect.Min, window->InnerRect.Max);
            if (ImGui::BeginDragDropTargetCustom(dropRect, window->ID))
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_ENTITY"))
                {
                    uint64_t droppedId = *(const uint64_t *)payload->Data;
                    Entity droppedEntity = m_contextScene->getEntityManager().tryGetEntityByUUID(UUID(droppedId));
                    if (droppedEntity)
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
        const char *label = "Entity";
        if (entity.hasComponent<TagComponent>())
        {
            label = entity.getComponent<TagComponent>().tag.c_str();
        }

        std::vector<Entity> childEntities;
        childEntities.reserve(entity.getChildren().size());
        for (UUID childId : entity.getChildren())
        {
            Entity child = m_contextScene->getEntityManager().tryGetEntityByUUID(childId);
            if (child)
                childEntities.emplace_back(child);
        }

        const bool hasChildren = !childEntities.empty();
        ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
                                   ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                   ImGuiTreeNodeFlags_SpanAvailWidth;
        if (!hasChildren)
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        bool opend = ImGui::TreeNodeEx((void *)(uint64_t)(uint32_t)entity, flags, label);
        if (m_editingEnabled && ImGui::IsItemClicked())
        {
            m_selectedEntity = entity;
            m_inspectorPanel.setSelectedEntity(entity);
        }

        if (m_editingEnabled && ImGui::BeginDragDropSource())
        {
            uint64_t entityId = static_cast<uint64_t>(entity.getUUID());
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
                if (droppedEntity && droppedEntity != entity && !isDescendant(entity, droppedEntity))
                {
                    if (droppedEntity.getParent() != entity)
                    {
                        m_contextScene->getEntityManager().convertToWorldSpace(droppedEntity);
                        droppedEntity.setParent(entity);
                        m_contextScene->getEntityManager().convertToLocalSpace(droppedEntity);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        bool enetityDeleted = false;
        bool createChildEntity = false;
        if (m_editingEnabled && ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Create Child"))
            {
                createChildEntity = true;
            }
            if (ImGui::MenuItem("Delete Entity"))
            {
                enetityDeleted = true;
            }
            ImGui::EndPopup();
        }
        if (opend && hasChildren)
        {
            for (const Entity &child : childEntities)
            {
                drawEntityNode(child);
            }
            ImGui::TreePop();
        }

        if (createChildEntity)
        {
            Entity child = m_contextScene->createChildEntity(entity, "Empty Entity");
            m_selectedEntity = child;
            m_inspectorPanel.setSelectedEntity(child);
        }

        if (enetityDeleted)
        {
            m_contextScene->getEntityManager().destroyEntity(entity);
            if (m_selectedEntity == entity)
            {
                m_selectedEntity = {};
                m_inspectorPanel.setSelectedEntity(Entity());
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
