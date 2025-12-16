#include "AssetManagerPanel.hpp"

#include <imgui.h>

#include "Asset/AssetRegistry.hpp"
#include "Asset/AssetTypes.hpp"
#include "Asset/Asset.hpp"
#include "Renderer/Texture.hpp"
#include "Project/Project.hpp"

namespace Fermion
{
    static const char *AssetTypeToString(AssetType type)
    {
        switch (type)
        {
        case AssetType::Texture:
            return "Texture";
        case AssetType::Scene:
            return "Scene";
        case AssetType::Font:
            return "Font";
        case AssetType::Shader:
            return "Shader";
        default:
            return "None";
        }
    }

    void AssetManagerPanel::onImGuiRender()
    {
        
        ImGui::Begin("Asset Manager");

        auto editorAssets = Project::getEditorAssetManager();

        const auto &registry = AssetRegistry::getRegistry();

        if (registry.empty())
        {
            ImGui::TextUnformatted("No assets registered.");
            ImGui::End();
            
            return;
        }

        if (ImGui::BeginTable("AssetTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("Handle");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Path");
            ImGui::TableSetupColumn("Status");
            ImGui::TableHeadersRow();

            for (const auto &[handle, info] : registry)
            {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%llu", (uint64_t)handle);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(AssetTypeToString(info.Type));

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(info.Name.c_str());

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(info.FilePath.string().c_str());

                ImGui::TableNextColumn();

                bool loaded = editorAssets->isAssetLoaded(handle);
                ImGui::TextUnformatted(loaded ? "Loaded" : "Unloaded");
                ImGui::SameLine();

               
                ImGui::PushID((int)handle);

                if (loaded)
                {
                    if (ImGui::SmallButton("Unload"))
                        editorAssets->unloadAsset(handle);
                }
                else
                {
                    if (ImGui::SmallButton("Load"))
                        editorAssets->getAsset<Asset>(handle);
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        ImGui::End();
      
    }
}
