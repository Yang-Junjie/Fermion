#include "AssetManagerPanel.hpp"

#include <imgui.h>

#include "Asset/AssetRegistry.hpp"
#include "Asset/AssetTypes.hpp"
#include "Asset/AssetExtensions.hpp"
#include "Asset/Asset.hpp"
#include "Renderer/Texture/Texture.hpp"
#include "Project/Project.hpp"

namespace Fermion
{
    void AssetManagerPanel::onImGuiRender()
    {
        ImGui::Begin("Asset Manager");

        auto editorAssets = Project::getEditorAssetManager();
        const auto &registry = AssetRegistry::getRegistry();

 
        static ImGuiTextFilter filter;
        filter.Draw("Search Assets...", ImGui::GetContentRegionAvail().x * 0.7f);
        ImGui::SameLine();
        if (ImGui::Button("Refresh"))
        {
        }

        ImGui::Separator();

        if (registry.empty())
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No assets registered.");
            ImGui::End();
            return;
        }

        ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable |
                                ImGuiTableFlags_ScrollY;

        if (ImGui::BeginTable("AssetTable", 5, flags))
        {            
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f); 

            ImGui::TableHeadersRow();

            for (const auto &[handle, info] : registry)
            {
                if (!filter.PassFilter(info.Name.c_str()) && !filter.PassFilter(info.FilePath.string().c_str()))
                    continue;

                ImGui::TableNextRow();
                ImGui::PushID((int)handle);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(GetAssetExtensionFromType(info.Type).c_str());

             
                ImGui::TableNextColumn();
                bool isSelected = false; 
                if (ImGui::Selectable(info.Name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
                {
                  
                }

                ImGui::TableNextColumn();
                ImGui::TextDisabled("%s", info.FilePath.string().c_str());

                ImGui::TableNextColumn();
                bool loaded = editorAssets->isAssetLoaded(handle);

                if (loaded)
                {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Loaded");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Unloaded");
                }

                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20); 
                if (loaded)
                {
                    if (ImGui::SmallButton("U"))
                    {
                        editorAssets->unloadAsset(handle);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Unload Asset");
                }
                else
                {
                    if (ImGui::SmallButton("L"))
                    {
                        editorAssets->getAsset<Asset>(handle);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Load Asset");
                }

                ImGui::TableNextColumn();
                ImGui::TextDisabled(std::to_string(info.Handle).c_str());
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%llu", (uint64_t)handle);

                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }
} // namespace Fermion
