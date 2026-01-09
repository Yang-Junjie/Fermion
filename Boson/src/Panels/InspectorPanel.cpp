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

namespace Fermion
{
    InspectorPanel::InspectorPanel()
    {
        m_spriteComponentDefaultTexture = Texture2D::create(1, 1);
        uint32_t white = 0xffffffff;
        m_spriteComponentDefaultTexture->setData(&white, sizeof(uint32_t));
    }

    void InspectorPanel::onImGuiRender()
    {
        ImGui::Begin("Inspector");
        if (m_selectedEntity)
        {
            drawComponents(m_selectedEntity);
        }
        ImGui::End();
    }

    static bool drawVec3Control(const std::string &label, glm::vec3 &values, float resetValue = 0.0f,
                                float columnWidth = 100.0f, float dragSpeed = 0.1f)
    {
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
        if (ImGui::Button("X", buttonSize))
        {
            values.x = resetValue;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        changed |= ImGui::DragFloat("##X", &values.x, dragSpeed);
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
        changed |= ImGui::DragFloat("##Y", &values.y, dragSpeed);
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
        changed |= ImGui::DragFloat("##Z", &values.z, dragSpeed);
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        ImGui::Columns(1);
        ImGui::PopID();
        return changed;
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
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

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

    void InspectorPanel::drawComponents(Entity entity)
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
            ImGui::SeparatorText("2D Component");
            displayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
            displayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
            displayAddComponentEntry<TextComponent>("Text");
            displayAddComponentEntry<Rigidbody2DComponent>("Rigidbody2D");
            displayAddComponentEntry<BoxCollider2DComponent>("Box Collider2D");
            displayAddComponentEntry<CircleCollider2DComponent>("Circle Collider2D");
            displayAddComponentEntry<BoxSensor2DComponent>("Box Sensor2D");
            ImGui::SeparatorText("3D Component");
            displayAddComponentEntry<MeshComponent>("Mesh");
            displayAddComponentEntry<Rigidbody3DComponent>("Rigidbody3D");
            displayAddComponentEntry<BoxCollider3DComponent>("Box Collider3D");
            displayAddComponentEntry<DirectionalLightComponent>("Directional Light");
            displayAddComponentEntry<PointLightComponent>("Point Light");
            displayAddComponentEntry<SpotLightComponent>("Spot Light");

            if (!(entity.hasComponent<PhongMaterialComponent>() || entity.hasComponent<PBRMaterialComponent>() || entity.hasComponent<MaterialSlotsComponent>()))
            {
                ImGui::SeparatorText("Materials");
                displayAddComponentEntry<PhongMaterialComponent>("Phong Material");
                displayAddComponentEntry<PBRMaterialComponent>("PBR Material");
                displayAddComponentEntry<MaterialSlotsComponent>("Material Slots");
            }

            ImGui::SeparatorText("Other");
            displayAddComponentEntry<CameraComponent>("Camera");
            displayAddComponentEntry<ScriptContainerComponent>("Scripts");

            // displayAddComponentEntry<ScriptComponent>("Script");
            // bool hasScriptComponent = m_selectedEntity.hasComponent<ScriptComponent>();
            // bool hasScriptContainerComponent = m_selectedEntity.hasComponent<ScriptContainerComponent>();
            // if (!hasScriptComponent && !hasScriptContainerComponent)
            // {
            // 	displayAddComponentEntry<ScriptComponent>("Script");
            // 	displayAddComponentEntry<ScriptContainerComponent>("Script Container");
            // }

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
            drawVec3Control("Rotation", rotationDeg,0.0f,100.f,1.0f);
            component.setRotationEuler(glm::radians(rotationDeg));

            drawVec3Control("Scale", component.scale, 1.0f); });

        drawComponent<CameraComponent>("Camera", entity, [](auto &component)
                                       {
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
            } });
        drawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [this](auto &component)
                                               {
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

            ImGui::DragFloat("Tiling Factor", &component.tilingFactor, 0.1f, 0.0f, 100.0f); });
        drawComponent<MeshComponent>("Mesh", entity, [](auto &component)
                                     {
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
                    component.meshHandle = MeshFactory::CreateBox(glm::vec3(1));
                    component.memoryMeshType = MemoryMeshType::Cube;
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Selectable("Sphere")) {
                    component.meshHandle = MeshFactory::CreateSphere(0.5f);
                    component.memoryMeshType = MemoryMeshType::Sphere;
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Selectable("Cylinder")) {
                    component.meshHandle = MeshFactory::CreateCylinder(0.5f, 1.0f, 32);
                    component.memoryMeshType = MemoryMeshType::Cylinder;
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Selectable("Capsule")) {
                    component.meshHandle = MeshFactory::CreateCapsule(0.5f, 1.5f, 32, 8);
                    component.memoryMeshType = MemoryMeshType::Capsule;
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Selectable("Cone")) {
                    component.meshHandle = MeshFactory::CreateCone(0.5f, 1.0f, 32);
                    component.memoryMeshType = MemoryMeshType::Cone;
                    ImGui::CloseCurrentPopup();
                }
                component.memoryOnly = true;
                ImGui::EndPopup();
            } });
        drawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto &component)
                                                 {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ImGui::DragFloat("Intensity", &component.intensity, 0.1f, 0.0f);
            ImGui::Checkbox("Main Light", &component.mainLight); });
        drawComponent<PointLightComponent>("Point Light", entity, [](auto &component)
                                           {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ImGui::DragFloat("Intensity", &component.intensity, 0.1f, 0.0f);
            ImGui::DragFloat("Range", &component.range, 0.1f, 0.0f); });
        drawComponent<SpotLightComponent>("Spot Light", entity, [](auto &c)
                                          {
            ImGui::ColorEdit4("Color", glm::value_ptr(c.color));
            ImGui::DragFloat("Intensity", &c.intensity, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Range", &c.range, 0.1f, 0.0f, 1000.0f);

            ImGui::Separator();
            ImGui::Text("Cone");

            ImGui::DragFloat("Angle (deg)", &c.angle, 0.5f, 1.0f, 89.0f);
            ImGui::DragFloat("Softness", &c.softness, 0.01f, 0.0f, 1.0f, "%.3f"); });

        drawComponent<TextComponent>("Text", entity, [](auto &component)
                                     {
            char buffer[1024];
            strncpy(buffer, component.textString.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';

            if (ImGui::InputTextMultiline("Text String", buffer, sizeof(buffer))) {
                component.textString = buffer;
            }
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ImGui::DragFloat("Kerning", &component.kerning, 0.025f);
            ImGui::DragFloat("Line Spacing", &component.lineSpacing, 0.025f); });

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
        drawComponent<ScriptContainerComponent>("Scripts Container", entity, [](auto &component)
                                                {
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
            } });

        drawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto &component)
                                               {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ImGui::DragFloat("Thickness", &component.thickness, 0.025f, 0.0f, 1.0f);
            ImGui::DragFloat("Fade", &component.fade, 0.00025f, 0.0f, 1.0f); });

        drawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto &component)
                                            {
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
        drawComponent<BoxSensor2DComponent>("Box Sensor 2D", entity, [](auto &component)
                                            {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.offset), 0.01f);
            ImGui::DragFloat2("Size", glm::value_ptr(component.size), 0.01f); });
        drawComponent<Rigidbody3DComponent>("Rigidbody 3D", entity, [](auto &component)
                                            {
            const char *bodyTypeStrings[] = {"Static", "Dynamic", "Kinematic"};
            const char *currentBodyTypeString = bodyTypeStrings[(int) component.type];
            if (ImGui::BeginCombo("Body Type##3D", currentBodyTypeString)) {
                for (int i = 0; i < 3; i++) {
                    bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                    if (ImGui::Selectable(bodyTypeStrings[i], isSelected)) {
                        currentBodyTypeString = bodyTypeStrings[i];
                        component.type = (Rigidbody3DComponent::BodyType) i;
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::DragFloat("Mass", &component.mass, 0.01f, 0.0f, 1000.0f);
            ImGui::DragFloat("Linear Damping", &component.linearDamping, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Angular Damping", &component.angularDamping, 0.01f, 0.0f, 10.0f);
            ImGui::Checkbox("Use Gravity", &component.useGravity); });
        drawComponent<BoxCollider3DComponent>("Box Collider 3D", entity, [](auto &component)
                                              {
            ImGui::DragFloat3("Offset##3d", glm::value_ptr(component.offset), 0.01f);
            ImGui::DragFloat3("Size##3d", glm::value_ptr(component.size), 0.01f);
            ImGui::DragFloat("Density##3d", &component.density, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Friction##3d", &component.friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution##3d", &component.restitution, 0.01f, 0.0f, 1.0f);
            ImGui::Checkbox("Trigger##3d", &component.isTrigger); });

        drawComponent<PhongMaterialComponent>("Phong Material", entity, [entity](auto &component) mutable
                                              {
           
            
            ImGui::Text("Phong Lighting Model");
            ImGui::Separator();
            
            ImGui::ColorEdit4("Diffuse Color", glm::value_ptr(component.diffuseColor));
            ImGui::ColorEdit4("Ambient Color", glm::value_ptr(component.ambientColor));
            
            ImGui::Separator();
            ImGui::Text("Texture Settings");
            ImGui::Checkbox("Use Texture", &component.useTexture);
            
            if (component.useTexture) {
                ImGui::Checkbox("Flip UV", &component.flipUV);
                
                ImGui::Text("Diffuse Texture");
                ImGui::SameLine();
                ImGui::Button("Drag texture here##phong");
                
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE")) {
                        const char *path = static_cast<const char *>(payload->Data);
                        if (path && path[0]) {
                            auto editorAssets = Project::getEditorAssetManager();
                            AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                            if (static_cast<uint64_t>(handle) != 0) {
                                component.diffuseTextureHandle = handle;
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                
                if (static_cast<uint64_t>(component.diffuseTextureHandle) != 0) {
                    ImGui::Text("Handle: %llu", static_cast<uint64_t>(component.diffuseTextureHandle));
                }
            } });

        drawComponent<PBRMaterialComponent>("PBR Material", entity, [entity](auto &component) mutable
                                            {
            ImGui::Text("Physically Based Rendering");
            ImGui::Separator();
            
            ImGui::ColorEdit3("Albedo", glm::value_ptr(component.albedo));
            ImGui::SliderFloat("Metallic", &component.metallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness", &component.roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Ambient Occlusion", &component.ao, 0.0f, 1.0f);
            
            ImGui::Separator();
            ImGui::Checkbox("Flip UV", &component.flipUV);
            
            ImGui::Separator();
            ImGui::Text("Texture Maps");
            
            // Albedo Map
            ImGui::Text("Albedo Map");
            ImGui::SameLine();
            ImGui::Button("Drag texture##albedo");
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE")) {
                    const char *path = static_cast<const char *>(payload->Data);
                    if (path && path[0]) {
                        auto editorAssets = Project::getEditorAssetManager();
                        AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                        if (static_cast<uint64_t>(handle) != 0) {
                            component.albedoMapHandle = handle;
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
            if (static_cast<uint64_t>(component.albedoMapHandle) != 0) {
                ImGui::SameLine();
                ImGui::Text("(%llu)", static_cast<uint64_t>(component.albedoMapHandle));
                
                auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(component.albedoMapHandle);
                if (texture && texture->isLoaded()) {
                    ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                    ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
                }
            }
            
            // Normal Map
            ImGui::Text("Normal Map");
            ImGui::SameLine();
            ImGui::Button("Drag texture##normal");
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE")) {
                    const char *path = static_cast<const char *>(payload->Data);
                    if (path && path[0]) {
                        auto editorAssets = Project::getEditorAssetManager();
                        AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                        if (static_cast<uint64_t>(handle) != 0) {
                            component.normalMapHandle = handle;
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
            if (static_cast<uint64_t>(component.normalMapHandle) != 0) {
                ImGui::SameLine();
                ImGui::Text("(%llu)", static_cast<uint64_t>(component.normalMapHandle));
                
                auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(component.normalMapHandle);
                if (texture && texture->isLoaded()) {
                    ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                    ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
                }
            }
            
            // Metallic Map
            ImGui::Text("Metallic Map");
            ImGui::SameLine();
            ImGui::Button("Drag texture##metallic");
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE")) {
                    const char *path = static_cast<const char *>(payload->Data);
                    if (path && path[0]) {
                        auto editorAssets = Project::getEditorAssetManager();
                        AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                        if (static_cast<uint64_t>(handle) != 0) {
                            component.metallicMapHandle = handle;
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
            if (static_cast<uint64_t>(component.metallicMapHandle) != 0) {
                ImGui::SameLine();
                ImGui::Text("(%llu)", static_cast<uint64_t>(component.metallicMapHandle));
                
                auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(component.metallicMapHandle);
                if (texture && texture->isLoaded()) {
                    ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                    ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
                }
            }
            
            // Roughness Map
            ImGui::Text("Roughness Map");
            ImGui::SameLine();
            ImGui::Button("Drag texture##roughness");
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE")) {
                    const char *path = static_cast<const char *>(payload->Data);
                    if (path && path[0]) {
                        auto editorAssets = Project::getEditorAssetManager();
                        AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                        if (static_cast<uint64_t>(handle) != 0) {
                            component.roughnessMapHandle = handle;
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
            if (static_cast<uint64_t>(component.roughnessMapHandle) != 0) {
                ImGui::SameLine();
                ImGui::Text("(%llu)", static_cast<uint64_t>(component.roughnessMapHandle));
                
                auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(component.roughnessMapHandle);
                if (texture && texture->isLoaded()) {
                    ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                    ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
                }
            }
            
            // AO Map
            ImGui::Text("AO Map");
            ImGui::SameLine();
            ImGui::Button("Drag texture##ao");
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE")) {
                    const char *path = static_cast<const char *>(payload->Data);
                    if (path && path[0]) {
                        auto editorAssets = Project::getEditorAssetManager();
                        AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                        if (static_cast<uint64_t>(handle) != 0) {
                            component.aoMapHandle = handle;
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
                if (static_cast<uint64_t>(component.aoMapHandle) != 0) {
                    ImGui::SameLine();
                    ImGui::Text("(%llu)", static_cast<uint64_t>(component.aoMapHandle));
                    
                    auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(component.aoMapHandle);
                    if (texture && texture->isLoaded()) {
                        ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                        ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
                    }
                }
            
            ImGui::Separator();
            ImGui::TextWrapped("Tip: Drag texture files from Content Browser to assign maps"); });

        drawComponent<MaterialSlotsComponent>("Material Slots", entity, [entity](auto &component) mutable
                                              {
            ImGui::Text("Multi-Material");
            ImGui::Separator();

            // 获取 Mesh 和 SubMesh 数量
            uint32_t subMeshCount = 0;
            std::shared_ptr<Mesh> mesh = nullptr;

            if (entity.hasComponent<MeshComponent>()) {
                auto &meshComponent = entity.getComponent<MeshComponent>();
                if (static_cast<uint64_t>(meshComponent.meshHandle) != 0) {
                    mesh = Project::getEditorAssetManager()->getAsset<Mesh>(meshComponent.meshHandle);
                    if (mesh)
                        subMeshCount = static_cast<uint32_t>(mesh->getSubMeshes().size());
                }
            }

            if (subMeshCount == 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Warning: No mesh attached or mesh has no submeshes");
                return;
            }

            ImGui::Text("SubMesh Count: %u", subMeshCount);
            ImGui::Separator();

            // 确保材质槽位数组大小匹配SubMesh数量
            if (component.materials.size() < subMeshCount)
                component.materials.resize(subMeshCount, nullptr);

            auto editorAssets = Project::getEditorAssetManager();

            // Drag & Drop + 缩略图函数
            auto handleTextureDragDropWithPreview = [&](const char* label, AssetHandle& textureHandle, auto setTextureCallback) {
                ImGui::Text("%s:", label);
                ImGui::SameLine();
                ImGui::Button(("Drag##" + std::string(label)).c_str());

                // Drag & Drop 逻辑
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE")) {
                        const char* path = static_cast<const char*>(payload->Data);
                        if (path && path[0]) {
                            AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                            if (static_cast<uint64_t>(handle) != 0) {
                                setTextureCallback(handle);
                                textureHandle = handle;
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                // 显示 handle
                ImGui::SameLine();
                ImGui::Text("(%llu)", static_cast<uint64_t>(textureHandle));

                // 显示缩略图
                if (static_cast<uint64_t>(textureHandle) != 0) {
                    auto texture = editorAssets->getAsset<Texture2D>(textureHandle);
                    if (texture && texture->isLoaded()) {
                        ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                        ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
                    }
                }
            };

            // 绘制材质 UI
            auto drawMaterialUI = [&](std::shared_ptr<Material>& material) {
                if (!material) {
                    if (ImGui::Button("Create New Material")) {
                        material = std::make_shared<Material>();
                        material->setMaterialType(MaterialType::Phong);
                        material->setDiffuseColor(glm::vec4(0.8f));
                        material->setAmbientColor(glm::vec4(0.1f));
                    }
                    return;
                }

                // 材质类型选择
                const char* materialTypeStrings[] = { "Phong", "PBR" };
                int currentType = (material->getType() == MaterialType::PBR) ? 1 : 0;
                if (ImGui::Combo("Material Type", &currentType, materialTypeStrings, 2)) {
                    MaterialType newType = (currentType == 1) ? MaterialType::PBR : MaterialType::Phong;
                    material->setMaterialType(newType);

                    // 默认参数
                    if (newType == MaterialType::Phong) {
                        material->setDiffuseColor(glm::vec4(0.8f));
                        material->setAmbientColor(glm::vec4(0.1f));
                    } else {
                        material->setAlbedo(glm::vec3(0.8f));
                        material->setMetallic(0.0f);
                        material->setRoughness(0.5f);
                        material->setAO(1.0f);
                    }
                }

                ImGui::Separator();

                if (material->getType() == MaterialType::Phong) {
                    ImGui::Text("Phong Material Properties");

                    glm::vec4 diffuse = material->getDiffuseColor();
                    if (ImGui::ColorEdit4("Diffuse Color", glm::value_ptr(diffuse)))
                        material->setDiffuseColor(diffuse);

                    glm::vec4 ambient = material->getAmbientColor();
                    if (ImGui::ColorEdit4("Ambient Color", glm::value_ptr(ambient)))
                        material->setAmbientColor(ambient);

                    // Diffuse Texture
                    handleTextureDragDropWithPreview("Diffuse Texture", material->getDiffuseTexture(),
                        [&](AssetHandle h) { material->setDiffuseTexture(h); });

                } else { // PBR
                    ImGui::Text("PBR Material Properties");

                    glm::vec3 albedo = material->getAlbedo();
                    if (ImGui::ColorEdit3("Albedo", glm::value_ptr(albedo)))
                        material->setAlbedo(albedo);

                    float metallic = material->getMetallic();
                    if (ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f))
                        material->setMetallic(metallic);

                    float roughness = material->getRoughness();
                    if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f))
                        material->setRoughness(roughness);

                    float ao = material->getAO();
                    if (ImGui::SliderFloat("AO", &ao, 0.0f, 1.0f))
                        material->setAO(ao);

                    ImGui::Separator();
                    ImGui::Text("Texture Maps:");

                    const char* mapNames[] = { "Albedo", "Normal", "Metallic", "Roughness", "AO" };
                    for (int m = 0; m < 5; m++) {
                        AssetHandle* handlePtr = nullptr;
                        switch(m) {
                            case 0: handlePtr = &material->getAlbedoMap(); break;
                            case 1: handlePtr = &material->getNormalMap(); break;
                            case 2: handlePtr = &material->getMetallicMap(); break;
                            case 3: handlePtr = &material->getRoughnessMap(); break;
                            case 4: handlePtr = &material->getAOMap(); break;
                        }
                        if (handlePtr) {
                            handleTextureDragDropWithPreview(mapNames[m], *handlePtr, [&](AssetHandle h){
                                switch(m) {
                                    case 0: material->setAlbedoMap(h); break;
                                    case 1: material->setNormalMap(h); break;
                                    case 2: material->setMetallicMap(h); break;
                                    case 3: material->setRoughnessMap(h); break;
                                    case 4: material->setAOMap(h); break;
                                }
                            });
                        }
                    }
                }

                ImGui::Separator();
                if (ImGui::Button("Remove Material"))
                    material = nullptr;
            };

            // 绘制每个 SubMesh 材质
            for (uint32_t i = 0; i < subMeshCount; i++) {
                ImGui::PushID(i);
                if (ImGui::TreeNodeEx(("SubMesh " + std::to_string(i)).c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    drawMaterialUI(component.materials[i]);
                    ImGui::TreePop();
                }
                ImGui::Spacing();
                ImGui::PopID();
            }

            ImGui::Separator();
            ImGui::TextWrapped("Tip: Create and configure materials for each SubMesh. Drag textures from Content Browser."); });
    }

    template <typename T>
    inline void InspectorPanel::displayAddComponentEntry(const std::string &entryName)
    {
        if (!m_selectedEntity.hasComponent<T>())
        {
            if (ImGui::MenuItem(entryName.c_str()))
            {
                m_selectedEntity.addComponent<T>();
                ImGui::CloseCurrentPopup();
            }
        }
    }
} // namespace Fermion
