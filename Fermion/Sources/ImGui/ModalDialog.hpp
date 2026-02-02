#pragma once
#include <imgui.h>
#include <string>
#include <functional>


namespace Fermion
{
    namespace ui
    {
        class ModalDialog
        {
        public:
            struct ModalConfig
            {
                std::string Title;
                bool ShowConfirm = true;
                bool ShowCancel = true;
                bool ShowClose = true;

                std::string ConfirmText = "Confirm";
                std::string CancelText = "Cancel";

                std::function<void()> ContentFunc = nullptr;
                std::function<void()> ConfirmFunc = nullptr;
            };
            static void Show(ModalConfig config)
            {
                s_Config = std::move(config);
                s_PendingOpen = true;
            }

            static void OnImGuiRender()
            {
                if (s_PendingOpen)
                {
                    ImGui::OpenPopup(s_Config.Title.c_str());
                    s_PendingOpen = false;
                }

                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                bool open = true;
                bool *p_open = s_Config.ShowClose ? &open : nullptr;

                if (ImGui::BeginPopupModal(s_Config.Title.c_str(), p_open, ImGuiWindowFlags_AlwaysAutoResize))
                {

                    if (s_Config.ContentFunc)
                    {
                        s_Config.ContentFunc();
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();


                    float buttonSize = 80.0f;
                    int visibleButtons = (s_Config.ShowConfirm ? 1 : 0) + (s_Config.ShowCancel ? 1 : 0);
                    if (visibleButtons > 0)
                    {
                        float widthNeeded = (visibleButtons * buttonSize) + (visibleButtons > 1 ? ImGui::GetStyle().ItemSpacing.x : 0);
                        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - widthNeeded + ImGui::GetCursorPosX());

                        if (s_Config.ShowCancel)
                        {
                            if (ImGui::Button(s_Config.CancelText.c_str(), ImVec2(buttonSize, 0)))
                            {
                                ImGui::CloseCurrentPopup();
                            }
                            if (s_Config.ShowConfirm)
                                ImGui::SameLine();
                        }

                        if (s_Config.ShowConfirm)
                        {
                            if (ImGui::Button(s_Config.ConfirmText.c_str(), ImVec2(buttonSize, 0)))
                            {
                                if (s_Config.ConfirmFunc)
                                    s_Config.ConfirmFunc();
                                ImGui::CloseCurrentPopup();
                            }
                        }
                    }

                    if (!open)
                        ImGui::CloseCurrentPopup();

                    ImGui::EndPopup();
                }
            }

        private:
            inline static ModalConfig s_Config = {};
            inline static bool s_PendingOpen = false;
        };
    }
}
