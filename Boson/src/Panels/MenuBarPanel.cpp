#include "MenuBarPanel.hpp"
#include "fmpch.hpp"
#include "../BosonLayer.hpp"

#include "Project/ProjectSerializer.hpp"
#include "ImGui/BosonUI.hpp"
#include "Core/Application.hpp"

namespace Fermion
{
    MenuBarPanel::MenuBarPanel()
    {
        m_WindowCloseIcon = Texture2D::create("../Boson/Resources/Icons/Close.png");
        m_WindowMaximizeIcon = Texture2D::create("../Boson/Resources/Icons/Maximize.png");
        m_WindowRestoreIcon = Texture2D::create("../Boson/Resources/Icons/Restore.png");
        m_WindowMinimizeIcon = Texture2D::create("../Boson/Resources/Icons/Minimize.png");
    }

    void MenuBarPanel::OnImGuiRender()
    {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, m_MenuBarHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("##CustomMenuBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);

        ImVec2 barPos = ImGui::GetCursorScreenPos();
        ImVec2 barSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
        // HandleWindowDrag(barPos, barSize);

        float leftX = barPos.x + 10.0f;
        float rightX = barPos.x + ImGui::GetWindowSize().x;
        float y = barPos.y;
        float itemWidth = 60.0f;
        float itemHeight = ImGui::GetWindowHeight();

        DrawMenuItem("File", "FilePopup", leftX, y, itemWidth, itemHeight);
        DrawMenuItem("Create", "CreatePopup", leftX, y, itemWidth, itemHeight);
        // DrawMenuItem("Build", "BuildPopup", leftX, y, itemWidth, itemHeight);
        DrawMenuItem("Help", "HelpPopup", leftX, y, itemWidth, itemHeight);

        // if (DrawWindowButton("##CloseWindow", m_WindowCloseIcon, rightX, y, itemHeight, IM_COL32(180, 60, 60, 255)))
        // {
        //     Application::get().close();
        // }
        // if (DrawWindowButton("##MaximizeWindow", m_IsWindowMaximized ? m_WindowRestoreIcon : m_WindowMaximizeIcon, rightX, y, itemHeight, IM_COL32(100, 100, 100, 255)))
        // {
        //     if (m_IsWindowMaximized)
        //     {
        //         Application::get().getWindow().setRestored();
        //     }
        //     else
        //     {
        //         Application::get().getWindow().setMaximized();
        //     }
        //     m_IsWindowMaximized = !m_IsWindowMaximized;
        // }
        // if (DrawWindowButton("##MinimizeWindow", m_WindowMinimizeIcon, rightX, y, itemHeight, IM_COL32(100, 100, 100, 255)))
        // {
        //     Application::get().getWindow().setMinimized();
        // }

        if (ui::BeginPopup("FilePopup"))
        {
            if (m_BosonLayer && ImGui::MenuItem("New Scene", "Ctrl+N"))
                m_BosonLayer->newScene();
            if (m_BosonLayer && ImGui::MenuItem("Open Scene...", "Ctrl+Shift+O"))
                m_BosonLayer->openScene();
            if (m_BosonLayer && ImGui::MenuItem("Save Scene", "Ctrl+S"))
                m_BosonLayer->saveScene();
            if (m_BosonLayer && ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
                m_BosonLayer->saveSceneAs();
            ImGui::Separator();
            if (m_BosonLayer && ImGui::MenuItem("new project"))
                m_BosonLayer->newProject();
            if (m_BosonLayer && ImGui::MenuItem("open project"))
                m_BosonLayer->openProject();
            if (m_BosonLayer && ImGui::MenuItem("save project"))
                m_BosonLayer->saveProject();
            if (ImGui::MenuItem("Exit"))
            {
                if (m_BosonLayer)
                    m_BosonLayer->saveProject();
                Application::get().close();
            }
            ui::EndPopup();
        }
        if (ui::BeginPopup("CreatePopup"))
        {
            const bool isOpenProject = Project::getActive() != nullptr;
            ImGui::BeginDisabled(!isOpenProject);
            if (ImGui::MenuItem("Material Editor"))
            {
                if (isOpenProject && m_BosonLayer)
                    m_BosonLayer->openMaterialEditorPanel();
            }
            ImGui::EndDisabled();
            ui::EndPopup();
        }

        // if (ui::BeginPopup("BuildPopup"))
        // {
        //     if (ImGui::MenuItem("Build Project"))
        //     {
        //         if (auto project = Project::getActive())
        //         {
        //             buildProject(project);
        //         }
        //         else
        //         {
        //             Log::Error("No active project found!");
        //         }
        //     }
        //     ui::EndPopup();
        // }

        if (ui::BeginPopup("HelpPopup"))
        {
            if (m_BosonLayer && ImGui::MenuItem("about"))
                m_BosonLayer->openAboutWindow();
            ui::EndPopup();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void MenuBarPanel::buildProject(const std::shared_ptr<Project> &project)
    {
        FERMION_ASSERT(project != nullptr, "No active project to build!");

        const auto projectDir = project->getProjectPath().parent_path();
        std::error_code ec;

        const std::filesystem::path shaderSrcDir = "../Boson/Resources/shaders";
        const std::filesystem::path shaderDstDir = projectDir / "Resources" / "shaders";

        std::filesystem::create_directories(shaderDstDir, ec);
        if (ec)
        {
            Log::Error(std::format("Failed to create shader output dir: {} ({})", shaderDstDir.string(), ec.message()));
            return;
        }

        if (!std::filesystem::exists(shaderSrcDir))
        {
            Log::Error(std::format("Shader source dir not found: {}", shaderSrcDir.string()));
            return;
        }

        for (const auto &entry : std::filesystem::directory_iterator(shaderSrcDir))
        {
            if (!entry.is_regular_file())
                continue;

            const auto &srcPath = entry.path();
            const auto dstPath = shaderDstDir / srcPath.filename();

            std::filesystem::copy_file(srcPath, dstPath, std::filesystem::copy_options::overwrite_existing, ec);
            if (ec)
            {
                Log::Error(std::format("Failed to copy shader {} -> {}: {}", srcPath.string(), dstPath.string(), ec.message()));
                ec.clear();
            }
        }
    }
    void MenuBarPanel::DrawMenuItem(const char *label,
                                    const char *popupName,
                                    float &leftX,
                                    float y,
                                    float itemWidth,
                                    float itemHeight) const
    {
        ImVec2 itemMin(leftX, y);
        ImVec2 itemMax(leftX + itemWidth, y + itemHeight);

        ImU32 bgColor = ImGui::IsMouseHoveringRect(itemMin, itemMax) ? IM_COL32(100, 100, 100, 255) : IM_COL32(0, 0, 0, 0);
        ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, bgColor);

        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImVec2 textPos(itemMin.x + (itemWidth - textSize.x) * 0.5f, itemMin.y + (itemHeight - textSize.y) * 0.5f);
        ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), label);

        ImGui::SetCursorScreenPos(itemMin);
        ImGui::InvisibleButton(label, ImVec2(itemWidth, itemHeight));

        if (ImGui::IsItemClicked())
            ImGui::OpenPopup(popupName);

        leftX += itemWidth + 5.0f;
    }

    bool MenuBarPanel::DrawWindowButton(const char *id,
                                        const std::shared_ptr<Texture2D> &icon,
                                        float &rightX,
                                        float y,
                                        float itemHeight,
                                        ImU32 hoverColor) const
    {
        float iconHeight = itemHeight * 0.6f;
        float scale = ((float)icon->getWidth() / icon->getHeight());
        float iconWidth = iconHeight;
        if (scale == 1.0f)
        {
            iconWidth = iconHeight;
        }
        else
        {
            iconHeight = itemHeight * 0.1f;
            iconWidth = iconHeight * scale;
        }

        float paddingX = (itemHeight - iconWidth) * 0.5f;
        float paddingY = (itemHeight - iconHeight) * 0.5f;

        ImVec2 itemMin(rightX - itemHeight, y);
        ImVec2 itemMax(rightX, y + itemHeight);

        bool hovered = ImGui::IsMouseHoveringRect(itemMin, itemMax);
        ImU32 bgColor = hovered ? hoverColor : IM_COL32(0, 0, 0, 0);
        ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, bgColor);

        ImVec2 iconMin(itemMin.x + paddingX, itemMin.y + paddingY);
        ImVec2 iconMax(iconMin.x + iconWidth, iconMin.y + iconHeight);

        ImGui::GetWindowDrawList()->AddImage((ImTextureID)icon->getRendererID(), iconMin, iconMax);

        ImGui::SetCursorScreenPos(itemMin);
        ImGui::InvisibleButton(id, ImVec2(itemHeight, itemHeight));
        bool clicked = ImGui::IsItemClicked();

        rightX -= itemHeight + 4.0f;
        return clicked;
    }

    void MenuBarPanel::HandleWindowDrag(const ImVec2 &barPos, const ImVec2 &barSize)
    {
        ImGui::SetCursorScreenPos(barPos);
        ImGui::SetNextItemAllowOverlap();
        ImGui::InvisibleButton("##MenuBarDrag", barSize);

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            ImGuiIO &io = ImGui::GetIO();
            if (!m_Dragging)
            {
                m_Dragging = true;
                m_DragStartMouse = io.MousePos;
                int winX, winY;
                Application::get().getWindow().getWindowPos(&winX, &winY);
                m_DragStartWindow = ImVec2((float)winX, (float)winY);
            }

            float newPosX = m_DragStartWindow.x + (io.MousePos.x - m_DragStartMouse.x);
            float newPosY = m_DragStartWindow.y + (io.MousePos.y - m_DragStartMouse.y);
            Application::get().getWindow().setWindowPos((int)newPosX, (int)newPosY);
        }
        else
        {
            m_Dragging = false;
        }

        ImGui::SetCursorScreenPos(barPos);
    }
} // namespace Fermion
