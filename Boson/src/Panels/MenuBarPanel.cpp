#include "MenuBarPanel.hpp"
#include "fmpch.hpp"
#include "Project/Project.hpp"
#include "Project/ProjectSerializer.hpp"
#include "Core/Application.hpp"

namespace Fermion
{
    void MenuBarPanel::OnImGuiRender()
    {

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, m_MenuBarHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("##CustomMenuBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoDocking);

        ImVec2 barPos = ImGui::GetCursorScreenPos();
        ImVec2 barSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);

        float x = barPos.x + 10.0f;
        float y = barPos.y;
        float itemWidth = 60.0f;
        float itemHeight = ImGui::GetWindowHeight();

        auto drawItem = [&](const char *label, const char *popupName)
        {
            ImVec2 itemMin(x, y);
            ImVec2 itemMax(x + itemWidth, y + itemHeight);

            ImU32 bgColor = ImGui::IsMouseHoveringRect(itemMin, itemMax) ? IM_COL32(100, 100, 100, 255) : IM_COL32(0, 0, 0, 0);
            ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, bgColor);

            ImVec2 textSize = ImGui::CalcTextSize(label);
            ImVec2 textPos(itemMin.x + (itemWidth - textSize.x) * 0.5f, itemMin.y + (itemHeight - textSize.y) * 0.5f);
            ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), label);

            ImGui::SetCursorScreenPos(itemMin);
            ImGui::InvisibleButton(label, ImVec2(itemWidth, itemHeight));

            if (ImGui::IsItemClicked())
                ImGui::OpenPopup(popupName);

            x += itemWidth + 5;
        };

        drawItem("File", "FilePopup");
        drawItem("Build", "BuildPopup");
        drawItem("Help", "HelpPopup");

        auto invoke = [](const std::function<void()> &fn)
        {
            if (fn)
                fn();
        };

        if (ImGui::BeginPopup("FilePopup"))
        {
            if (ImGui::MenuItem("New Scene", "Ctrl+N"))
                invoke(m_Callbacks.NewScene);
            if (ImGui::MenuItem("Open Scene...", "Ctrl+Shift+O"))
                invoke(m_Callbacks.OpenScene);
            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
                invoke(m_Callbacks.SaveScene);
            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
                invoke(m_Callbacks.SaveSceneAs);
            ImGui::Separator();
            if (ImGui::MenuItem("new project"))
                invoke(m_Callbacks.NewProject);
            if (ImGui::MenuItem("open project"))
                invoke(m_Callbacks.OpenProject);
            if (ImGui::MenuItem("save project"))
                invoke(m_Callbacks.SaveProject);
            if (ImGui::MenuItem("Exit"))
            {
                invoke(m_Callbacks.ExitApplication);
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("BuildPopup"))
        {
            if (ImGui::MenuItem("Build Project"))
            {
                auto project = Project::getActive();
                FERMION_ASSERT(project != nullptr, "No active project to build!");

                const auto projectFile = project->getProjectPath();
                const auto projectDir = projectFile.parent_path();

                {
                    ProjectSerializer serializer(project);
                    const auto runtimeFile = projectDir / "Project.fdat";
                    serializer.sertializeRuntime(runtimeFile);
                }

                {
                    std::filesystem::path shaderSrcDir = "../Boson/Resources/shaders";
                    std::filesystem::path shaderDstDir = projectDir / "Resources" / "shaders";

                    std::error_code ec;

                    std::filesystem::create_directories(shaderDstDir, ec);
                    if (ec)
                    {
                        Log::Error(std::format("Failed to create shader output dir: {} ({})",
                                               shaderDstDir.string(), ec.message()));
                    }
                    else
                    {
                        if (!std::filesystem::exists(shaderSrcDir) ||
                            !std::filesystem::is_directory(shaderSrcDir))
                        {
                            Log::Error(std::format("Shader source dir not found: {}",
                                                   shaderSrcDir.string()));
                        }
                        else
                        {
                            for (const auto &entry : std::filesystem::directory_iterator(shaderSrcDir))
                            {
                                if (!entry.is_regular_file())
                                    continue;

                                const auto &srcPath = entry.path();
                                auto dstPath = shaderDstDir / srcPath.filename();

                                std::filesystem::copy_file(
                                    srcPath,
                                    dstPath,
                                    std::filesystem::copy_options::overwrite_existing,
                                    ec);

                                if (ec)
                                {
                                    Log::Error(std::format("Failed to copy shader {} -> {}: {}",
                                                           srcPath.string(),
                                                           dstPath.string(),
                                                           ec.message()));
                                    ec.clear();
                                }
                            }
                        }
                    }
                }
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("HelpPopup"))
        {
            if (ImGui::MenuItem("about"))
                invoke(m_Callbacks.ShowAbout);
            ImGui::EndPopup();
        }
        ImGui::SetCursorScreenPos(barPos);
        ImGui::InvisibleButton("##MenuBarDrag", barSize);
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            if (!m_Dragging)
            {
                m_Dragging = true;
                m_DragStartMouse = ImGui::GetIO().MousePos;
                int winX, winY;
                Application::get().getWindow().getWindowPos(&winX, &winY);
                m_DragStartWindow = ImVec2((float)winX, (float)winY);
            }

            float newPosX = m_DragStartWindow.x + (ImGui::GetIO().MousePos.x - m_DragStartMouse.x);
            float newPosY = m_DragStartWindow.y + (ImGui::GetIO().MousePos.y - m_DragStartMouse.y);
            Application::get().getWindow().setWindowPos((int)newPosX, (int)newPosY);
        }
        else
        {
            m_Dragging = false;
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

}
