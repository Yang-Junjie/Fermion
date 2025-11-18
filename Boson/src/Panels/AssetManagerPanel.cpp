#include "AssetManagerPanel.hpp"
#include <imgui.h>
namespace Fermion
{
    void AssetManagerPanel::onImGuiRender()
    {
        ImGui::Begin("Asset Manager");

        ImGui::Text("Assets");
        ImGui::Separator();
        ImGui::End();
    }
}