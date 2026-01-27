#pragma once
namespace Fermion
{
    namespace ui
    {
        bool BeginPopup(const char *str_id, ImGuiWindowFlags flags = 0)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));

            bool is_open = ImGui::BeginPopup(str_id, flags);

            if (!is_open)
            {
                ImGui::PopStyleVar();
            }
            return is_open;
        }

        void EndPopup()
        {
            ImGui::EndPopup();
            ImGui::PopStyleVar(); 
        }
    }
}