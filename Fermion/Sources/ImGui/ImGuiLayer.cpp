#include "ImGuiLayer.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace Fermion
{
    ImGuiLayer::ImGuiLayer(void *nativeWindow)
        : Layer("ImGuiLayer"), m_window(static_cast<GLFWwindow *>(nativeWindow))
    {
    }

    void ImGuiLayer::onAttach()
    {
        FM_PROFILE_FUNCTION();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 430");
    }

    void ImGuiLayer::onDetach()
    {
        FM_PROFILE_FUNCTION();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::onEvent(IEvent &e)
    {
        if (m_blockEvents)
        {
            ImGuiIO &io = ImGui::GetIO();
            e.handled |= e.isInCategory(EventCategory::EventCategoryMouse) & io.WantCaptureMouse;
            e.handled |= e.isInCategory(EventCategory::EventCategoryKeyboard) & io.WantCaptureKeyboard;
        }
    }
    void ImGuiLayer::onImGuiRender()
    {
    }
    void ImGuiLayer::begin()
    {
        FM_PROFILE_FUNCTION();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    void ImGuiLayer::end()
    {
        FM_PROFILE_FUNCTION();
        // 渲染
        ImGuiIO &io = ImGui::GetIO();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }
}
