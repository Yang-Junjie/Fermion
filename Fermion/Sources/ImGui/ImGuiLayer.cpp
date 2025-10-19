#include "ImGuiLayer.hpp"
#include "fmpch.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "Platform/SFML/SFMLWindow.hpp"
#include <SFML/Graphics.hpp>

namespace Fermion
{
    ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer")
    {
    }

    void ImGuiLayer::OnAttach()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  

        ImGui::StyleColorsDark();
    }

    void ImGuiLayer::OnDetach()
    {
       
    }

    void ImGuiLayer::OnUpdate()
    {
       
    }

    void ImGuiLayer::OnEvent(IEvent &e)
    {
      
    }

    void ImGuiLayer::Begin()
    {
        ImGui::NewFrame();
        
        
    }

    void ImGuiLayer::End()
    {
    }

}