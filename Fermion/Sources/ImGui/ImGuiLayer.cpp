#include "ImGuiLayer.hpp"
#include "imgui.h"

namespace Fermion
{
    ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

    void ImGuiLayer::OnAttach()
    {
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
    }

    void ImGuiLayer::OnDetach() {}

    void ImGuiLayer::OnUpdate(Timestep dt)
    {
        ImGui::Begin("Hello, world!");
        ImGui::Button("button");
        ImGui::ShowDemoWindow();
        ImGui::End();
    }

    void ImGuiLayer::OnEvent(IEvent &e) {}

}
