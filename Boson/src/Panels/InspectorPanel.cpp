#include "InspectorPanel.hpp"
#include "Renderer/Model/Mesh.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Components.hpp"
#include "Script/ScriptManager.hpp"
#include "Project/Project.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <entt/entt.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer/Model/MeshFactory.hpp"

namespace Fermion {
    InspectorPanel::InspectorPanel() {
        m_spriteComponentDefaultTexture = Texture2D::create(1, 1);
        uint32_t white = 0xffffffff;
        m_spriteComponentDefaultTexture->setData(&white, sizeof(uint32_t));
    }

    void InspectorPanel::onImGuiRender() {
        ImGui::Begin("Inspector");
        if (m_selectedEntity) {
            drawComponents(m_selectedEntity);
        }
        ImGui::End();
    }

    static bool drawVec3Control(const std::string &label, glm::vec3 &values, float resetValue = 0.0f,
                                float columnWidth = 100.0f) {
        ImGuiIO &io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        bool changed = false;

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
        if (ImGui::Button("X", buttonSize)) {
            values.x = resetValue;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        changed |= ImGui::DragFloat("##X", &values.x, 0.01f);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize)) {
            values.y = resetValue;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        changed |= ImGui::DragFloat("##Y", &values.y, 0.01f);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize)) {
            values.z = resetValue;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        changed |= ImGui::DragFloat("##Z", &values.z, 0.01f);
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        ImGui::Columns(1);
        ImGui::PopID();
        return changed;
    }

    template<typename T, typename UIFunction>
    static void drawComponent(const std::string &name, Entity entity, UIFunction uiFunction) {
        if (!entity.hasComponent<T>())
            return;

        auto &component = entity.getComponent<T>();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
        ImGui::Separator();

        const ImGuiTreeNodeFlags treeNodeFlags =
                ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

        bool open = ImGui::TreeNodeEx((void *) typeid(T).hash_code(), treeNodeFlags, name.c_str());
        ImGui::PopStyleVar();

        bool removeComponent = false;
        if (ImGui::BeginPopupContextItem("ComponentSettings")) {
            if (ImGui::MenuItem("Remove component"))
                removeComponent = true;
            ImGui::EndPopup();
        }

        if (open) {
            uiFunction(component);
            ImGui::TreePop();
        }

        if (removeComponent)
            entity.removeComponent<T>();
    }

    void InspectorPanel::drawComponents(Entity entity) {
        if (entity.hasComponent<TagComponent>()) {
            auto &tag = entity.getComponent<TagComponent>().tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, tag.c_str(), sizeof(buffer));

            if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                tag = std::string(buffer);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("Add Component");
        }
        if (ImGui::BeginPopup("Add Component")) {
            displayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
            displayAddComponentEntry<MeshComponent>("Mesh");
            displayAddComponentEntry<MaterialComponent>("Material");
            displayAddComponentEntry<DirectionalLightComponent>("Directional Light");
            displayAddComponentEntry<PointLightComponent>("Point Light");
            displayAddComponentEntry<SpotLightComponent>("Spot Light");
            displayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
            displayAddComponentEntry<CameraComponent>("Camera");
            displayAddComponentEntry<TextComponent>("Text");

            // displayAddComponentEntry<ScriptComponent>("Script");
            // bool hasScriptComponent = m_selectedEntity.hasComponent<ScriptComponent>();
            // bool hasScriptContainerComponent = m_selectedEntity.hasComponent<ScriptContainerComponent>();
            // if (!hasScriptComponent && !hasScriptContainerComponent)
            // {
            // 	displayAddComponentEntry<ScriptComponent>("Script");
            // 	displayAddComponentEntry<ScriptContainerComponent>("Script Container");
            // }
            displayAddComponentEntry<ScriptContainerComponent>("Scripts");
            displayAddComponentEntry<Rigidbody2DComponent>("Rigidbody2D");
            displayAddComponentEntry<BoxCollider2DComponent>("Box Collider2D");
            displayAddComponentEntry<CircleCollider2DComponent>("Circle Collider2D");
            displayAddComponentEntry<BoxSensor2DComponent>("Box Sensor2D");

            // if (ImGui::MenuItem("Native Script"))
            // {
            // 	m_selectedEntity.addComponent<NativeScriptComponent>().bind<CameraController>();
            // 	ImGui::CloseCurrentPopup();
            // }

            ImGui::EndPopup();
        }

        drawComponent<TransformComponent>("Transform", entity, [](auto &component) {
            drawVec3Control("Translation", component.translation);


            glm::vec3 rotationDeg = glm::degrees(component.getRotationEuler());
            drawVec3Control("Rotation", rotationDeg);
            component.setRotationEuler(glm::radians(rotationDeg));

            drawVec3Control("Scale", component.scale, 1.0f);
        });

        drawComponent<CameraComponent>("Camera", entity, [](auto &component) {
            auto &camera = component.camera;

            ImGui::Checkbox("Primary", &component.primary);

            // Match enum order: Orthographic = 0, Perspective = 1
            const char *projectionTypeStrings[] = {"Orthographic", "Perspective"};
            const char *currentProjectionTypeString = projectionTypeStrings[(int) camera.getProjectionType()];
            if (ImGui::BeginCombo("Projection", currentProjectionTypeString)) {
                for (int i = 0; i < 2; i++) {
                    bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                    if (ImGui::Selectable(projectionTypeStrings[i], isSelected)) {
                        currentProjectionTypeString = projectionTypeStrings[i];
                        camera.setProjectionType((SceneCamera::ProjectionType) i);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (camera.getProjectionType() == SceneCamera::ProjectionType::Perspective) {
                float perspectiveFOV = glm::degrees(camera.getPerspectiveFOV());
                if (ImGui::DragFloat("FOV", &perspectiveFOV)) {
                    camera.setPerspectiveFOV(glm::radians(perspectiveFOV));
                }

                float perspectiveNear = camera.getPerspectiveNearClip();
                if (ImGui::DragFloat("Near", &perspectiveNear)) {
                    camera.setPerspectiveNearClip(perspectiveNear);
                }
                float perspectiveFar = camera.getPerspectiveFarClip();
                if (ImGui::DragFloat("Far", &perspectiveFar)) {
                    camera.setPerspectiveFarClip(perspectiveFar);
                }
            }
            if (camera.getProjectionType() == SceneCamera::ProjectionType::Orthographic) {
                float orthoSize = camera.getOrthographicSize();
                if (ImGui::DragFloat("Size", &orthoSize)) {
                    camera.setOrthographicSize(orthoSize);
                }

                float orthoNear = camera.getOrthographicNearClip();
                if (ImGui::DragFloat("Near", &orthoNear)) {
                    camera.setOrthographicNearClip(orthoNear);
                }
                float orthoFar = camera.getOrthographicFarClip();
                if (ImGui::DragFloat("Far", &orthoFar)) {
                    camera.setOrthographicFarClip(orthoFar);
                }

                ImGui::Checkbox("Fixed Aspect Ratio", &component.fixedAspectRatio);
            }
        });
        drawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [this](auto &component) {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));


            ImTextureID textureID = (ImTextureID) (uintptr_t) m_spriteComponentDefaultTexture->getRendererID();
            ImVec2 imageSize = ImVec2(64.0f, 64.0f);

            ImGui::Text("Texture");
            if (component.texture && component.texture->isLoaded()) {
                textureID = (ImTextureID) (uintptr_t) component.texture->getRendererID();

                float texW = (float) component.texture->getWidth();
                float texH = (float) component.texture->getHeight();
                if (texW > 0.0f && texH > 0.0f) {
                    float maxSize = 64.0f;
                    float scale = std::min(maxSize / texW, maxSize / texH);
                    imageSize = ImVec2(texW * scale, texH * scale);
                }
            } else if (m_spriteComponentDefaultTexture && m_spriteComponentDefaultTexture->isLoaded()) {
                textureID = (ImTextureID) (uintptr_t) m_spriteComponentDefaultTexture->getRendererID();
            }


            ImGui::SameLine();
            if (textureID)
                ImGui::Image(textureID, imageSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

            // Drag & drop to assign texture
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE")) {
                    const char *path = static_cast<const char *>(payload->Data);
                    if (path && path[0]) {
                        auto editorAssets = Project::getEditorAssetManager();
                        AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                        if (static_cast<uint64_t>(handle) != 0) {
                            component.textureHandle = handle;
                            component.texture = editorAssets->getAsset<Texture2D>(handle);
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::DragFloat("Tiling Factor", &component.tilingFactor, 0.1f, 0.0f, 100.0f);
        });
        drawComponent<MeshComponent>("Mesh", entity, [](auto &component) {
            ImGui::Text("Mesh Handle: %s", std::to_string(component.meshHandle).c_str());
            {
                ImGui::Button("Change or Add");
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_MODEL")) {
                        auto path = std::string(static_cast<const char *>(payload->Data));
                        auto editorAssets = Project::getEditorAssetManager();
                        AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                        if (static_cast<uint64_t>(handle) != 0) {
                            component.meshHandle = handle;
                            component.memoryOnly = false;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
            if (ImGui::Button("Add Engine internal Mesh")) {
                ImGui::OpenPopup("mesh_popup");
            }

            ImVec2 popup_pos = ImGui::GetItemRectMin();
            ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Appearing, ImVec2(0.0f, 0.0f));

            if (ImGui::BeginPopup("mesh_popup")) {
                ImGui::Text("Available Meshes:");
                ImGui::Separator();

                if (ImGui::Selectable("Cube")) {
                    component.meshHandle = MeshFactory::GetMemoryMeshHandle(MemoryMeshType::Cube);
                    component.memoryMeshType = MemoryMeshType::Cube;
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Selectable("Sphere")) {
                    component.meshHandle = MeshFactory::GetMemoryMeshHandle(MemoryMeshType::Sphere);
                    component.memoryMeshType = MemoryMeshType::Sphere;
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Selectable("Cylinder")) {
                    component.meshHandle = MeshFactory::GetMemoryMeshHandle(MemoryMeshType::Cylinder);
                    component.memoryMeshType = MemoryMeshType::Cylinder;
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Selectable("Capsule")) {
                    component.meshHandle = MeshFactory::GetMemoryMeshHandle(MemoryMeshType::Capsule);
                    component.memoryMeshType = MemoryMeshType::Capsule;
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Selectable("Cone")) {
                    component.meshHandle = MeshFactory::GetMemoryMeshHandle(MemoryMeshType::Cone);
                    component.memoryMeshType = MemoryMeshType::Cone;
                    ImGui::CloseCurrentPopup();
                }
                component.memoryOnly = true;
                ImGui::EndPopup();
            }
        });
        drawComponent<MaterialComponent>("Material", entity, [](auto &component) {
            bool overrideMaterial = component.overrideMaterial;
            if (ImGui::Checkbox("Override Material", &overrideMaterial)) {
                component.overrideMaterial = overrideMaterial;
            }
            auto materialInstance = component.MaterialInstance;
            glm::vec3 ambient = glm::vec3(materialInstance->getAmbientColor());
            if (drawVec3Control("Ambient", ambient, 0.0f)) {
                materialInstance->setAmbientColor(glm::vec4(ambient, 1.0f));
            }
            glm::vec3 diffuse = glm::vec3(materialInstance->getDiffuseColor());
            if (drawVec3Control("Diffuse", diffuse, 0.0f)) {
                materialInstance->setDiffuseColor(glm::vec4(diffuse, 1.0f));
            }
        });
        drawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto &component) {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ImGui::DragFloat("Intensity", &component.intensity, 0.1f, 0.0f);
            ImGui::Checkbox("Main Light", &component.mainLight);
        });
        drawComponent<PointLightComponent>("Point Light", entity, [](auto &component) {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ImGui::DragFloat("Intensity", &component.intensity, 0.1f, 0.0f);
            ImGui::DragFloat("Range", &component.range, 0.1f, 0.0f);
        });
        drawComponent<SpotLightComponent>("Spot Light", entity, [](auto &c) {
            ImGui::ColorEdit4("Color", glm::value_ptr(c.color));
            ImGui::DragFloat("Intensity", &c.intensity, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Range", &c.range, 0.1f, 0.0f, 1000.0f);

            ImGui::Separator();
            ImGui::Text("Cone");

            ImGui::DragFloat("Angle (deg)", &c.angle, 0.5f, 1.0f, 89.0f);
            ImGui::DragFloat("Softness", &c.softness, 0.01f, 0.0f, 1.0f, "%.3f");
        });

        drawComponent<TextComponent>("Text", entity, [](auto &component) {
            char buffer[1024];
            strncpy(buffer, component.textString.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';

            if (ImGui::InputTextMultiline("Text String", buffer, sizeof(buffer))) {
                component.textString = buffer;
            }
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ImGui::DragFloat("Kerning", &component.kerning, 0.025f);
            ImGui::DragFloat("Line Spacing", &component.lineSpacing, 0.025f);
        });

        // drawComponent<ScriptComponent>("Script", entity, [](auto &component)
        //                                {
        // 	for (auto name : ScriptManager::getALLEntityClasses()){
        // 		size_t dotPos = name.find('.');
        // 		if (dotPos != std::string::npos)
        // 		{
        // 			std::string namespaceName = name.substr(0, dotPos);
        // 			if (namespaceName != "Fermion")
        // 			{
        // 				bool isSelected = (component.className == name);
        // 				if (ImGui::Selectable(name.c_str(), isSelected))
        // 				{
        // 					component.className = name;
        // 				}
        // 			}
        // 		}
        // 	} });
        drawComponent<ScriptContainerComponent>("Scripts Container", entity, [](auto &component) {
            static std::unordered_map<std::string, bool> checkedState;

            auto allClasses = ScriptManager::getALLEntityClasses();

            // 初始化checked状态
            for (auto &name: allClasses) {
                size_t dotPos = name.find('.');
                if (dotPos == std::string::npos)
                    continue;

                std::string namespaceName = name.substr(0, dotPos);
                if (namespaceName == "Fermion")
                    continue;

                if (checkedState.find(name) == checkedState.end())
                    checkedState[name] = false;

                bool isInComponent = std::find(
                                         component.scriptClassNames.begin(),
                                         component.scriptClassNames.end(),
                                         name
                                     ) != component.scriptClassNames.end();

                checkedState[name] = isInComponent;
            }

            // 渲染和更新选中状态
            for (auto &name: allClasses) {
                size_t dotPos = name.find('.');
                if (dotPos == std::string::npos)
                    continue;

                std::string namespaceName = name.substr(0, dotPos);
                if (namespaceName == "Fermion")
                    continue;

                bool checked = checkedState[name];
                std::string label = "##checkbox_" + name;

                if (ImGui::Checkbox(label.c_str(), &checked)) {
                    checkedState[name] = checked;

                    if (checked) {
                        if (std::find(component.scriptClassNames.begin(),
                                      component.scriptClassNames.end(),
                                      name) == component.scriptClassNames.end()) {
                            component.scriptClassNames.push_back(name);
                        }
                    } else {
                        component.scriptClassNames.erase(
                            std::remove(component.scriptClassNames.begin(),
                                        component.scriptClassNames.end(),
                                        name),
                            component.scriptClassNames.end()
                        );
                    }
                }

                ImGui::SameLine();
                ImGui::Text("%s", name.c_str());
            }
        });

        drawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto &component) {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ImGui::DragFloat("Thickness", &component.thickness, 0.025f, 0.0f, 1.0f);
            ImGui::DragFloat("Fade", &component.fade, 0.00025f, 0.0f, 1.0f);
        });

        drawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto &component) {
            const char *bodyTypeStrings[] = {"Static", "Dynamic", "Kinematic"};
            const char *currentBodyTypeString = bodyTypeStrings[(int) component.type];
            if (ImGui::BeginCombo("Body Type", currentBodyTypeString)) {
                for (int i = 0; i < 2; i++) {
                    bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                    if (ImGui::Selectable(bodyTypeStrings[i], isSelected)) {
                        currentBodyTypeString = bodyTypeStrings[i];
                        component.type = (Rigidbody2DComponent::BodyType) i;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            ImGui::Checkbox("Fixed Rotation", &component.fixedRotation);
        });

        drawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto &component) {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.offset));
            ImGui::DragFloat2("Size", glm::value_ptr(component.size));
            ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f);
        });
        drawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto &component) {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.offset));
            ImGui::DragFloat("Radius", &component.radius);
            ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f);
        });
        drawComponent<BoxSensor2DComponent>("Box Sensor 2D", entity, [](auto &component) {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.offset), 0.01f);
            ImGui::DragFloat2("Size", glm::value_ptr(component.size), 0.01f);
        });
    }

    template<typename T>
    inline void InspectorPanel::displayAddComponentEntry(const std::string &entryName) {
        if (!m_selectedEntity.hasComponent<T>()) {
            if (ImGui::MenuItem(entryName.c_str())) {
                m_selectedEntity.addComponent<T>();
                ImGui::CloseCurrentPopup();
            }
        }
    }
} // namespace Fermion
