#include "fmpch.hpp"
#include "SceneHierarchyPanel.hpp"
#include <imgui.h>
#include <imgui_internal.h>

#include <entt/entt.hpp>
#include "Scene/Components.hpp"
#include "Scene/Scene.hpp"
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
		if (ImGui::BeginPopupContextWindow("WindowContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				m_context->createEntity("Empty Entity");
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		if (m_selectedEntity)
		{
			drawComponents(m_selectedEntity);
			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("Add Component");
			}
			if (ImGui::BeginPopup("Add Component"))
			{
				if (ImGui::MenuItem("Sprite Renderer"))
				{
					m_selectedEntity.addComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Camera"))
				{
					m_selectedEntity.addComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
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

		bool enetityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				enetityDeleted = true;
			}
			ImGui::EndPopup();
		}
		if (opend)
		{
			ImGui::TreePop();
		}

		if (enetityDeleted)
		{
			m_context->destroyEntity(entity);
			if (m_selectedEntity == entity)
			{
				m_selectedEntity = {};
			}
		}
	}

	static void drawVec3Control(const std::string &label, glm::vec3 &values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

		float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = ImVec2{lineHeight + 3.0f, lineHeight};

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.01f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.01f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.01f);
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();
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
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap;
		if (entity.hasComponent<TransformComponent>())
		{
			bool open = ImGui::TreeNodeEx((void *)typeid(TransformComponent).hash_code(),
										  treeNodeFlags,
										  "Transform");
			if (open)
			{
				auto &tc = entity.getComponent<TransformComponent>();
				drawVec3Control("Translation", tc.translation);

				glm::vec3 rotation = glm::degrees(tc.rotation);
				drawVec3Control("Rotation", rotation);
				tc.rotation = glm::radians(rotation);

				drawVec3Control("Scale", tc.scale, 1.0f);

				ImGui::TreePop();
			}
		}
		if (entity.hasComponent<CameraComponent>())
		{
			if (ImGui::TreeNodeEx((void *)typeid(CameraComponent).hash_code(),
								  ImGuiTreeNodeFlags_DefaultOpen,
								  "Camera"))
			{
				auto &cameraComponent = entity.getComponent<CameraComponent>();
				auto &camera = cameraComponent.camera;

				ImGui::Checkbox("Primary", &cameraComponent.primary);

				// Match enum order: Orthographic = 0, Perspective = 1
				const char *projectionTypeStrings[] = {"Orthographic", "Perspective"};
				const char *currentProjectionTypeString = projectionTypeStrings[(int)camera.getProjectionType()];
				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.setProjectionType((SceneCamera::ProjectionType)i);
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				if (camera.getProjectionType() == SceneCamera::ProjectionType::Perspective)
				{

					float perspectiveFOV = glm::degrees(camera.getPerspectiveFOV());
					if (ImGui::DragFloat("FOV", &perspectiveFOV))
					{
						camera.setPerspectiveFOV(glm::radians(perspectiveFOV));
					}

					float perspectiveNear = camera.getPerspectiveNearClip();
					if (ImGui::DragFloat("Near", &perspectiveNear))
					{
						camera.setPerspectiveNearClip(perspectiveNear);
					}
					float perspectiveFar = camera.getPerspectiveFarClip();
					if (ImGui::DragFloat("Far", &perspectiveFar))
					{
						camera.setPerspectiveFarClip(perspectiveFar);
					}
				}
				if (camera.getProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.getOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize))
					{
						camera.setOrthographicSize(orthoSize);
					}

					float orthoNear = camera.getOrthographicNearClip();
					if (ImGui::DragFloat("Near", &orthoNear))
					{
						camera.setOrthographicNearClip(orthoNear);
					}
					float orthoFar = camera.getOrthographicFarClip();
					if (ImGui::DragFloat("Far", &orthoFar))
					{
						camera.setOrthographicFarClip(orthoFar);
					}

					ImGui::Checkbox("Fixed Aspect Ratio", &cameraComponent.fixedAspectRatio);
				}
				ImGui::TreePop();
			}
		}
		if (entity.hasComponent<SpriteRendererComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
			bool open = ImGui::TreeNodeEx((void *)typeid(SpriteRendererComponent).hash_code(),
										  treeNodeFlags,
										  "Sprite Renderer");
			ImGui::PopStyleVar();

			ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
			if (ImGui::Button("+"))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeComponent = true;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			if (open)
			{
				auto &spriteRendererComponent = entity.getComponent<SpriteRendererComponent>();
				ImGui::ColorEdit4("Color", glm::value_ptr(spriteRendererComponent.color));
				ImGui::TreePop();
			}
			if (removeComponent)
			{
				entity.removeComponent<SpriteRendererComponent>();
			}
		}
	}
}
