#include "Fermion.hpp"
#include "BosonLayer.hpp"
#include "Scene/EntityManager.hpp"
#include "Renderer/Font/Font.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Script/ScriptManager.hpp"

#include "ImGui/ModalDialog.hpp"
#include "ImGui/ConsolePanel.hpp"

#include <imgui.h>
#include <ImGuizmo.h>

#include <utility>
#include <vector>

namespace Fermion
{
    static std::shared_ptr<Font> s_Font; // TODO: Temporary
    BosonLayer::BosonLayer(const std::string &name, std::filesystem::path initialProjectPath)
        : Layer(name),
          m_pendingProjectPath(std::move(initialProjectPath))
    {
        s_Font = Font::getDefault();
        m_contentBrowserPanel.setProjectOpenCallback([this](const std::filesystem::path &path)
                                                     { openProject(path); });
    }

    void BosonLayer::onAttach()
    {
        FM_PROFILE_FUNCTION();

        // Initialize icons
        m_iconPlay = Texture2D::create("../Boson/Resources/Icons/PlayButton.png");
        m_iconStop = Texture2D::create("../Boson/Resources/Icons/StopButton.png");
        m_iconPause = Texture2D::create("../Boson/Resources/Icons/PauseButton.png");
        m_iconSimulate = Texture2D::create("../Boson/Resources/Icons/SimulateButton.png");
        m_iconStep = Texture2D::create("../Boson/Resources/Icons/StepButton.png");

        // Initialize frame buffer to render to viewport
        FramebufferSpecification fbSpec;
        fbSpec.width = 1280;
        fbSpec.height = 720;
        fbSpec.attachments = {
            FramebufferTextureFormat::RGBA8,
            FramebufferTextureFormat::RED_INTEGER,
            FramebufferTextureFormat::DEPTH24STENCIL8};
        m_framebuffer = Framebuffer::create(fbSpec);
        // Initialize render about something
        m_activeScene = std::make_shared<Scene>();
        m_viewportRenderer = std::make_shared<SceneRenderer>();
        m_editorScene = m_activeScene;
        const float aspectRatio = static_cast<float>(fbSpec.width) / static_cast<float>(fbSpec.height);
        m_editorCamera = EditorCamera(45.0f, aspectRatio, 0.1f, 1000.0f);

        m_sceneHierarchyPanel.setContext(m_activeScene);
        m_viewportRenderer->setScene(m_activeScene);

        // Initialize script manager scene renderer
        ScriptManager::get()->setSceneRenderer(m_viewportRenderer);

        // Initialize menu bar
        m_menuBarPanel.SetBosonLayer(this);

        // Initialize ViewportPanel callbacks
        m_viewportPanel.setCallbacks({
            .onPlay = [this]() { onScenePlay(); },
            .onSimulate = [this]() { onSceneSimulate(); },
            .onStop = [this]() { onSceneStop(); },
            .onOpenScene = [this](const std::filesystem::path &p) { openScene(p); },
            .onOpenProject = [this](const std::filesystem::path &p) { openProject(p); },
            .onSelectEntity = [this](Entity e) { m_sceneHierarchyPanel.setSelectedEntity(e); },
        });

        m_isInitialized = true;
        if (!m_pendingProjectPath.empty())
        {
            const auto pendingPath = m_pendingProjectPath;
            m_pendingProjectPath.clear();
            openProject(pendingPath);
        }
    }

    void BosonLayer::onDetach()
    {
        FM_PROFILE_FUNCTION();
    }

    void BosonLayer::onUpdate(Timestep dt)
    {
        FM_PROFILE_FUNCTION();

        // Resize
        const auto &viewportSize = m_viewportPanel.getViewportSize();
        const auto viewportWidth = static_cast<uint32_t>(viewportSize.x);
        const auto viewportHeight = static_cast<uint32_t>(viewportSize.y);
        if (viewportWidth > 0 && viewportHeight > 0)
        {
            const FramebufferSpecification spec = m_framebuffer->getSpecification();

            const bool needResize = spec.width != viewportWidth || spec.height != viewportHeight;

            if (needResize)
            {
                m_framebuffer->resize(viewportWidth, viewportHeight);
                m_editorCamera.setViewportSize(static_cast<float>(viewportWidth), static_cast<float>(viewportHeight));
                m_activeScene->onViewportResize(viewportWidth, viewportHeight);
            }
        }

        // Render
        Renderer2DCompat::resetStatistics();
        if (m_viewportRenderer)
            m_viewportRenderer->resetStatistics();
        m_framebuffer->bind();
        Renderer::getRendererAPI().setClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        Renderer::getRendererAPI().clear();
        m_framebuffer->clearAttachment(1, -1);

        // Tell SceneRenderer which framebuffer to restore after shadow pass
        m_viewportRenderer->setTargetFramebuffer(m_framebuffer);

        if (m_viewportRenderer)
        {
            std::vector<int> outlineIDs;
            Entity selectedEntity = m_sceneHierarchyPanel.getSelectedEntity();
            if (selectedEntity && selectedEntity.hasComponent<MeshComponent>())
                outlineIDs.push_back((int)selectedEntity);
            m_viewportRenderer->setOutlineIDs(outlineIDs);
        }

        // Update the active scene based on the current scene state
        if (m_sceneState == SceneState::Play)
        {
            m_activeScene->onUpdateRuntime(m_viewportRenderer, dt, m_showRenderEntities);
            m_sceneHierarchyPanel.setSelectedEntity({});
        }
        else if (m_sceneState == SceneState::Simulate)
        {
            m_editorCamera.setCanEnterFpsMode(m_viewportPanel.isViewportHovered());
            m_editorCamera.onUpdate(dt);
            m_activeScene->onUpdateSimulation(m_viewportRenderer, dt, m_editorCamera, m_showRenderEntities);
        }
        else
        {
            m_editorCamera.setCanEnterFpsMode(m_viewportPanel.isViewportHovered());
            m_editorCamera.onUpdate(dt);
            m_activeScene->onUpdateEditor(m_viewportRenderer, dt, m_editorCamera, m_showRenderEntities);
        }
        // Mouse picking
        ViewportPanel::Context vpCtx{
            .framebuffer = m_framebuffer,
            .activeScene = m_activeScene,
            .viewportRenderer = m_viewportRenderer,
            .editorCamera = &m_editorCamera,
            .sceneState = static_cast<int>(m_sceneState),
            .selectedEntity = m_sceneHierarchyPanel.getSelectedEntity(),
        };
        m_viewportPanel.updateMousePicking(vpCtx);

        // Overlay render
        OverlayRenderPanel::Context overlayCtx{
            .activeScene = m_activeScene,
            .viewportRenderer = m_viewportRenderer,
            .editorCamera = &m_editorCamera,
            .sceneState = static_cast<int>(m_sceneState),
            .showPhysicsDebug = m_showPhysicsDebug,
            .selectedEntity = m_sceneHierarchyPanel.getSelectedEntity(),
        };
        m_overlayRenderPanel.render(overlayCtx);

        m_framebuffer->unbind();
    }
    void BosonLayer::onImGuiRender()
    {
        FM_PROFILE_FUNCTION();

        static bool dockspaceOpen = true;
        if (dockspaceOpen)
        {
            static bool opt_fullscreen = true;
            static bool opt_padding = false;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
            {
                const ImGuiViewport *viewport = ImGui::GetMainViewport();
                ImVec2 dockspacePos = viewport->WorkPos;
                dockspacePos.y += m_menuBarPanel.GetMenuBarHeight();
                ImVec2 dockspaceSize = viewport->WorkSize;
                dockspaceSize.y -= m_menuBarPanel.GetMenuBarHeight();
                ImGui::SetNextWindowPos(dockspacePos);
                ImGui::SetNextWindowSize(dockspaceSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }
            else
            {
                dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
            }
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            if (!opt_padding)
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            // DockSpace
            {
                ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
                if (!opt_padding)
                    ImGui::PopStyleVar();

                if (opt_fullscreen)
                    ImGui::PopStyleVar(2);

                ImGuiIO &io = ImGui::GetIO();
                ImGuiStyle &style = ImGui::GetStyle();
                float minWinSizeX = style.WindowMinSize.x;
                style.WindowMinSize.x = 350.0f;
                if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
                {
                    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                }
                style.WindowMinSize.x = minWinSizeX;
                ImGui::End();
            }

            m_sceneHierarchyPanel.onImGuiRender();
            m_contentBrowserPanel.onImGuiRender();
            m_assetManagerPanel.onImGuiRender();
            m_menuBarPanel.OnImGuiRender();
            m_materialEditorPanel.onImGuiRender();
            ConsolePanel::get().onImGuiRender();

            onHelpPanel();

            // Settings panel
            SettingsPanel::Context settingsCtx{
                .viewportRenderer = m_viewportRenderer,
                .editorCamera = &m_editorCamera,
                .hoveredEntity = m_viewportPanel.getHoveredEntity(),
                .viewportFocused = m_viewportPanel.isViewportFocused(),
                .showPhysicsDebug = &m_showPhysicsDebug,
                .showRenderEntities = &m_showRenderEntities,
            };
            m_settingsPanel.onImGuiRender(settingsCtx);

            // Viewport panel
            ViewportPanel::Context vpCtx{
                .framebuffer = m_framebuffer,
                .activeScene = m_activeScene,
                .viewportRenderer = m_viewportRenderer,
                .editorCamera = &m_editorCamera,
                .sceneState = static_cast<int>(m_sceneState),
                .selectedEntity = m_sceneHierarchyPanel.getSelectedEntity(),
                .iconPlay = m_iconPlay.get(),
                .iconStop = m_iconStop.get(),
                .iconPause = m_iconPause.get(),
                .iconSimulate = m_iconSimulate.get(),
                .iconStep = m_iconStep.get(),
            };
            m_viewportPanel.onImGuiRender(vpCtx);
        }
    }
    void BosonLayer::onEvent(IEvent &event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowResizeEvent>([](WindowResizeEvent &e)
                                               { return false; });
        dispatcher.dispatch<KeyPressedEvent>([this](KeyPressedEvent &e)
                                             { return this->onKeyPressedEvent(e); });
        dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent &e)
                                                     { return this->onMouseButtonPressedEvent(e); });

        m_editorCamera.onEvent(event);
    }

    bool BosonLayer::onKeyPressedEvent(KeyPressedEvent &e)
    {
        if (e.isRepeat())
        {
            return false;
        }

        const bool control = Input::isKeyPressed(KeyCode::LeftControl) || Input::isKeyPressed(KeyCode::RightControl);
        const bool shift = Input::isKeyPressed(KeyCode::LeftShift) || Input::isKeyPressed(KeyCode::RightShift);

        switch (e.getKeyCode())
        {
        case KeyCode::Escape:
            if (m_sceneHierarchyPanel.isEntityPickingActive())
            {
                m_sceneHierarchyPanel.cancelEntityPicking();
                return true;
            }
            break;
        case KeyCode::S:
            if (control && shift)
            {
                saveSceneAs();
                return true;
            }
            else if (control)
            {
                saveScene();
                return true;
            }
            break;
        case KeyCode::N:
            if (control)
            {
                newScene();
                return true;
            }
            break;
        case KeyCode::O:
            if (control)
            {
                openScene();
                return true;
            }
            break;
        case KeyCode::D:
            if (control)
            {
                if (const Entity selected = m_sceneHierarchyPanel.getSelectedEntity(); selected)
                {
                    m_activeScene->getEntityManager().destroyEntity(selected);
                    m_sceneHierarchyPanel.setSelectedEntity({});
                }
            }
            break;
        case KeyCode::Q:
            m_viewportPanel.setGizmoType(-1);
            break;
        case KeyCode::W:
            m_viewportPanel.setGizmoType(ImGuizmo::OPERATION::TRANSLATE);
            break;
        case KeyCode::E:
            m_viewportPanel.setGizmoType(ImGuizmo::OPERATION::ROTATE);
            break;
        case KeyCode::R:
            if (control)
                onDuplicateEntity();
            else
                m_viewportPanel.setGizmoType(ImGuizmo::OPERATION::SCALE);
            break;
        default:;
        }

        return false;
    }

    bool BosonLayer::onMouseButtonPressedEvent(const MouseButtonPressedEvent &e)
    {
        if (e.getMouseButton() == MouseCode::Left)
        {
            Entity hoveredEntity = m_viewportPanel.getHoveredEntity();

            // If entity picking mode is active (e.g. RevoluteJoint2D connected body selection)
            if (m_sceneHierarchyPanel.isEntityPickingActive())
            {
                if (m_viewportPanel.isViewportHovered() && hoveredEntity && hoveredEntity.isValid())
                {
                    m_sceneHierarchyPanel.deliverPickedEntity(hoveredEntity);
                }
                else
                {
                    m_sceneHierarchyPanel.cancelEntityPicking();
                }
                return true;
            }

            if (m_viewportPanel.isViewportHovered() && !ImGuizmo::IsOver())
                m_sceneHierarchyPanel.setSelectedEntity(
                    hoveredEntity && hoveredEntity.isValid()
                        ? hoveredEntity
                        : Entity());
        }
        return false;
    }

    void BosonLayer::onHelpPanel()
    {
        if (m_showNewSceneDialog)
        {
            ui::ModalDialog::Show(
                {.Title = "Create New Scene",
                 .ShowConfirm = false,
                 .ShowCancel = false,
                 .ShowClose = true,
                 .ContentFunc = [&]()
                 {
                    ImGui::Text("Please select scene type:");
                    ImGui::Spacing();

                    if (ImGui::Button("2D Scene", ImVec2(120, 0)))
                    {
                        createScene2D();
                        m_showNewSceneDialog = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("3D Scene", ImVec2(120, 0)))
                    {
                        createScene3D();
                        m_showNewSceneDialog = false;
                        ImGui::CloseCurrentPopup();
                    } }});
        }

        if (m_isAboutWindowOpen)
        {
            ui::ModalDialog::Show(
                {.Title = "About Fermion",
                 .ShowConfirm = true,
                 .ShowCancel = false,
                 .ShowClose = false,
                 .ConfirmText = "OK",
                 .CancelText = "",
                 .ContentFunc = [&]()
                 {
                    ImGui::SeparatorText("About Fermion");
                    ImGui::Text("Fermion Engine v0.1");
                    ImGui::Text("Copyright (c) 2025-2026, Beisent");
                    ImGui::Text("This is free and open-source software under the MIT License.");
                    ImGui::SeparatorText("From");
                    ImGui::Text("This engine was written based on TheCherno's game engine tutorial series.");
                    ImGui::Text("Hazel Engine: ");
                    ImGui::TextLinkOpenURL("https: // github.com/TheCherno/Hazel");
                    ImGui::Text("TheCherno's game engine tutorial series:");
                    ImGui::TextLinkOpenURL(
                        "https://www.youtube.com/watch?v=JxIZbV_XjAs&list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT"); },
                 .ConfirmFunc = [&]()
                 { m_isAboutWindowOpen = false; }});
        }

        ui::ModalDialog::OnImGuiRender();
    }

    void BosonLayer::openAboutWindow()
    {
        m_isAboutWindowOpen = true;
    }

    void BosonLayer::openMaterialEditorPanel()
    {
        m_materialEditorPanel.clearData();
        m_materialEditorPanel.setPanelOpenState(true);
    }

    void BosonLayer::onDuplicateEntity()
    {
        if (m_sceneState != SceneState::Edit)
            return;

        Entity selectedEntity = m_sceneHierarchyPanel.getSelectedEntity();
        if (selectedEntity)
        {
            const Entity newEntity = m_editorScene->getEntityManager().duplicateEntity(selectedEntity);
            m_sceneHierarchyPanel.setSelectedEntity(newEntity);
        }
    }
} // namespace Fermion
