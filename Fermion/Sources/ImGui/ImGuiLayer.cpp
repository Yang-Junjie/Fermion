#include "ImGuiLayer.hpp"
#include "Core/Log.hpp"
#include <imgui.h>
#ifdef FM_PLATFORM_DESKTOP
#include <backends/imgui_impl_glfw.h>
#endif
#include <backends/imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <filesystem>

// TODO:该类直接依赖于opengl和glfw，记得抽象出来以支持跨平台
namespace Fermion {
    ImGuiLayer::ImGuiLayer(void *nativeWindow) : Layer("ImGuiLayer"),
                                                 m_window(static_cast<GLFWwindow *>(nativeWindow)) {
    }

    void ImGuiLayer::onAttach() {
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

        if (std::filesystem::exists(fontRegularPath)) {
            fontRegular = io.Fonts->AddFontFromFileTTF(fontRegularPath, 18.0f);
        } else {
            Log::Warn(std::format("Font file not found: {}", fontRegularPath));
        }

        if (std::filesystem::exists(fontBoldPath)) {
            fontBold = io.Fonts->AddFontFromFileTTF(fontBoldPath, 18.0f);
        } else {
            Log::Warn(std::format("Font file not found: {}", fontRegularPath));
        }

        if (!fontRegular) {
            Log::Warn("Using default imgui font!");
            fontRegular = io.Fonts->AddFontDefault();
        }

        io.FontDefault = fontRegular;

        if (!fontBold)
            fontBold = io.Fonts->AddFontDefault();

        ImGui::StyleColorsDark();
        setDarkThemeColors();

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 430");
    }

    void ImGuiLayer::onDetach() {
        FM_PROFILE_FUNCTION();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::onEvent(IEvent &e) {
        if (m_blockEvents) {
            ImGuiIO &io = ImGui::GetIO();
            e.handled |= e.isInCategory(EventCategory::EventCategoryMouse) & io.WantCaptureMouse;
            e.handled |= e.isInCategory(EventCategory::EventCategoryKeyboard) & io.WantCaptureKeyboard;
        }
    }

    void ImGuiLayer::onImGuiRender() {
    }

    void ImGuiLayer::begin() {
        FM_PROFILE_FUNCTION();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void ImGuiLayer::end() {
        FM_PROFILE_FUNCTION();
        // 渲染
        ImGuiIO &io = ImGui::GetIO();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImGuiLayer::setDarkThemeColors() {
        ImGuiStyle &style = ImGui::GetStyle();
        auto &colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{0.145f, 0.145f, 0.149f, 1.0f};

        // Headers
        colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
        colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
        colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    }
} // namespace Fermion
