#include "InspectorPanel.hpp"
#include "Renderer/Model/Mesh.hpp"
#include "Renderer/Model/Material.hpp"
#include "Renderer/Model/ModelAsset.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Components.hpp"
#include "Script/ScriptManager.hpp"
#include "Script/ScriptTypes.hpp"
#include "Project/Project.hpp"
#include "Asset/AssetManager/EditorAssetManager.hpp"
#include "ImGui/BosonUI.hpp"

#include "Renderer/Model/MeshFactory.hpp"
#include "Renderer/Preview/MaterialPreviewRenderer.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <optional>
#include <string_view>

namespace Fermion
{
    namespace
    {
        // 从完整类名获取显示名称
        auto getScriptDisplayName = [](const std::string &fullName) -> std::string
        {
            size_t dotPos = fullName.rfind('.');
            return (dotPos != std::string::npos) ? fullName.substr(dotPos + 1) : fullName;
        };

        // 判断是否Photon 命名空间
        auto isUserScript = [](const std::string &fullName) -> bool
        {
            size_t dotPos = fullName.find('.');
            if (dotPos == std::string::npos)
                return false;
            return fullName.substr(0, dotPos) == "Photon";
        };

        // 获取字段类型名称
        auto getFieldTypeName = [](ScriptFieldType type) -> const char *
        {
            switch (type)
            {
            case ScriptFieldType::Float:
                return "Float";
            case ScriptFieldType::Double:
                return "Double";
            case ScriptFieldType::Int:
                return "Int";
            case ScriptFieldType::Bool:
                return "Bool";
            case ScriptFieldType::ULong:
                return "ULong";
            case ScriptFieldType::Vector2:
                return "Vector2";
            case ScriptFieldType::Vector3:
                return "Vector3";
            case ScriptFieldType::Vector4:
                return "Vector4";
            case ScriptFieldType::Entity:
                return "Entity";
            default:
                return "Unknown";
            }
        };

        // 绘制单个脚本字段
        auto drawScriptField = [&](const std::string &fieldName,
                                   const ScriptField &field,
                                   std::shared_ptr<ScriptInstance> instance)
        {
            ImGui::PushID(fieldName.c_str());

            bool hasInstance = (instance != nullptr);

            switch (field.type)
            {
            case ScriptFieldType::Float:
            {
                float value = hasInstance ? instance->getFieldValue<float>(fieldName) : 0.0f;
                if (ui::drawFloatControl(fieldName.c_str(), value, 120.0f, 0.1f) && hasInstance)
                    instance->setFieldValue<float>(fieldName, value);
                break;
            }
            case ScriptFieldType::Double:
            {
                float value = hasInstance ? static_cast<float>(instance->getFieldValue<double>(fieldName)) : 0.0f;
                if (ui::drawFloatControl(fieldName.c_str(), value, 120.0f, 0.1f) && hasInstance)
                    instance->setFieldValue<double>(fieldName, static_cast<double>(value));
                break;
            }
            case ScriptFieldType::Int:
            {
                int value = hasInstance ? instance->getFieldValue<int>(fieldName) : 0;
                if (ui::drawIntControl(fieldName.c_str(), value, 120.0f) && hasInstance)
                    instance->setFieldValue<int>(fieldName, value);
                break;
            }
            case ScriptFieldType::Bool:
            {
                bool value = hasInstance ? instance->getFieldValue<bool>(fieldName) : false;
                if (ui::drawCheckboxControl(fieldName.c_str(), value, 120.0f) && hasInstance)
                    instance->setFieldValue<bool>(fieldName, value);
                break;
            }
            case ScriptFieldType::ULong:
            {
                uint64_t value = hasInstance ? instance->getFieldValue<uint64_t>(fieldName) : 0;
                ImGui::Text("%s: %llu", fieldName.c_str(), value);
                break;
            }
            default:
                ImGui::BulletText("%s", fieldName.c_str());
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5f, 0.7f, 1.0f, 1.0f), "(%s)", getFieldTypeName(field.type));
                break;
            }

            ImGui::PopID();
        };

    }
    static std::unique_ptr<MaterialPreviewRenderer> s_materialPreviewRenderer;
    static std::unordered_map<uint64_t, std::unique_ptr<Texture2D>> s_materialPreviewCache;

    static MaterialPreviewRenderer &getMaterialPreviewRenderer()
    {
        if (!s_materialPreviewRenderer)
            s_materialPreviewRenderer = std::make_unique<MaterialPreviewRenderer>();
        return *s_materialPreviewRenderer;
    }
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
        ImGui::Button("Drag Model Here", ImVec2(-1, 20));

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
        if (ImGui::Button("Add Internal Mesh", ImVec2(-1, 20)))
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
        static int selectedSubmeshIndex = -1;

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
            selectedSubmeshIndex = -1;
            return;
        }

        if (selectedSubmeshIndex >= static_cast<int>(subMeshCount))
            selectedSubmeshIndex = -1;

        ImGui::Separator();
        ImGui::Text("Materials");

        ImGui::BeginChild("SubmeshList", ImVec2(0, 120), true, ImGuiWindowFlags_HorizontalScrollbar);
        {
            for (uint32_t i = 0; i < subMeshCount; i++)
            {
                ImGui::PushID(i);

                bool isSelected = (selectedSubmeshIndex == static_cast<int>(i));
                if (ImGui::Selectable(("SubMesh " + std::to_string(i)).c_str(), isSelected))
                {
                    selectedSubmeshIndex = static_cast<int>(i);
                }

                ImGui::PopID();
            }
        }
        ImGui::EndChild();

        if (selectedSubmeshIndex >= 0 && selectedSubmeshIndex < static_cast<int>(subMeshCount))
        {
            ImGui::Separator();
            ImGui::Text("SubMesh %d", selectedSubmeshIndex);
            ImGui::Spacing();

            AssetHandle currentMaterial = component.getSubmeshMaterial(selectedSubmeshIndex);
            ImGui::Text("Material Handle: %llu", static_cast<uint64_t>(currentMaterial));

            // Material preview thumbnail
            {
                std::shared_ptr<Material> material;
                uint64_t key = 0;

                if (static_cast<uint64_t>(currentMaterial) != 0)
                {
                    material = editorAssets->template getAsset<Material>(currentMaterial);
                    key = static_cast<uint64_t>(currentMaterial);
                }

                if (!material)
                {
                    key = 1; // Use a constant key for default material
                    static auto defaultMaterial = []()
                    {
                        auto mat = std::make_shared<Material>();
                        mat->setMaterialType(MaterialType::PBR);
                        mat->setAlbedo(glm::vec3(1.0f));
                        mat->setMetallic(0.0f);
                        mat->setRoughness(1.0f);
                        mat->setAO(1.0f);
                        return mat;
                    }();
                    material = defaultMaterial;
                }

                auto it = s_materialPreviewCache.find(key);

                if (it == s_materialPreviewCache.end())
                {
                    auto &renderer = getMaterialPreviewRenderer();
                    if (renderer.isInitialized())
                    {
                        auto preview = renderer.renderPreview(material);
                        if (preview && preview->isLoaded())
                            s_materialPreviewCache[key] = std::move(preview);
                    }
                    it = s_materialPreviewCache.find(key);
                }

                if (it != s_materialPreviewCache.end() && it->second && it->second->isLoaded())
                {
                    float previewSize = 128.0f;
                    ImGui::Image(
                        (ImTextureID)(uintptr_t)it->second->getRendererID(),
                        ImVec2(previewSize, previewSize),
                        ImVec2(0, 1), ImVec2(1, 0));
                }
            }

            ImGui::Button("Drag Material Here", ImVec2(-1, 40));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_MATERIAL"))
                {
                    if (auto view = payloadToStringView(payload))
                    {
                        AssetHandle handle = editorAssets->importAsset(std::filesystem::path(*view));
                        if (static_cast<uint64_t>(handle) != 0)
                            component.setSubmeshMaterial(selectedSubmeshIndex, handle);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::Button("Clear Material", ImVec2(-1, 0)))
                component.clearSubmeshMaterial(selectedSubmeshIndex);
        }

        ImGui::Separator();
        if (ImGui::Button("Clear All Materials", ImVec2(-1, 0)))
        {
            component.clearAllSubmeshMaterials();
            selectedSubmeshIndex = -1;
        }
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
        if (ui::BeginPopup("Add Component"))
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
            displayAddComponentEntry<AnimatorComponent>("Animator");
            displayAddComponentEntry<Rigidbody3DComponent>("Rigidbody3D");
            displayAddComponentEntry<BoxCollider3DComponent>("Box Collider3D");
            displayAddComponentEntry<CircleCollider3DComponent>("Circle Collider3D");
            displayAddComponentEntry<CapsuleCollider3DComponent>("Capsule Collider3D");
            displayAddComponentEntry<MeshCollider3DComponent>("Mesh Collider3D");
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

            ui::EndPopup();
        }

        drawComponent<TransformComponent>("Transform", entity, [](auto &component)
                                          {
            ui::drawVec3Control("Translation", component.translation);


            glm::vec3 rotationDeg = glm::degrees(component.getRotationEuler());
            ui::drawVec3Control("Rotation", rotationDeg,0.0f,100.f,1.0f);
            component.setRotationEuler(glm::radians(rotationDeg));

            ui::drawVec3Control("Scale", component.scale, 1.0f); });

        drawComponent<CameraComponent>("Camera", entity, [](auto &component)
                                       {
            auto &camera = component.camera;

            ui::drawCheckboxControl("Primary", component.primary, 150.0f);

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
                if(ui::drawFloatControl("FOV",perspectiveFOV,150.0f)){
                    camera.setPerspectiveFOV(glm::radians(perspectiveFOV));
                }

                float perspectiveNear = camera.getPerspectiveNearClip();
                if (ui::drawFloatControl("Near", perspectiveNear, 150.0f)) {
                    camera.setPerspectiveNearClip(perspectiveNear);
                }
                float perspectiveFar = camera.getPerspectiveFarClip();
                if (ui::drawFloatControl("Far", perspectiveFar, 150.0f)) {
                    camera.setPerspectiveFarClip(perspectiveFar);
                }
            }
            if (camera.getProjectionType() == SceneCamera::ProjectionType::Orthographic) {
                float orthoSize = camera.getOrthographicSize();
                if (ui::drawFloatControl("Size", orthoSize, 150.0f)) {
                    camera.setOrthographicSize(orthoSize);
                }

                float orthoNear = camera.getOrthographicNearClip();
                if (ui::drawFloatControl("Near", orthoNear, 150.0f)) {
                    camera.setOrthographicNearClip(orthoNear);
                }
                float orthoFar = camera.getOrthographicFarClip();
                if (ui::drawFloatControl("Far", orthoFar, 150.0f)) {
                    camera.setOrthographicFarClip(orthoFar);
                }

                ui::drawCheckboxControl("Fixed Aspect Ratio", component.fixedAspectRatio, 150.0f);
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

            ui::drawFloatControl("Tiling Factor", component.tilingFactor, 150.0f, 0.1f, 0.0f, 100.0f); });
        drawComponent<MeshComponent>("Mesh", entity, [](auto &component)
                                     {
                                         auto editorAssets = Project::getEditorAssetManager();

                                         ImGui::Text("Mesh Source");
                                         ImGui::Text("Mesh Handle: %llu",static_cast<uint64_t>(component.meshHandle));
                                         ImGui::Spacing();

                                         drawMeshModelDropTarget(component, editorAssets);
                                         drawEngineInternalMeshPopup(component);

                                         ImGui::Spacing();
                                         drawSubmeshMaterialsEditor(component, editorAssets); });

        drawComponent<AnimatorComponent>("Animator", entity, [entity](auto &component) mutable
                                         {
            auto editorAssets = Project::getEditorAssetManager();

            // Skeleton section
            ImGui::Text("Skeleton");
            ImGui::Indent();

            if (static_cast<uint64_t>(component.skeletonHandle) != 0)
            {
                auto skeleton = editorAssets->getAsset<Skeleton>(component.skeletonHandle);
                if (skeleton)
                {
                    ImGui::Text("Handle: %llu", static_cast<uint64_t>(component.skeletonHandle));
                    ImGui::Text("Bones: %zu", skeleton->getBoneCount());
                    component.runtimeSkeleton = skeleton;
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Failed to load skeleton");
                }

                if (ImGui::Button("Clear Skeleton", ImVec2(-1, 0)))
                {
                    component.skeletonHandle = AssetHandle(0);
                    component.runtimeSkeleton = nullptr;
                    component.runtimeAnimator = nullptr;
                }
            }
            else
            {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No skeleton assigned");
            }

            ImGui::Button("Drag Skeleton Here (.fskel)", ImVec2(-1, 20));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_SKELETON"))
                {
                    if (auto view = payloadToStringView(payload))
                    {
                        AssetHandle handle = editorAssets->importAsset(std::filesystem::path(*view));
                        if (static_cast<uint64_t>(handle) != 0)
                        {
                            component.skeletonHandle = handle;
                            component.runtimeSkeleton = editorAssets->getAsset<Skeleton>(handle);
                            component.runtimeAnimator = nullptr;
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::Unindent();

            ImGui::Separator();

            // Animation Clips section
            ImGui::Text("Animation Clips");
            ImGui::Indent();

            // Load runtime clips if needed
            if (component.runtimeClips.size() != component.animationClipHandles.size())
            {
                component.runtimeClips.clear();
                for (const auto &clipHandle : component.animationClipHandles)
                {
                    if (static_cast<uint64_t>(clipHandle) != 0)
                    {
                        auto clip = editorAssets->getAsset<AnimationClip>(clipHandle);
                        component.runtimeClips.push_back(clip);
                    }
                    else
                    {
                        component.runtimeClips.push_back(nullptr);
                    }
                }
            }

            if (component.animationClipHandles.empty())
            {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No animation clips");
            }
            else
            {
                ImGui::Text("Clips: %zu", component.animationClipHandles.size());

                // Animation clip selector
                const char *currentClipName = "None";
                if (component.activeClipIndex < component.runtimeClips.size() && component.runtimeClips[component.activeClipIndex])
                {
                    currentClipName = component.runtimeClips[component.activeClipIndex]->getName().c_str();
                    if (strlen(currentClipName) == 0)
                        currentClipName = "Unnamed Clip";
                }

                if (ImGui::BeginCombo("Active Clip", currentClipName))
                {
                    for (size_t i = 0; i < component.runtimeClips.size(); i++)
                    {
                        bool isSelected = (component.activeClipIndex == static_cast<uint32_t>(i));
                        const auto &clip = component.runtimeClips[i];

                        std::string label;
                        if (clip)
                        {
                            label = clip->getName();
                            if (label.empty())
                                label = "Animation " + std::to_string(i);
                        }
                        else
                        {
                            label = "[Invalid] " + std::to_string(i);
                        }

                        if (ImGui::Selectable(label.c_str(), isSelected))
                        {
                            component.activeClipIndex = static_cast<uint32_t>(i);
                            if (component.runtimeAnimator && clip)
                            {
                                component.runtimeAnimator->play(clip);
                            }
                        }
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();

                        if (clip && ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();
                            ImGui::Text("Duration: %.2f ticks", clip->getDuration());
                            ImGui::Text("Ticks/sec: %.1f", clip->getTicksPerSecond());
                            ImGui::Text("Duration: %.2fs", clip->getDuration() / clip->getTicksPerSecond());
                            ImGui::Text("Channels: %zu", clip->getChannels().size());
                            ImGui::EndTooltip();
                        }
                    }
                    ImGui::EndCombo();
                }

                // Show current clip info
                if (component.activeClipIndex < component.runtimeClips.size())
                {
                    const auto &activeClip = component.runtimeClips[component.activeClipIndex];
                    if (activeClip)
                    {
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Duration: %.2fs",
                            activeClip->getDuration() / activeClip->getTicksPerSecond());
                    }
                }

                // List all clips with remove button
                ImGui::BeginChild("ClipList", ImVec2(0, 80), true);
                for (size_t i = 0; i < component.animationClipHandles.size(); i++)
                {
                    ImGui::PushID(static_cast<int>(i));
                    const auto &clip = component.runtimeClips[i];
                    std::string clipName = clip ? (clip->getName().empty() ? "Animation " + std::to_string(i) : clip->getName()) : "[Invalid]";

                    bool isActive = (component.activeClipIndex == static_cast<uint32_t>(i));
                    if (isActive)
                        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "> %s", clipName.c_str());
                    else
                        ImGui::Text("  %s", clipName.c_str());

                    if (ImGui::BeginPopupContextItem("ClipContextMenu"))
                    {
                        if (ImGui::MenuItem("Remove"))
                        {
                            component.removeAnimationClip(i);
                            ImGui::EndPopup();
                            ImGui::PopID();
                            break;
                        }
                        if (ImGui::MenuItem("Set Active"))
                        {
                            component.activeClipIndex = static_cast<uint32_t>(i);
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
                ImGui::EndChild();

                if (ImGui::Button("Clear All Clips", ImVec2(-1, 0)))
                {
                    component.clearAnimationClips();
                }
            }

            ImGui::Button("Drag Animation Here (.fanim)", ImVec2(-1, 20));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_ANIMATION"))
                {
                    if (auto view = payloadToStringView(payload))
                    {
                        AssetHandle handle = editorAssets->importAsset(std::filesystem::path(*view));
                        if (static_cast<uint64_t>(handle) != 0)
                        {
                            component.addAnimationClip(handle);
                            auto clip = editorAssets->getAsset<AnimationClip>(handle);
                            component.runtimeClips.push_back(clip);
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::Unindent();

            ImGui::Separator();
            ImGui::Text("Playback");

            ui::drawCheckboxControl("Playing", component.playing, 150.0f);
            ui::drawCheckboxControl("Looping", component.looping, 150.0f);
            ui::drawFloatControl("Speed", component.speed, 150.0f, 0.1f, 0.0f, 10.0f);

            // Runtime info
            if (component.runtimeAnimator)
            {
                ImGui::Separator();
                ImGui::Text("Runtime Info");
                ImGui::Indent();

                float currentTime = component.runtimeAnimator->getCurrentTime();
                ImGui::Text("Current Time: %.2f", currentTime);

                bool isPlaying = component.runtimeAnimator->isPlaying();
                ImGui::Text("Status: %s", isPlaying ? "Playing" : "Paused/Stopped");

                const auto &boneMatrices = component.runtimeAnimator->getFinalBoneMatrices();
                ImGui::Text("Bone Matrices: %zu", boneMatrices.size());

                ImGui::Unindent();
            }

            // Playback controls
            ImGui::Separator();
            float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;

            if (ImGui::Button("Play", ImVec2(buttonWidth, 0)))
            {
                component.playing = true;
                if (component.runtimeAnimator && !component.runtimeClips.empty() && component.activeClipIndex < component.runtimeClips.size())
                {
                    component.runtimeAnimator->play(component.runtimeClips[component.activeClipIndex]);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Pause", ImVec2(buttonWidth, 0)))
            {
                component.playing = false;
                if (component.runtimeAnimator)
                {
                    component.runtimeAnimator->pause();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop", ImVec2(buttonWidth, 0)))
            {
                component.playing = false;
                if (component.runtimeAnimator)
                {
                    component.runtimeAnimator->stop();
                }
            } });
        drawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto &component)
                                                 {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ui::drawFloatControl("Intensity", component.intensity, 150.0f, 0.1f, 0.0f);
            ui::drawCheckboxControl("Main Light", component.mainLight, 150.0f); });
        drawComponent<PointLightComponent>("Point Light", entity, [](auto &component)
                                           {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ui::drawFloatControl("Intensity", component.intensity, 150.0f, 0.1f, 0.0f);
            ui::drawFloatControl("Range", component.range, 150.0f, 0.1f, 0.0f); });
        drawComponent<SpotLightComponent>("Spot Light", entity, [](auto &c)
                                          {
            ImGui::ColorEdit4("Color", glm::value_ptr(c.color));
            ui::drawFloatControl("Intensity", c.intensity, 150.0f, 0.1f, 0.0f, 100.0f);
            ui::drawFloatControl("Range", c.range, 150.0f, 0.1f, 0.0f, 1000.0f);

            ImGui::Separator();
            ImGui::Text("Cone");

            ui::drawFloatControl("Angle (deg)", c.angle, 150.0f, 0.5f, 1.0f, 89.0f);
            ui::drawFloatControl("Softness", c.softness, 150.0f, 0.01f, 0.0f, 1.0f, "%.3f"); });

        drawComponent<TextComponent>("Text", entity, [](auto &component)
                                     {
            char buffer[1024];
            strncpy(buffer, component.textString.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';

            if (ImGui::InputTextMultiline("Text String", buffer, sizeof(buffer))) {
                component.textString = buffer;
            }
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ui::drawFloatControl("Kerning", component.kerning, 150.0f, 0.025f);
            ui::drawFloatControl("Line Spacing", component.lineSpacing, 150.0f, 0.025f); });

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

        drawComponent<ScriptContainerComponent>("Scripts Container", entity, [&, entity](auto &component) mutable
                                                {
            static int selectedScriptIndex = -1;

            const auto &allClasses = ScriptManager::getALLEntityClasses();
            const size_t scriptCount = component.scriptClassNames.size();

            if (selectedScriptIndex >= static_cast<int>(scriptCount))
                selectedScriptIndex = -1;

            //  已添加的脚本列表 
            ImGui::Text("Attached Scripts");
            ImGui::BeginChild("ScriptList", ImVec2(0, 120), true, ImGuiWindowFlags_HorizontalScrollbar);
            for (size_t i = 0; i < scriptCount; i++)
            {
                ImGui::PushID(static_cast<int>(i));

                const std::string &scriptName = component.scriptClassNames[i];
                bool isSelected = (selectedScriptIndex == static_cast<int>(i));

                if (ImGui::Selectable(getScriptDisplayName(scriptName).c_str(), isSelected))
                    selectedScriptIndex = static_cast<int>(i);

                // 右键菜单
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Remove"))
                    {
                        component.scriptClassNames.erase(component.scriptClassNames.begin() + i);
                        if (selectedScriptIndex == static_cast<int>(i))
                            selectedScriptIndex = -1;
                        else if (selectedScriptIndex > static_cast<int>(i))
                            selectedScriptIndex--;
                        ImGui::EndPopup();
                        ImGui::PopID();
                        break;
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }
            if (scriptCount == 0)
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No scripts attached");
            ImGui::EndChild();

            //  添加脚本按钮 
            if (ImGui::Button("Add Script", ImVec2(-1, 0)))
                ImGui::OpenPopup("AddScriptPopup");

            if (ImGui::BeginPopup("AddScriptPopup"))
            {
                ImGui::Text("Available Scripts:");
                ImGui::Separator();

                bool hasAvailable = false;
                for (const auto &name : allClasses)
                {
                    if (!isUserScript(name))
                        continue;

                    bool alreadyAdded = std::find(component.scriptClassNames.begin(),
                                                  component.scriptClassNames.end(),
                                                  name) != component.scriptClassNames.end();
                    if (alreadyAdded)
                        continue;

                    hasAvailable = true;
                    if (ImGui::Selectable(getScriptDisplayName(name).c_str()))
                    {
                        component.scriptClassNames.push_back(name);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", name.c_str());
                }

                if (!hasAvailable)
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No more scripts available");

                ImGui::EndPopup();
            }

            //  选中脚本的详情面板 
            if (selectedScriptIndex >= 0 && selectedScriptIndex < static_cast<int>(scriptCount))
            {
                ImGui::Separator();

                const std::string &selectedScript = component.scriptClassNames[selectedScriptIndex];
                ImGui::Text("Script: %s", getScriptDisplayName(selectedScript).c_str());
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", selectedScript.c_str());
                ImGui::Spacing();

                auto scriptClass = ScriptManager::getScriptClass(selectedScript);
                if (scriptClass)
                {
                    const auto &fields = scriptClass->getFields();
                    if (!fields.empty())
                    {
                        auto instance = ScriptManager::getEntityScriptInstance(entity.getUUID(), selectedScript);
                        ImGui::Text(instance ? "Public Fields (Runtime):" : "Public Fields:");
                        ImGui::Separator();

                        for (const auto &[fieldName, field] : fields)
                            drawScriptField(fieldName, field, instance);
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No public fields");
                    }
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Script class not loaded");
                }

                ImGui::Spacing();
                if (ImGui::Button("Remove Script", ImVec2(-1, 0)))
                {
                    component.scriptClassNames.erase(component.scriptClassNames.begin() + selectedScriptIndex);
                    selectedScriptIndex = -1;
                }
            }

            //  清空所有脚本
            if (scriptCount > 0)
            {
                ImGui::Separator();
                if (ImGui::Button("Clear All Scripts", ImVec2(-1, 0)))
                {
                    component.scriptClassNames.clear();
                    selectedScriptIndex = -1;
                }
            } });

        drawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto &component)
                                               {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
            ui::drawFloatControl("Thickness", component.thickness, 150.0f, 0.025f, 0.0f, 1.0f);
            ui::drawFloatControl("Fade", component.fade, 150.0f, 0.00025f, 0.0f, 1.0f); });

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

            ui::drawCheckboxControl("Fixed Rotation", component.fixedRotation, 150.0f); });

        drawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto &component)
                                              {
            ui::drawVec2Control("Offset",component.offset,0.0f,100.0f,0.1f);
            ui::drawVec2Control("Size",component.size,0.0f,100.0f,0.1f);
            ui::drawFloatControl("Density", component.density, 150.0f, 0.01f, 0.0f, 1.0f);
            ui::drawFloatControl("Friction", component.friction, 150.0f, 0.01f, 0.0f, 1.0f);
            ui::drawFloatControl("Restitution", component.restitution, 150.0f, 0.01f, 0.0f, 1.0f);
            ui::drawFloatControl("Restitution Threshold", component.restitutionThreshold, 150.0f, 0.01f, 0.0f); });
        drawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto &component)
                                                 {
            ui::drawVec2Control("Offset",component.offset,0.0f,100.0f,0.1f);
            ui::drawFloatControl("Radius", component.radius, 150.0f);
            ui::drawFloatControl("Density", component.density, 150.0f, 0.01f, 0.0f, 1.0f);
            ui::drawFloatControl("Friction", component.friction, 150.0f, 0.01f, 0.0f, 1.0f);
            ui::drawFloatControl("Restitution", component.restitution, 150.0f, 0.01f, 0.0f, 1.0f);
            ui::drawFloatControl("Restitution Threshold", component.restitutionThreshold, 150.0f, 0.01f, 0.0f); });
        drawComponent<BoxSensor2DComponent>("Box Sensor 2D", entity, [](auto &component)
                                            {
            ui::drawVec2Control("Offset",component.offset,0.0f,100.0f,0.1f); 
            ui::drawVec2Control("Size",component.size,0.0f,100.0f,0.1f); });
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

            ui::drawFloatControl("Mass",component.mass,150.0f,0.01f,0.0f,1000.0f);
            ui::drawFloatControl("Linear Damping",component.linearDamping,150.0f,0.01f,0.0f,10.0f);
            ui::drawFloatControl("Angular Damping",component.angularDamping,150.0f,0.01f,0.0f,10.0f);
            ui::drawCheckboxControl("Use Gravity", component.useGravity, 150.0f);
            ui::drawCheckboxControl("Fixed Rotation", component.fixedRotation, 150.0f); });
        drawComponent<BoxCollider3DComponent>("Box Collider 3D", entity, [](auto &component)
                                              {
            ui::drawVec3Control("Offset",component.offset,0.0f,100.0f,0.1f);
            ui::drawVec3Control("Size",component.size,0.0f,100.0f,0.1f);
            ui::drawFloatControl("Density",component.density,150.0f,0.01f,0.0f,10.0f);
            ui::drawFloatControl("Friction",component.friction,150.0f,0.01f,0.0f,1.0f);
            ui::drawFloatControl("Restitution",component.restitution,150.0f,0.01f,0.0f,1.0f);
            ui::drawCheckboxControl("Trigger", component.isTrigger, 150.0f); });
        drawComponent<CircleCollider3DComponent>("Circle Collider 3D", entity, [](auto &component)
                                                 {
            ui::drawVec3Control("Offset",component.offset,0.0f,100.0f,0.1f);
            ui::drawFloatControl("Radius",component.radius,150.0f,0.01f,0.0f,100.0f);
            ui::drawFloatControl("Density",component.density,150.0f,0.01f,0.0f,10.0f);
            ui::drawFloatControl("Friction",component.friction,150.0f,0.01f,0.0f,1.0f);
            ui::drawFloatControl("Restitution",component.restitution,150.0f,0.01f,0.0f,1.0f);
            ui::drawCheckboxControl("Trigger", component.isTrigger, 150.0f); });
        drawComponent<CapsuleCollider3DComponent>("Capsule Collider 3D", entity, [](auto &component)
                                                  {
            ui::drawVec3Control("Offset",component.offset,0.0f,100.0f,0.1f);
            ui::drawFloatControl("Radius",component.radius,150.0f,0.01f,0.0f,100.0f);
            ui::drawFloatControl("Height",component.height,150.0f,0.01f,0.0f,100.0f);
            ui::drawFloatControl("Density",component.density,150.0f,0.01f,0.0f,10.0f);
            ui::drawFloatControl("Friction",component.friction,150.0f,0.01f,0.0f,1.0f);
            ui::drawFloatControl("Restitution",component.restitution,150.0f,0.01f,0.0f,1.0f);
            ui::drawCheckboxControl("Trigger", component.isTrigger, 150.0f); });
        drawComponent<MeshCollider3DComponent>("Mesh Collider 3D", entity, [](auto &component)
                                               {
            auto editorAssets = Project::getEditorAssetManager();

            ui::drawVec3Control("Offset",component.offset,0.0f,100.0f,0.1f);

            ImGui::Separator();
            ImGui::Text("Mesh Source");
            ImGui::Text("Mesh Handle: %llu", static_cast<uint64_t>(component.meshHandle));

            ImGui::Button("Drag Mesh Here", ImVec2(-1, 20));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_MODEL"))
                {
                    if (auto view = payloadToStringView(payload))
                    {
                        AssetHandle modelHandle = editorAssets->importAsset(std::filesystem::path(*view));
                        if (static_cast<uint64_t>(modelHandle) != 0)
                        {
                            auto modelAsset = editorAssets->getAsset<ModelAsset>(modelHandle);
                            if (modelAsset)
                                component.meshHandle = modelAsset->mesh;
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::Button("Clear Mesh", ImVec2(-1, 0)))
                component.meshHandle = AssetHandle(0);

            ImGui::Separator();
            ui::drawCheckboxControl("Convex", component.convex, 150.0f);
            ui::drawFloatControl("Density",component.density,150.0f,0.01f,0.0f,10.0f);
            ui::drawFloatControl("Friction",component.friction,150.0f,0.01f,0.0f,1.0f);
            ui::drawFloatControl("Restitution",component.restitution,150.0f,0.01f,0.0f,1.0f);
            ui::drawCheckboxControl("Trigger", component.isTrigger, 150.0f); });
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
