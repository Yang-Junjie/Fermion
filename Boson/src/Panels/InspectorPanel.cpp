#include "InspectorPanel.hpp"
#include "Renderer/Model/Mesh.hpp"
#include "Renderer/Model/ModelAsset.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Components.hpp"
#include "Script/ScriptManager.hpp"
#include "Project/Project.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <entt/entt.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <optional>
#include <string_view>

#include "Renderer/Model/MeshFactory.hpp"
#include "Renderer/Model/MaterialFactory.hpp"

namespace Fermion
{
    template <typename AssetManagerT>
    static void applyModelAssetToMeshComponent(MeshComponent &component, AssetManagerT &editorAssets,
                                               const std::filesystem::path &path)
    {
        AssetHandle modelHandle = editorAssets->importAsset(path);
        FERMION_ASSERT(modelHandle.isValid(), "Failed to import model asset");
        auto modelAsset = editorAssets->template getAsset<ModelAsset>(modelHandle);
        FERMION_ASSERT(modelAsset != nullptr, "Failed to get model asset");

        component.meshHandle = modelAsset->mesh;
        component.memoryOnly = false;

        auto mesh = editorAssets->template getAsset<Mesh>(modelAsset->mesh);
        FERMION_ASSERT(mesh != nullptr, "Failed to get mesh asset");

        const auto &subMeshes = mesh->getSubMeshes();
        component.resizeSubmeshMaterials(static_cast<uint32_t>(subMeshes.size()));

        for (size_t submeshIdx = 0; submeshIdx < subMeshes.size(); ++submeshIdx)
        {
            uint32_t materialSlot = subMeshes[submeshIdx].MaterialSlotIndex;
            if (materialSlot < modelAsset->materials.size())
            {
                component.setSubmeshMaterial(static_cast<uint32_t>(submeshIdx), modelAsset->materials[materialSlot]);
            }
        }
    }

    static std::optional<std::string_view> payloadToStringView(const ImGuiPayload *payload)
    {
        if (!payload || !payload->Data || payload->DataSize <= 0)
            return std::nullopt;

        const char *data = static_cast<const char *>(payload->Data);
        std::string_view view(data, static_cast<size_t>(payload->DataSize));
        size_t nullPos = view.find('\0');
        if (nullPos != std::string_view::npos)
            view = view.substr(0, nullPos);

        if (view.empty())
            return std::nullopt;
        return view;
    }

    template <typename AssetManagerT>
    static void drawMeshModelDropTarget(MeshComponent &component, AssetManagerT &editorAssets)
    {
        ImGui::Button("Change or Add Mesh");

        if (!ImGui::BeginDragDropTarget())
            return;

        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_MODEL"))
        {
            if (auto view = payloadToStringView(payload))
                applyModelAssetToMeshComponent(component, editorAssets, std::filesystem::path(*view));
        }
        ImGui::EndDragDropTarget();
    }

    static void drawEngineInternalMeshPopup(MeshComponent &component)
    {
        if (ImGui::Button("Add Engine Internal Mesh"))
            ImGui::OpenPopup("mesh_popup");

        ImVec2 popupPos = ImGui::GetItemRectMin();
        ImGui::SetNextWindowPos(popupPos, ImGuiCond_Appearing, ImVec2(0.0f, 0.0f));

        if (!ImGui::BeginPopup("mesh_popup"))
            return;

        ImGui::Text("Available Meshes:");
        ImGui::Separator();

        const auto selectMesh = [&](const char *label, const auto &createHandle, MemoryMeshType type)
        {
            if (!ImGui::Selectable(label))
                return;
            component.meshHandle = createHandle();
            component.memoryMeshType = type;
            ImGui::CloseCurrentPopup();
        };

        selectMesh("Cube", []
                   { return MeshFactory::CreateBox(glm::vec3(1)); }, MemoryMeshType::Cube);
        selectMesh("Sphere", []
                   { return MeshFactory::CreateSphere(0.5f); }, MemoryMeshType::Sphere);
        selectMesh("Cylinder", []
                   { return MeshFactory::CreateCylinder(0.5f, 1.0f, 32); }, MemoryMeshType::Cylinder);
        selectMesh("Capsule", []
                   { return MeshFactory::CreateCapsule(0.5f, 1.5f, 32, 8); }, MemoryMeshType::Capsule);
        selectMesh("Cone", []
                   { return MeshFactory::CreateCone(0.5f, 1.0f, 32); }, MemoryMeshType::Cone);

        component.memoryOnly = true;
        ImGui::EndPopup();
    }

    template <typename AssetManagerT>
    static void drawSubmeshMaterialsEditor(MeshComponent &component, AssetManagerT &editorAssets)
    {
        ImGui::Separator();
        ImGui::Text("Multi-Material Configuration");

        uint32_t subMeshCount = 0;
        std::shared_ptr<Mesh> mesh = nullptr;

        if (static_cast<uint64_t>(component.meshHandle) != 0)
        {
            mesh = editorAssets->template getAsset<Mesh>(component.meshHandle);
            if (mesh)
            {
                subMeshCount = static_cast<uint32_t>(mesh->getSubMeshes().size());
                component.resizeSubmeshMaterials(subMeshCount);
            }
        }

        if (subMeshCount == 0)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No mesh attached or mesh has no submeshes");
            return;
        }

        ImGui::Text("SubMesh Count: %u", subMeshCount);

        for (uint32_t i = 0; i < subMeshCount; i++)
        {
            ImGui::PushID(i);

            if (ImGui::TreeNodeEx(("SubMesh " + std::to_string(i)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                AssetHandle currentMaterial = component.getSubmeshMaterial(i);
                ImGui::Text("Material Handle: %llu", static_cast<uint64_t>(currentMaterial));

                ImGui::Button("Drag Material Here");
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_MATERIAL"))
                    {
                        if (auto view = payloadToStringView(payload))
                        {
                            AssetHandle handle = editorAssets->importAsset(std::filesystem::path(*view));
                            if (static_cast<uint64_t>(handle) != 0)
                                component.setSubmeshMaterial(i, handle);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine();
                if (ImGui::Button("Clear"))
                    component.clearSubmeshMaterial(i);

                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        if (ImGui::Button("Clear All Materials"))
            component.clearAllSubmeshMaterials();
    }

    InspectorPanel::InspectorPanel()
    {
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

    static bool drawVec3Control(const std::string &label, glm::vec3 &values, float resetValue = 0.0f, float columnWidth = 100.0f, float dragSpeed = 0.1f)
    {
        bool changed = false;
        ImGui::PushID(label.c_str());

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{0, 1});

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 1});

        if (ImGui::BeginTable("##Vec3Table", 2, ImGuiTableFlags_NoSavedSettings))
        {
            ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
            ImGui::TableSetupColumn("##controls", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFontSize() + 2.0f);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text(label.c_str());

            ImGui::TableNextColumn();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

            float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
            ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};
            float itemWidth = (ImGui::GetContentRegionAvail().x - buttonSize.x * 3.0f) / 3.0f;

            auto drawAxisControl = [&](const char *axisLabel, float &value, const ImVec4 &color, const ImVec4 &colorHovered)
            {
                bool axisChanged = false;

                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

                if (ImGui::Button(axisLabel, buttonSize))
                {
                    value = resetValue;
                    axisChanged = true;
                }
                ImGui::PopStyleColor(3);

                ImGui::SameLine();
                ImGui::SetNextItemWidth(itemWidth);

                if (ImGui::DragFloat((std::string("##") + axisLabel).c_str(), &value, dragSpeed, 0.0f, 0.0f, "%.2f"))
                {
                    axisChanged = true;
                }
                ImGui::SameLine();

                return axisChanged;
            };

            changed |= drawAxisControl("X", values.x, {0.8f, 0.1f, 0.15f, 1.0f}, {0.9f, 0.2f, 0.2f, 1.0f});
            changed |= drawAxisControl("Y", values.y, {0.2f, 0.7f, 0.2f, 1.0f}, {0.3f, 0.8f, 0.3f, 1.0f});
            changed |= drawAxisControl("Z", values.z, {0.1f, 0.25f, 0.8f, 1.0f}, {0.2f, 0.35f, 0.9f, 1.0f});

            ImGui::PopStyleVar();
            ImGui::EndTable();
        }

        ImGui::PopStyleVar(2);
        ImGui::PopID();
        return changed;
    }
    static bool drawVec2Control(const std::string &label,
                                glm::vec2 &values,
                                float resetValue = 0.0f,
                                float columnWidth = 100.0f,
                                float dragSpeed = 0.1f)
    {
        bool changed = false;
        ImGui::PushID(label.c_str());

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{0, 1});
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 1});

        if (ImGui::BeginTable("##Vec2Table", 2, ImGuiTableFlags_NoSavedSettings))
        {
            ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
            ImGui::TableSetupColumn("##controls", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFontSize() + 2.0f);

            // Label
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", label.c_str());

            // Controls
            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

            float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
            ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};
            float itemWidth =
                (ImGui::GetContentRegionAvail().x - buttonSize.x * 2.0f) / 2.0f;

            auto drawAxisControl = [&](const char *axisLabel,
                                       float &value,
                                       const ImVec4 &color,
                                       const ImVec4 &colorHovered)
            {
                bool axisChanged = false;

                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

                if (ImGui::Button(axisLabel, buttonSize))
                {
                    value = resetValue;
                    axisChanged = true;
                }
                ImGui::PopStyleColor(3);

                ImGui::SameLine();
                ImGui::SetNextItemWidth(itemWidth);

                if (ImGui::DragFloat((std::string("##") + axisLabel).c_str(),
                                     &value,
                                     dragSpeed,
                                     0.0f,
                                     0.0f,
                                     "%.2f"))
                {
                    axisChanged = true;
                }

                ImGui::SameLine();
                return axisChanged;
            };

            changed |= drawAxisControl("X", values.x,
                                       {0.8f, 0.1f, 0.15f, 1.0f},
                                       {0.9f, 0.2f, 0.2f, 1.0f});

            changed |= drawAxisControl("Y", values.y,
                                       {0.2f, 0.7f, 0.2f, 1.0f},
                                       {0.3f, 0.8f, 0.3f, 1.0f});

            ImGui::PopStyleVar();
            ImGui::EndTable();
        }

        ImGui::PopStyleVar(2);
        ImGui::PopID();
        return changed;
    }

    template <typename T, typename UIFunction>
    static void drawComponent(const std::string &name, Entity entity, UIFunction uiFunction)
    {
        if (!entity.hasComponent<T>())
            return;

        auto &component = entity.getComponent<T>();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 2});

        const ImGuiTreeNodeFlags treeNodeFlags =
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_Framed |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_FramePadding |
            ImGuiTreeNodeFlags_AllowOverlap;

        bool open = ImGui::TreeNodeEx((void *)typeid(T).hash_code(), treeNodeFlags, name.c_str());

        ImGui::PopStyleVar();

        bool removeComponent = false;
        if (ImGui::BeginPopupContextItem("ComponentSettings"))
        {
            if (ImGui::MenuItem("Remove Component"))
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
            std::strncpy(buffer, tag.c_str(), sizeof(buffer));
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
            displayAddComponentEntry<CircleCollider3DComponent>("Circle Collider3D");
            displayAddComponentEntry<CapsuleCollider3DComponent>("Capsule Collider3D");
            displayAddComponentEntry<DirectionalLightComponent>("Directional Light");
            displayAddComponentEntry<PointLightComponent>("Point Light");
            displayAddComponentEntry<SpotLightComponent>("Spot Light");

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
        drawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto &component)
                                               {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));

            auto editorAssets = Project::getEditorAssetManager();

            if (static_cast<uint64_t>(component.textureHandle) != 0 && !component.texture) {
                component.texture = editorAssets->getAsset<Texture2D>(component.textureHandle);
            }

            ImTextureID textureID = (ImTextureID)0;
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
            } else {
                auto defaultTexture = editorAssets->template getDefaultAssetForType<Texture2D>();
                if (defaultTexture && defaultTexture->isLoaded()) {
                    textureID = (ImTextureID) (uintptr_t) defaultTexture->getRendererID();
                }
            }


            ImGui::SameLine();
            if (textureID)
                ImGui::Image(textureID, imageSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            else
                ImGui::Text("[No Texture]");

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
                                         ImGui::Text("Mesh Handle: %llu", static_cast<uint64_t>(component.meshHandle));

                                         auto editorAssets = Project::getEditorAssetManager();
                                         drawMeshModelDropTarget(component, editorAssets);
                                         drawEngineInternalMeshPopup(component);
                                         drawSubmeshMaterialsEditor(component, editorAssets); });
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
            drawVec2Control("Offset",component.offset,0.0f,100.0f,0.1f); 
            drawVec2Control("Size",component.size,0.0f,100.0f,0.1f);
            ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f); });
        drawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto &component)
                                                 {
            drawVec2Control("Offset",component.offset,0.0f,100.0f,0.1f); 
            ImGui::DragFloat("Radius", &component.radius);
            ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f); });
        drawComponent<BoxSensor2DComponent>("Box Sensor 2D", entity, [](auto &component)
                                            {
            drawVec2Control("Offset",component.offset,0.0f,100.0f,0.1f); 
            drawVec2Control("Size",component.size,0.0f,100.0f,0.1f); });
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
            ImGui::Checkbox("Use Gravity", &component.useGravity);
            ImGui::Checkbox("Fixed Rotation##3d", &component.fixedRotation); });
        drawComponent<BoxCollider3DComponent>("Box Collider 3D", entity, [](auto &component)
                                              {
            drawVec3Control("Offset",component.offset,0.0f,100.0f,0.1f);
            drawVec3Control("Size",component.size,0.0f,100.0f,0.1f);
            ImGui::DragFloat("Density##3d", &component.density, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Friction##3d", &component.friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution##3d", &component.restitution, 0.01f, 0.0f, 1.0f);
            ImGui::Checkbox("Trigger##3d", &component.isTrigger); });
        drawComponent<CircleCollider3DComponent>("Circle Collider 3D", entity, [](auto &component)
                                                 {
            drawVec3Control("Offset",component.offset,0.0f,100.0f,0.1f);
            ImGui::DragFloat("Radius##circle3d", &component.radius, 0.01f, 0.01f, 100.0f);
            ImGui::DragFloat("Density##circle3d", &component.density, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Friction##circle3d", &component.friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution##circle3d", &component.restitution, 0.01f, 0.0f, 1.0f);
            ImGui::Checkbox("Trigger##circle3d", &component.isTrigger); });
        drawComponent<CapsuleCollider3DComponent>("Capsule Collider 3D", entity, [](auto &component)
                                                  {
            drawVec3Control("Offset",component.offset,0.0f,100.0f,0.1f);
            ImGui::DragFloat("Radius##capsule3d", &component.radius, 0.01f, 0.01f, 100.0f);
            ImGui::DragFloat("Height##capsule3d", &component.height, 0.01f, 0.01f, 100.0f);
            ImGui::DragFloat("Density##capsule3d", &component.density, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Friction##capsule3d", &component.friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution##capsule3d", &component.restitution, 0.01f, 0.0f, 1.0f);
            ImGui::Checkbox("Trigger##capsule3d", &component.isTrigger); });
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
