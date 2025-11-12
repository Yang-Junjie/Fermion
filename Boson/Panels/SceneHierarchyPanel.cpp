#include "fmpch.hpp"
#include "SceneHierarchyPanel.hpp"
#include <imgui.h>

#include <entt/entt.hpp>
#include "Scene/Components.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace Fermion
{
	SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &scene) : m_context(scene)
	{
	}
	void SceneHierarchyPanel::setContext(const std::shared_ptr<Scene> &scene)
	{
		m_context = scene;
	}
	void SceneHierarchyPanel::onImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (m_context)
		{
			auto view = m_context->m_registry.view<TagComponent>();
			for (auto entityID : view)
			{
				Entity entity{entityID, m_context.get()};
				drawEntityNode(entity);
			}
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_selectedEntity = {};
		}
		ImGui::End();
		
		ImGui::Begin("Properties");
		if (m_selectedEntity)
		{
			drawComponents(m_selectedEntity);
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::drawEntityNode(Entity entity)
	{
		auto &tag = entity.getComponent<TagComponent>().tag;

		ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opend = ImGui::TreeNodeEx((void *)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_selectedEntity = entity;
		}

		if (opend)
		{
			ImGui::TreePop();
		}
	}
	void SceneHierarchyPanel::drawComponents(Entity entity)
	{
		if (entity.hasComponent<TagComponent>())
		{
			auto &tag = entity.getComponent<TagComponent>().tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, tag.c_str(), sizeof(buffer));

			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}
		if (entity.hasComponent<TransformComponent>())
		{
			if (ImGui::TreeNodeEx((void *)typeid(TransformComponent).hash_code(),
								  ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow |
									  ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth,
								  "Transform"))
			{
				auto &transform = entity.getComponent<TransformComponent>().transform;
				ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);

				ImGui::TreePop();
			}
		}
	}
}
