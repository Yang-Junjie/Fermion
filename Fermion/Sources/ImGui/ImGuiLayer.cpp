#include "ImGuiLayer.hpp"
#include "Core/Log.hpp"
#include <imgui.h>
#include "fmpch.hpp"
#ifdef FM_PLATFORM_DESKTOP
#include <backends/imgui_impl_glfw.h>
#endif
#include <backends/imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <filesystem>

// TODO:该类直接依赖于opengl和glfw，记得抽象出来以支持跨平台
namespace Fermion
{
    ImGuiLayer::ImGuiLayer(void *nativeWindow)
        : Layer("ImGuiLayer"),
          m_window(static_cast<GLFWwindow *>(nativeWindow))
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

        const char *fontBoldPath = "../Boson/Resources/assets/fonts/Cuprum-Bold.ttf";
        const char *fontRegularPath = "../Boson/Resources/assets/fonts/Play-Regular.ttf";

        ImFont *fontRegular = nullptr;
        ImFont *fontBold = nullptr;

        if (std::filesystem::exists(fontRegularPath))
        {
            fontRegular = io.Fonts->AddFontFromFileTTF(fontRegularPath, 16.0f);
        }
        else
        {
            Log::Warn(std::format("Font file not found: {}", fontRegularPath));
        }

        if (std::filesystem::exists(fontBoldPath))
        {
            fontBold = io.Fonts->AddFontFromFileTTF(fontBoldPath, 16.0f);
        }
        else
        {
            Log::Warn(std::format("Font file not found: {}", fontRegularPath));
        }

        if (!fontRegular)
        {
            Log::Warn("Using default imgui font!");
            fontRegular = io.Fonts->AddFontDefault();
        }

        io.FontDefault = fontRegular;

        if (!fontBold)
            fontBold = io.Fonts->AddFontDefault();

        ImGui::StyleColorsDark();
        setDarkThemeColors();
        setImGuiWidgetStyle();

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
        ImGuizmo::BeginFrame();
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

    void ImGuiLayer::setImGuiWidgetStyle()
    {
        ImGuiStyle &style = ImGui::GetStyle();
        style.ScrollbarRounding = 0.0f;
    }

    void ImGuiLayer::setDarkThemeColors()
    {
        auto &style = ImGui::GetStyle();
        auto &colors = style.Colors;

        colors[ImGuiCol_WindowBg] = ImVec4{0.11f, 0.11f, 0.11f, 1.00f};
        colors[ImGuiCol_ChildBg] = ImVec4{0.11f, 0.11f, 0.11f, 1.00f};
        colors[ImGuiCol_PopupBg] = ImVec4{0.08f, 0.08f, 0.08f, 0.96f};
        colors[ImGuiCol_Border] = ImVec4{0.17f, 0.17f, 0.18f, 1.00f};

 
        colors[ImGuiCol_TitleBg] = ImVec4{0.07f, 0.07f, 0.07f, 1.00f};
        colors[ImGuiCol_TitleBgActive] = ImVec4{0.09f, 0.09f, 0.09f, 1.00f};
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.07f, 0.07f, 0.07f, 1.00f};


        colors[ImGuiCol_FrameBg] = ImVec4{0.16f, 0.16f, 0.17f, 1.00f};
        colors[ImGuiCol_FrameBgHovered] = ImVec4{0.22f, 0.22f, 0.23f, 1.00f};
        colors[ImGuiCol_FrameBgActive] = ImVec4{0.13f, 0.13f, 0.14f, 1.00f};


        const ImVec4 orangeMain = ImVec4{0.92f, 0.45f, 0.11f, 1.00f};
        const ImVec4 orangeHovered = ImVec4{1.00f, 0.55f, 0.20f, 1.00f}; 
        const ImVec4 orangeActive = ImVec4{0.80f, 0.38f, 0.08f, 1.00f}; 

        colors[ImGuiCol_Button] = ImVec4{0.20f, 0.20f, 0.21f, 1.00f}; 
        colors[ImGuiCol_ButtonHovered] = orangeMain;               
        colors[ImGuiCol_ButtonActive] = orangeActive;

        colors[ImGuiCol_Tab] = ImVec4{0.12f, 0.12f, 0.13f, 1.00f};
        colors[ImGuiCol_TabHovered] = orangeHovered;
        colors[ImGuiCol_TabActive] = orangeMain;
        colors[ImGuiCol_TabUnfocused] = ImVec4{0.12f, 0.12f, 0.13f, 1.00f};
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.18f, 0.18f, 0.19f, 1.00f};

        colors[ImGuiCol_CheckMark] = orangeMain;
        colors[ImGuiCol_SliderGrab] = orangeMain;
        colors[ImGuiCol_SliderGrabActive] = orangeActive;
        colors[ImGuiCol_Header] = ImVec4{0.35f, 0.20f, 0.08f, 0.50f};
        colors[ImGuiCol_HeaderHovered] = ImVec4{0.92f, 0.45f, 0.11f, 0.30f};
        colors[ImGuiCol_HeaderActive] = ImVec4{0.92f, 0.45f, 0.11f, 0.50f};

        colors[ImGuiCol_TextSelectedBg] = ImVec4{0.92f, 0.45f, 0.11f, 0.35f};

        colors[ImGuiCol_SeparatorHovered] = orangeMain;
        colors[ImGuiCol_SeparatorActive] = orangeActive;
        colors[ImGuiCol_ResizeGrip] = ImVec4{0.92f, 0.45f, 0.11f, 0.20f};
        colors[ImGuiCol_ResizeGripHovered] = orangeMain;
        colors[ImGuiCol_ResizeGripActive] = orangeActive;

        style.WindowRounding = 2.0f; 
        style.FrameRounding = 2.0f;
        style.PopupRounding = 2.0f;
        style.TabRounding = 2.0f;
        style.FrameBorderSize = 0.0f;
        style.WindowPadding = ImVec2(8, 8);
        style.ItemSpacing = ImVec2(8, 4);
    }
} // namespace Fermion
