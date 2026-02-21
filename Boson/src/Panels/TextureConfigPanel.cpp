#include "TextureConfigPanel.hpp"
#include "Asset/Importer/TextureImporter.hpp"
#include "Asset/AssetManager/EditorAssetManager.hpp"
#include "Project/Project.hpp"
#include "Core/Log.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <format>

namespace Fermion
{
    TextureConfigPanel::TextureConfigPanel()
    {
        Log::Info(std::format("TextureConfigPanel created"));
    }

    void TextureConfigPanel::setPanelOpenState(bool state)
    {
        Log::Info(std::format("TextureConfigPanel::setPanelOpenState called with state: {}", state));
        m_IsOpened = state;
    }

    void TextureConfigPanel::clearData()
    {
        m_Textures.clear();
        m_SelectedIndex = -1;
        m_BatchMode = false;
    }

    void TextureConfigPanel::loadTexture(const std::filesystem::path &ftexPath)
    {
        Log::Info(std::format("TextureConfigPanel::loadTexture called with path: {}", ftexPath.string()));

        if (ftexPath.extension() != ".ftex")
        {
            Log::Warn(std::format("File is not a .ftex file: {}", ftexPath.string()));
            return;
        }

        // Check if already loaded
        for (size_t i = 0; i < m_Textures.size(); ++i)
        {
            if (m_Textures[i].FtexPath == ftexPath)
            {
                m_SelectedIndex = static_cast<int>(i);
                return;
            }
        }

        TextureConfigData data;
        data.FtexPath = ftexPath;
        data.Name = ftexPath.stem().string();
        data.Spec = TextureImporter::deserializeFtex(ftexPath);
        data.Modified = false;

        m_Textures.push_back(data);
        m_SelectedIndex = static_cast<int>(m_Textures.size() - 1);
        m_BatchMode = false;
    }

    void TextureConfigPanel::loadTextures(const std::vector<std::filesystem::path> &ftexPaths)
    {
        if (ftexPaths.empty())
            return;

        clearData();

        for (const auto &path : ftexPaths)
        {
            if (path.extension() != ".ftex")
                continue;

            TextureConfigData data;
            data.FtexPath = path;
            data.Name = path.stem().string();
            data.Spec = TextureImporter::deserializeFtex(path);
            data.Modified = false;

            m_Textures.push_back(data);
        }

        if (!m_Textures.empty())
        {
            m_SelectedIndex = 0;
            m_BatchMode = m_Textures.size() > 1;

            if (m_BatchMode)
            {
                // Initialize batch spec with first texture's settings
                m_BatchSpec = m_Textures[0].Spec;
            }
        }
    }

    void TextureConfigPanel::onImGuiRender()
    {
        if (!m_IsOpened)
            return;

        if (!ImGui::Begin("Texture Config", &m_IsOpened))
        {
            ImGui::End();
            return;
        }

        if (m_Textures.empty())
        {
            ImGui::TextDisabled("Drag and drop .ftex files here");

            // Handle drag-drop on empty window
            ImGuiWindow *window = ImGui::GetCurrentWindow();
            if (ImGui::BeginDragDropTargetCustom(window->ContentRegionRect, window->ID))
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_FTEX"))
                {
                    const char *path = static_cast<const char *>(payload->Data);
                    if (path && path[0])
                        loadTexture(std::filesystem::path(path));
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::End();
            return;
        }

        // Top bar
        if (ImGui::Button("Save All"))
            saveAllTextures();

        ImGui::SameLine();
        if (ImGui::Button("Clear All"))
            clearData();

        ImGui::SameLine();
        ImGui::Text("Loaded: %zu texture(s)", m_Textures.size());

        if (m_Textures.size() > 1)
        {
            ImGui::SameLine();
            ImGui::Checkbox("Batch Mode", &m_BatchMode);
        }

        ImGui::Separator();

        // Layout: Left panel (texture list) | Right panel (config)
        if (ImGui::BeginTable("TextureConfigLayout", 2, ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("List", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableSetupColumn("Config", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            // Left: Texture list
            ImGui::BeginChild("TextureList", ImVec2(0, 0), true);

            for (int i = 0; i < static_cast<int>(m_Textures.size()); ++i)
            {
                bool selected = (m_SelectedIndex == i);
                std::string label = m_Textures[i].Name;
                if (m_Textures[i].Modified)
                    label += " *";

                if (ImGui::Selectable(label.c_str(), selected))
                {
                    m_SelectedIndex = i;
                    m_BatchMode = false;
                }
            }

            // Allow dropping more .ftex files into the list
            ImGuiWindow *listWindow = ImGui::GetCurrentWindow();
            if (ImGui::BeginDragDropTargetCustom(listWindow->ContentRegionRect, listWindow->ID))
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_FTEX"))
                {
                    const char *path = static_cast<const char *>(payload->Data);
                    if (path && path[0])
                        loadTexture(std::filesystem::path(path));
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::EndChild();

            ImGui::TableNextColumn();

            // Right: Config panel
            ImGui::BeginChild("ConfigPanel", ImVec2(0, 0), true);

            if (m_BatchMode && m_Textures.size() > 1)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Batch Edit Mode: %zu textures", m_Textures.size());
                ImGui::Text("Changes will be applied to all loaded textures immediately");
                ImGui::Separator();

                if (ImGui::CollapsingHeader("Batch Settings", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (ImGui::Checkbox("Generate Mipmaps##Batch", &m_BatchSpec.GenerateMipmaps))
                    {
                        for (auto &tex : m_Textures)
                        {
                            tex.Spec.GenerateMipmaps = m_BatchSpec.GenerateMipmaps;
                            tex.Modified = true;
                        }
                    }

                    const char *filterModes[] = {"Nearest", "Linear"};
                    int minFilter = static_cast<int>(m_BatchSpec.MinFilter);
                    if (ImGui::Combo("Min Filter##Batch", &minFilter, filterModes, 2))
                    {
                        m_BatchSpec.MinFilter = static_cast<TextureFilterMode>(minFilter);
                        for (auto &tex : m_Textures)
                        {
                            tex.Spec.MinFilter = m_BatchSpec.MinFilter;
                            tex.Modified = true;
                        }
                    }

                    int magFilter = static_cast<int>(m_BatchSpec.MagFilter);
                    if (ImGui::Combo("Mag Filter##Batch", &magFilter, filterModes, 2))
                    {
                        m_BatchSpec.MagFilter = static_cast<TextureFilterMode>(magFilter);
                        for (auto &tex : m_Textures)
                        {
                            tex.Spec.MagFilter = m_BatchSpec.MagFilter;
                            tex.Modified = true;
                        }
                    }

                    const char *wrapModes[] = {"Repeat", "ClampToEdge", "MirroredRepeat"};
                    int wrapS = static_cast<int>(m_BatchSpec.WrapS);
                    if (ImGui::Combo("Wrap S##Batch", &wrapS, wrapModes, 3))
                    {
                        m_BatchSpec.WrapS = static_cast<TextureWrapMode>(wrapS);
                        for (auto &tex : m_Textures)
                        {
                            tex.Spec.WrapS = m_BatchSpec.WrapS;
                            tex.Modified = true;
                        }
                    }

                    int wrapT = static_cast<int>(m_BatchSpec.WrapT);
                    if (ImGui::Combo("Wrap T##Batch", &wrapT, wrapModes, 3))
                    {
                        m_BatchSpec.WrapT = static_cast<TextureWrapMode>(wrapT);
                        for (auto &tex : m_Textures)
                        {
                            tex.Spec.WrapT = m_BatchSpec.WrapT;
                            tex.Modified = true;
                        }
                    }

                    if (ImGui::SliderFloat("Anisotropy##Batch", &m_BatchSpec.Anisotropy, 1.0f, 16.0f))
                    {
                        for (auto &tex : m_Textures)
                        {
                            tex.Spec.Anisotropy = m_BatchSpec.Anisotropy;
                            tex.Modified = true;
                        }
                    }

                    if (ImGui::Checkbox("sRGB##Batch", &m_BatchSpec.sRGB))
                    {
                        for (auto &tex : m_Textures)
                        {
                            tex.Spec.sRGB = m_BatchSpec.sRGB;
                            tex.Modified = true;
                        }
                    }
                }

                ImGui::Separator();
                ImGui::Text("Click on a texture in the list to edit individually");
            }
            else if (m_SelectedIndex >= 0 && m_SelectedIndex < static_cast<int>(m_Textures.size()))
            {
                drawTextureConfig(m_Textures[m_SelectedIndex]);
            }

            ImGui::EndChild();
            ImGui::EndTable();
        }

        ImGui::End();

        if (!m_IsOpened)
            clearData();
    }

    void TextureConfigPanel::drawTextureConfig(TextureConfigData &data)
    {
        ImGui::PushID(&data);

        ImGui::Text("Name: %s", data.Name.c_str());
        ImGui::Text("Path: %s", data.FtexPath.string().c_str());
        ImGui::Text("Source: %s", data.Spec.SourcePath.string().c_str());

        ImGui::Separator();

        if (ImGui::Checkbox("Generate Mipmaps", &data.Spec.GenerateMipmaps))
            data.Modified = true;

        const char *filterModes[] = {"Nearest", "Linear"};
        int minFilter = static_cast<int>(data.Spec.MinFilter);
        if (ImGui::Combo("Min Filter", &minFilter, filterModes, 2))
        {
            data.Spec.MinFilter = static_cast<TextureFilterMode>(minFilter);
            data.Modified = true;
        }

        int magFilter = static_cast<int>(data.Spec.MagFilter);
        if (ImGui::Combo("Mag Filter", &magFilter, filterModes, 2))
        {
            data.Spec.MagFilter = static_cast<TextureFilterMode>(magFilter);
            data.Modified = true;
        }

        const char *wrapModes[] = {"Repeat", "ClampToEdge", "MirroredRepeat"};
        int wrapS = static_cast<int>(data.Spec.WrapS);
        if (ImGui::Combo("Wrap S", &wrapS, wrapModes, 3))
        {
            data.Spec.WrapS = static_cast<TextureWrapMode>(wrapS);
            data.Modified = true;
        }

        int wrapT = static_cast<int>(data.Spec.WrapT);
        if (ImGui::Combo("Wrap T", &wrapT, wrapModes, 3))
        {
            data.Spec.WrapT = static_cast<TextureWrapMode>(wrapT);
            data.Modified = true;
        }

        if (ImGui::SliderFloat("Anisotropy", &data.Spec.Anisotropy, 1.0f, 16.0f))
            data.Modified = true;

        if (ImGui::Checkbox("sRGB", &data.Spec.sRGB))
            data.Modified = true;

        ImGui::Separator();

        if (data.Modified)
        {
            if (ImGui::Button("Save"))
                saveTexture(data);
            ImGui::SameLine();
            if (ImGui::Button("Revert"))
            {
                data.Spec = TextureImporter::deserializeFtex(data.FtexPath);
                data.Modified = false;
            }
        }

        ImGui::PopID();
    }

    void TextureConfigPanel::saveTexture(TextureConfigData &data)
    {
        Log::Info(std::format("Saving texture config: {}", data.FtexPath.string()));
        TextureImporter::serializeFtex(data.FtexPath, data.Spec);
        data.Modified = false;

        // Hot reload: find the asset handle and reload it
        auto editorAssets = Project::getEditorAssetManager();
        if (editorAssets)
        {
            AssetHandle handle = editorAssets->importAsset(data.FtexPath);
            if (static_cast<uint64_t>(handle) != 0)
            {
                Log::Info(std::format("Reloading texture asset - Path: {}, Handle: {}",
                    data.FtexPath.string(), static_cast<uint64_t>(handle)));

                // Check if asset is loaded before reload
                bool wasLoaded = editorAssets->isAssetLoaded(handle);
                Log::Info(std::format("Asset was loaded before reload: {}", wasLoaded));

                editorAssets->reloadAsset(handle);

                // Verify it's loaded after reload
                bool isLoadedAfter = editorAssets->isAssetLoaded(handle);
                Log::Info(std::format("Asset is loaded after reload: {}", isLoadedAfter));
            }
            else
            {
                Log::Warn(std::format("Failed to get asset handle for: {}", data.FtexPath.string()));
            }
        }
        else
        {
            Log::Warn("EditorAssetManager is null");
        }
    }

    void TextureConfigPanel::saveAllTextures()
    {
        for (auto &tex : m_Textures)
        {
            if (tex.Modified)
                saveTexture(tex);
        }
    }

} // namespace Fermion
