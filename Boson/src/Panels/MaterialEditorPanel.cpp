#include "MaterialEditorPanel.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Renderer/Model/MaterialFactory.hpp"
#include "Project/Project.hpp"
namespace Fermion
{

    void MaterialEditorPanel::onImGuiRender()
    {

        ImGui::Begin("Material Editor");
        ImGui::Checkbox("Use PBR Material", &m_isCreatePBRMaterial);
        InputTextStdString("Name", m_MaterialInfo.Name);
        ImGui::BeginDisabled(!m_isCreatePBRMaterial);
        ImGui::SeparatorText("PBR Numeric Material");
        {
            ImGui::Button("Change Material");
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_MATERIAL"))
                {
                    auto path = std::string(static_cast<const char *>(payload->Data));
                    auto editorAssets = Project::getEditorAssetManager();
                    AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                    if (static_cast<uint64_t>(handle) != 0)
                    {
                        auto material = editorAssets->getAsset<Material>(handle);
                        m_MaterialInfo.Name = material->getName();
                        m_MaterialInfo.Type = material->getType();
                        if (m_MaterialInfo.Type == MaterialType::PBR)
                        {
                            m_MaterialInfo.Albedo = material->getAlbedo();
                            m_MaterialInfo.Metallic = material->getMetallic();
                            m_MaterialInfo.Roughness = material->getRoughness();
                            m_MaterialInfo.AO = material->getAO();
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }

        ImGui::ColorEdit3("Albedo", glm::value_ptr(m_MaterialInfo.Albedo));
        ImGui::SliderFloat("Metallic", &m_MaterialInfo.Metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &m_MaterialInfo.Roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("Ambient Occlusion", &m_MaterialInfo.AO, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::Text("Texture Maps");

        // Albedo Map
        ImGui::Text("Albedo Map");
        ImGui::SameLine();
        ImGui::Button("Drag texture##albedo");
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0])
                {
                    auto editorAssets = Project::getEditorAssetManager();
                    AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                    if (static_cast<uint64_t>(handle) != 0)
                    {
                        m_MaterialInfo.Maps.AlbedoMapHandle = handle;
                        m_useTexture = true;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        if (static_cast<uint64_t>(m_MaterialInfo.Maps.AlbedoMapHandle) != 0)
        {
            ImGui::SameLine();
            ImGui::Text("(%llu)", static_cast<uint64_t>(m_MaterialInfo.Maps.AlbedoMapHandle));

            auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(m_MaterialInfo.Maps.AlbedoMapHandle);
            if (texture && texture->isLoaded())
            {
                ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
            }
        }

        // Normal Map
        ImGui::Text("Normal Map");
        ImGui::SameLine();
        ImGui::Button("Drag texture##normal");
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0])
                {
                    auto editorAssets = Project::getEditorAssetManager();
                    AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                    if (static_cast<uint64_t>(handle) != 0)
                    {
                        m_MaterialInfo.Maps.NormalMapHandle = handle;
                        m_useTexture = true;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        if (static_cast<uint64_t>(m_MaterialInfo.Maps.NormalMapHandle) != 0)
        {
            ImGui::SameLine();
            ImGui::Text("(%llu)", static_cast<uint64_t>(m_MaterialInfo.Maps.NormalMapHandle));

            auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(m_MaterialInfo.Maps.NormalMapHandle);
            if (texture && texture->isLoaded())
            {
                ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
            }
        }

        // Metallic Map
        ImGui::Text("Metallic Map");
        ImGui::SameLine();
        ImGui::Button("Drag texture##metallic");
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0])
                {
                    auto editorAssets = Project::getEditorAssetManager();
                    AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                    if (static_cast<uint64_t>(handle) != 0)
                    {
                        m_MaterialInfo.Maps.MetallicMapHandle = handle;
                        m_useTexture = true;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        if (static_cast<uint64_t>(m_MaterialInfo.Maps.MetallicMapHandle) != 0)
        {
            ImGui::SameLine();
            ImGui::Text("(%llu)", static_cast<uint64_t>(m_MaterialInfo.Maps.MetallicMapHandle));

            auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(m_MaterialInfo.Maps.MetallicMapHandle);
            if (texture && texture->isLoaded())
            {
                ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
            }
        }

        // Roughness Map
        ImGui::Text("Roughness Map");
        ImGui::SameLine();
        ImGui::Button("Drag texture##roughness");
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0])
                {
                    auto editorAssets = Project::getEditorAssetManager();
                    AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                    if (static_cast<uint64_t>(handle) != 0)
                    {
                        m_MaterialInfo.Maps.RoughnessMapHandle = handle;
                        m_useTexture = true;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        if (static_cast<uint64_t>(m_MaterialInfo.Maps.RoughnessMapHandle) != 0)
        {
            ImGui::SameLine();
            ImGui::Text("(%llu)", static_cast<uint64_t>(m_MaterialInfo.Maps.RoughnessMapHandle));

            auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(m_MaterialInfo.Maps.RoughnessMapHandle);
            if (texture && texture->isLoaded())
            {
                ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
            }
        }

        // AO Map
        ImGui::Text("AO Map");
        ImGui::SameLine();
        ImGui::Button("Drag texture##ao");
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0])
                {
                    auto editorAssets = Project::getEditorAssetManager();
                    AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                    if (static_cast<uint64_t>(handle) != 0)
                    {
                        m_MaterialInfo.Maps.AOMapHandle = handle;
                        m_useTexture = true;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        if (static_cast<uint64_t>(m_MaterialInfo.Maps.AOMapHandle) != 0)
        {
            ImGui::SameLine();
            ImGui::Text("(%llu)", static_cast<uint64_t>(m_MaterialInfo.Maps.AOMapHandle));

            auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(m_MaterialInfo.Maps.AOMapHandle);
            if (texture && texture->isLoaded())
            {
                ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                ImGui::Image(textureID, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
            }
        }

        ImGui::Separator();
        ImGui::TextWrapped("Tip: Drag texture files from Content Browser to assign maps");
        if (ImGui::Button("Create Material"))
        {
            if (!m_useTexture)
            {
                MaterialFactory::createMaterial(
                    m_MaterialInfo.Name,
                    m_MaterialInfo.Albedo,
                    m_MaterialInfo.Metallic,
                    m_MaterialInfo.Roughness,
                    m_MaterialInfo.AO);
            }
            else
            {
                MaterialFactory::createMaterial(
                    m_MaterialInfo.Name,
                    m_MaterialInfo.Maps,
                    m_MaterialInfo.Albedo,
                    m_MaterialInfo.Metallic,
                    m_MaterialInfo.Roughness,
                    m_MaterialInfo.AO);
            }
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled(m_isCreatePBRMaterial);

        ImGui::Text("Phong Lighting Model");
        ImGui::Separator();

        ImGui::ColorEdit4("Diffuse Color", glm::value_ptr(m_MaterialInfo.DiffuseColor));
        ImGui::ColorEdit4("Ambient Color", glm::value_ptr(m_MaterialInfo.AmbientColor));

        ImGui::Separator();
        ImGui::Text("Texture Settings");

        ImGui::Text("Diffuse Texture");
        ImGui::SameLine();
        ImGui::Button("Drag texture here##phong");

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0])
                {
                    auto editorAssets = Project::getEditorAssetManager();
                    AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                    if (static_cast<uint64_t>(handle) != 0)
                    {
                        m_MaterialInfo.DiffuseTextureHandle = handle;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (static_cast<uint64_t>(m_MaterialInfo.DiffuseTextureHandle) != 0)
        {
            ImGui::Text("Handle: %llu", static_cast<uint64_t>(m_MaterialInfo.DiffuseTextureHandle));
        }
        if (ImGui::Button("Create Phong Material"))
        {
            MaterialFactory::createMaterial(
                m_MaterialInfo.Name,
                m_MaterialInfo.DiffuseColor,
                m_MaterialInfo.AmbientColor,
                m_MaterialInfo.DiffuseTextureHandle);
        }
        ImGui::EndDisabled();
        ImGui::End();
    }
}