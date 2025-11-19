#include "fmpch.hpp"
#include "SceneHierarchyPanel.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include "Core/Input.hpp"

#include <entt/entt.hpp>
#include "Scene/Components.hpp"
#include "Scene/Scene.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Asset/AssetManager.hpp"

namespace Fermion
{
	SceneHierarchyPanel::SceneHierarchyPanel()
	{
		m_spriteComponentDefaultTexture = Texture2D::create(1, 1);
		uint32_t white = 0xffffffff;
		m_spriteComponentDefaultTexture->setData(&white, sizeof(uint32_t));
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &scene) : m_contextScene(scene)
	{
		m_spriteComponentDefaultTexture = Texture2D::create(1, 1);
		uint32_t white = 0xffffffff;
		m_spriteComponentDefaultTexture->setData(&white, sizeof(uint32_t));
	}
	void SceneHierarchyPanel::setContext(const std::shared_ptr<Scene> &scene)
	{
		m_contextScene = scene;
		m_selectedEntity = {};
	}
	void SceneHierarchyPanel::onImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (m_contextScene)
		{
			auto view = m_contextScene->m_registry.view<TagComponent>();
			for (auto entityID : view)
			{
				Entity entity{entityID, m_contextScene.get()};
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
				m_contextScene->createEntity("Empty Entity");
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		if (m_selectedEntity)
		{
			drawComponents(m_selectedEntity);
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::setSelectedEntity(Entity entity)
	{
		m_selectedEntity = entity;
	}
	void SceneHierarchyPanel::setEditingEnabled(bool enabled)
	{
		m_editingEnabled = enabled;
		if (!m_editingEnabled)
			m_selectedEntity = {};
	}

	void SceneHierarchyPanel::drawEntityNode(Entity entity)
	{
		auto &tag = entity.getComponent<TagComponent>().tag;

		ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opend = ImGui::TreeNodeEx((void *)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (m_editingEnabled && ImGui::IsItemClicked())
		{
			m_selectedEntity = entity;
		}

		bool enetityDeleted = false;
		if (m_editingEnabled && ImGui::BeginPopupContextItem())
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
			m_contextScene->destroyEntity(entity);
			if (m_selectedEntity == entity)
			{
				m_selectedEntity = {};
			}
		}
	}

	static void drawVec3Control(const std::string &label, glm::vec3 &values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO &io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

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
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.01f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.01f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.01f);
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();
	}

	template <typename T, typename UIFunction>
	static void drawComponent(const std::string &name, Entity entity, UIFunction uiFunction)
	{
		if (!entity.hasComponent<T>())
			return;

		auto &component = entity.getComponent<T>();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
		ImGui::Separator();

		const ImGuiTreeNodeFlags treeNodeFlags =
			ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_Framed |
			ImGuiTreeNodeFlags_FramePadding;

		bool open = ImGui::TreeNodeEx((void *)typeid(T).hash_code(), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();

		bool removeComponent = false;
		if (ImGui::BeginPopupContextItem("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove component"))
				removeComponent = true;
			ImGui::EndPopup();
		}

		if (open)
		{
			uiFunction(component);
			ImGui::TreePop();
		}

		if (removeComponent)
			entity.removeComponent<T>();
	}

	void SceneHierarchyPanel::drawComponents(Entity entity)
	{
		if (entity.hasComponent<TagComponent>())
		{
			auto &tag = entity.getComponent<TagComponent>().tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, tag.c_str(), sizeof(buffer));

			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("Add Component");
		}
		if (ImGui::BeginPopup("Add Component"))
		{
			if (!m_selectedEntity.hasComponent<SpriteRendererComponent>())
			{
				if (ImGui::MenuItem("Sprite Renderer"))
				{

					m_selectedEntity.addComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!m_selectedEntity.hasComponent<CircleRendererComponent>())
			{
				if (ImGui::MenuItem("Circle Renderer"))
				{
					m_selectedEntity.addComponent<CircleRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!m_selectedEntity.hasComponent<CameraComponent>())
			{
				if (ImGui::MenuItem("Camera"))
				{
					m_selectedEntity.addComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!m_selectedEntity.hasComponent<TextComponent>())
			{
				if (ImGui::MenuItem("Text"))
				{
					m_selectedEntity.addComponent<TextComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!m_selectedEntity.hasComponent<Rigidbody2DComponent>())
			{
				if (ImGui::MenuItem("Rigidbody2D"))
				{
					m_selectedEntity.addComponent<Rigidbody2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_selectedEntity.hasComponent<BoxCollider2DComponent>())
			{
				if (ImGui::MenuItem("Box Collider2D"))
				{
					m_selectedEntity.addComponent<BoxCollider2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!m_selectedEntity.hasComponent<CircleCollider2DComponent>())
			{
				if (ImGui::MenuItem("Circle Collider2D"))
				{
					m_selectedEntity.addComponent<CircleCollider2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			// if (ImGui::MenuItem("Native Script"))
			// {
			// 	m_selectedEntity.addComponent<NativeScriptComponent>().bind<CameraController>();
			// 	ImGui::CloseCurrentPopup();
			// }

			ImGui::EndPopup();
		}

		drawComponent<TransformComponent>("Transform", entity, [](auto &component)
										  {
			drawVec3Control("Translation", component.translation);


			glm::vec3 rotationDeg = glm::degrees(component.getRotationEuler());
    		drawVec3Control("Rotation", rotationDeg);          
    		component.setRotationEuler(glm::radians(rotationDeg)); 

			drawVec3Control("Scale", component.scale, 1.0f); });

		drawComponent<CameraComponent>("Camera", entity, [](auto &component)
									   {
			auto &camera = component.camera;

				ImGui::Checkbox("Primary", &component.primary);

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

					ImGui::Checkbox("Fixed Aspect Ratio", &component.fixedAspectRatio);
				} });
		drawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [this](auto &component)
											   {
			ImGui::ColorEdit4("Color", glm::value_ptr(component.color));


			ImTextureID textureID =  (ImTextureID)(uintptr_t)m_spriteComponentDefaultTexture->getRendererID();
			ImVec2 imageSize = ImVec2(64.0f, 64.0f);

			ImGui::Text("Texture");
			if (component.texture && component.texture->isLoaded())
			{
				textureID = (ImTextureID)(uintptr_t)component.texture->getRendererID();

				float texW = (float)component.texture->getWidth();
				float texH = (float)component.texture->getHeight();
				if (texW > 0.0f && texH > 0.0f)
				{
					float maxSize = 64.0f;
					float scale = std::min(maxSize / texW, maxSize / texH);
					imageSize = ImVec2(texW * scale, texH * scale);
				}
			}
			else if (m_spriteComponentDefaultTexture && m_spriteComponentDefaultTexture->isLoaded())
			{
				textureID = (ImTextureID)(uintptr_t)m_spriteComponentDefaultTexture->getRendererID();
			}

			
			ImGui::SameLine();
			if (textureID)
				ImGui::Image(textureID, imageSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

			// Drag & drop to assign texture
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE"))
				{
					const char *path = static_cast<const char *>(payload->Data);
					if (path && path[0])
					{
						AssetHandle handle = AssetManager::importAsset(std::filesystem::path(path));
						if (handle != AssetHandle{})
						{
							component.texture = AssetManager::getAsset<Texture2D>(handle);
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::DragFloat("Tiling Factor", &component.tilingFactor, 0.1f, 0.0f, 100.0f); });
		drawComponent<TextComponent>("Text", entity, [](auto &component)
									 {
			char buffer[1024]; 
			strncpy(buffer, component.textString.c_str(), sizeof(buffer));
			buffer[sizeof(buffer)-1] = '\0';

			if (ImGui::InputTextMultiline("Text String", buffer, sizeof(buffer)))
			{
				component.textString = buffer; 
			}
			ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
			ImGui::DragFloat("Kerning", &component.kerning, 0.025f);
			ImGui::DragFloat("Line Spacing", &component.lineSpacing, 0.025f); });
		drawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto &component)
											   {
			ImGui::ColorEdit4("Color", glm::value_ptr(component.color)); 
			ImGui::DragFloat("Thickness", &component.thickness, 0.025f, 0.0f, 1.0f);
			ImGui::DragFloat("Fade", &component.fade, 0.00025f, 0.0f, 1.0f); });

		drawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto &component)
											{
			const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic"};
			const char* currentBodyTypeString = bodyTypeStrings[(int)component.type];
			if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
					if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
					{
						currentBodyTypeString = bodyTypeStrings[i];
						component.type = (Rigidbody2DComponent::BodyType)i;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::Checkbox("Fixed Rotation", &component.fixedRotation); });

		drawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto &component)
											  {
			ImGui::DragFloat2("Offset", glm::value_ptr(component.offset));
			ImGui::DragFloat2("Size", glm::value_ptr(component.size));
			ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f); });
		drawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto &component)
												 {
			ImGui::DragFloat2("Offset", glm::value_ptr(component.offset));
			ImGui::DragFloat("Radius", &component.radius);
			ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f); });
	}
}
