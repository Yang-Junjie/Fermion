#include "Fermion.hpp"
#include "BosonLayer.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Scene/EntityManager.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Renderer/Font/Font.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Asset/SceneAsset.hpp"
#include "Script/ScriptManager.hpp"

#include "ImGui/ModalDialog.hpp"
#include "ImGui/ConsolePanel.hpp"
#include "ImGui/BosonUI.hpp"

#include "Math/Math.hpp"

#include <format>
#include <imgui.h>
#include <ImGuizmo.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <utility>
#include <vector>

namespace
{
    bool isProjectDescriptor(const std::filesystem::path &path)
    {
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c)
                       { return static_cast<char>(std::tolower(c)); });
        return ext == ".fmproj";
    }

    std::filesystem::path findProjectFileInDirectory(const std::filesystem::path &directory)
    {
        std::error_code ec;
        for (std::filesystem::directory_iterator it(directory, ec), end; it != end; it.increment(ec))
        {
            if (ec)
                break;

            std::error_code entryEc;
            if (!it->is_regular_file(entryEc) || entryEc)
                continue;

            if (isProjectDescriptor(it->path()))
                return it->path();
        }
        return {};
    }
} // namespace

namespace Fermion
{
    static std::shared_ptr<Font> s_Font; // TODO:  Temporary
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
        const auto viewportWidth = static_cast<uint32_t>(m_viewportSize.x);
        const auto viewportHeight = static_cast<uint32_t>(m_viewportSize.y);

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
        RenderCommand::setClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::clear();
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
            m_editorCamera.setCanEnterFpsMode(m_viewportHovered);
            m_editorCamera.onUpdate(dt);
            m_activeScene->onUpdateSimulation(m_viewportRenderer, dt, m_editorCamera, m_showRenderEntities);
        }
        else
        {
            m_editorCamera.setCanEnterFpsMode(m_viewportHovered);
            m_editorCamera.onUpdate(dt);
            m_activeScene->onUpdateEditor(m_viewportRenderer, dt, m_editorCamera, m_showRenderEntities);
        }

        updateMousePicking();

        onOverlayRender();

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
            // ImGui::ShowDemoWindow();

            onHelpPanel();
            onSettingsPanel();
            onViewportPanel();
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
        // m_cameraController.onEvent(event);

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
            m_gizmoType = -1;
            break;
        case KeyCode::W:
            m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
            break;
        case KeyCode::E:
            m_gizmoType = ImGuizmo::OPERATION::ROTATE;
            break;
        case KeyCode::R:
            if (control)
                onDuplicateEntity();
            else
                m_gizmoType = ImGuizmo::OPERATION::SCALE;
            break;
        default:;
        }

        return false;
    }

    bool BosonLayer::onMouseButtonPressedEvent(const MouseButtonPressedEvent &e)
    {
        if (e.getMouseButton() == MouseCode::Left)
        {
            if (m_viewportHovered && !ImGuizmo::IsOver())
                m_sceneHierarchyPanel.setSelectedEntity(
                    m_hoveredEntity && m_hoveredEntity.isValid()
                        ? m_hoveredEntity
                        : Entity());
        }
        return false;
    }

    void BosonLayer::onHelpPanel()
    {
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

    void BosonLayer::onSettingsPanel()
    {

        {
            ImGui::Begin("Renderer Info");
            ImGui::SeparatorText("Device Info");
            DeviceInfo info = Application::get().getWindow().getDeviceInfo();
            ImGui::Text("vendor: %s", info.vendor.c_str());
            ImGui::Text("renderer: %s", info.renderer.c_str());
            ImGui::Text("version: %s", info.version.c_str());
            float frameTimeMs = Application::get().getTimestep().getMilliseconds();
            static float avgFrameTimeMs = frameTimeMs;
            const float smoothing = 0.01f; // 0.0~1.0 越小越平滑

            avgFrameTimeMs = avgFrameTimeMs * (1.0f - smoothing) + frameTimeMs * smoothing;

            ImGui::Text("Frame time: %.3f ms", avgFrameTimeMs);
            ImGui::Text("FPS: %.1f FPS", 1000.0f / avgFrameTimeMs);

            ImGui::SeparatorText("Renderer Statistics");
            const SceneRenderer::RenderStatistics stats = m_viewportRenderer
                                                              ? m_viewportRenderer->getStatistics()
                                                              : SceneRenderer::RenderStatistics{};

            ImGui::Text("Draw Calls (Total): %u", stats.getTotalDrawCalls());
            ImGui::SeparatorText("Renderer2D");
            ImGui::Text("Draw Calls: %u", stats.renderer2D.drawCalls);
            ImGui::Text("Quads: %u", stats.renderer2D.quadCount);
            ImGui::Text("Lines: %u", stats.renderer2D.lineCount);
            ImGui::Text("Circles: %u", stats.renderer2D.circleCount);
            ImGui::Text("Vertices: %u", stats.getTotalVertexCount());
            ImGui::Text("Indices: %u", stats.getTotalIndexCount());

            ImGui::SeparatorText("Renderer3D");
            ImGui::Text("Meshes: %u", stats.renderer3D.meshCount);
            ImGui::Text("Geometry Draw Calls: %u", stats.renderer3D.geometryDrawCalls);
            ImGui::Text("Shadow Draw Calls: %u", stats.renderer3D.shadowDrawCalls);
            ImGui::Text("Skybox Draw Calls: %u", stats.renderer3D.skyboxDrawCalls);
            ImGui::Text("IBL Draw Calls: %u", stats.renderer3D.iblDrawCalls);
            ImGui::Text("Draw Calls (3D Total): %u", stats.renderer3D.getTotalDrawCalls());
            ImGui::SeparatorText("Editor Camera Info");
            ImGui::Text("FPS speed: %.2f", m_editorCamera.getFPSSpeed());

            ImGui::End();
        }

        {
            ImGui::Begin("Environment Settings");
            auto &sceneInfo = m_viewportRenderer->getSceneInfo();
            ImGui::Button("Drag HDR to load");
            const char *hdrPath = nullptr;
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_HDR"))
                {
                    hdrPath = static_cast<const char *>(payload->Data);
                    if (hdrPath && hdrPath[0])
                    {
                        m_viewportRenderer->loadHDREnvironment(hdrPath);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::TextWrapped("HDR: %s",
                               hdrPath
                                   ? hdrPath
                                   : std::filesystem::path(m_viewportRenderer->getSceneInfo().defaultHdrPath).filename().string().c_str());
            ImGui::Checkbox("showSkybox", &m_viewportRenderer->getSceneInfo().showSkybox);
            ImGui::Checkbox("enable shadows", &m_viewportRenderer->getSceneInfo().enableShadows);
            ImGui::Checkbox("use IBL", &m_viewportRenderer->getSceneInfo().useIBL);
            ImGui::DragFloat("Ambient Intensity", &m_viewportRenderer->getSceneInfo().ambientIntensity, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Normal Map Strength", &m_viewportRenderer->getSceneInfo().normalMapStrength, 0.1f, 0.0f, 5.0f);
            ImGui::DragFloat("Toksvig Strength", &m_viewportRenderer->getSceneInfo().toksvigStrength, 0.05f, 0.0f, 4.0f);

            ImGui::SeparatorText("SSGI");
            if (ImGui::Checkbox("Enable SSGI", &sceneInfo.enableSSGI))
            {
                if (sceneInfo.enableSSGI)
                    sceneInfo.renderMode = SceneRenderer::RenderMode::DeferredHybrid;
            }
            if (sceneInfo.enableSSGI)
            {
                ImGui::Indent();
                ImGui::DragFloat("SSGI Intensity", &sceneInfo.ssgiIntensity, 0.05f, 0.0f, 10.0f);
                ImGui::DragFloat("SSGI Radius", &sceneInfo.ssgiRadius, 0.05f, 0.1f, 10.0f);
                ImGui::DragFloat("SSGI Bias", &sceneInfo.ssgiBias, 0.001f, 0.0f, 1.0f);
                ImGui::SliderInt("SSGI Samples", &sceneInfo.ssgiSampleCount, 1, 1024);
                ImGui::Unindent();
            }

            ImGui::SeparatorText("GTAO");
            if (ImGui::Checkbox("Enable GTAO", &sceneInfo.enableGTAO))
            {
                if (sceneInfo.enableGTAO)
                    sceneInfo.renderMode = SceneRenderer::RenderMode::DeferredHybrid;
            }
            if (sceneInfo.enableGTAO)
            {
                ImGui::Indent();
                ImGui::DragFloat("GTAO Intensity", &sceneInfo.gtaoIntensity, 0.05f, 0.0f, 4.0f);
                ImGui::DragFloat("GTAO Radius", &sceneInfo.gtaoRadius, 0.05f, 0.1f, 10.0f);
                ImGui::DragFloat("GTAO Bias", &sceneInfo.gtaoBias, 0.001f, 0.0f, 0.5f);
                ImGui::DragFloat("GTAO Power", &sceneInfo.gtaoPower, 0.05f, 0.1f, 4.0f);
                ImGui::SliderInt("GTAO Slices", &sceneInfo.gtaoSliceCount, 1, 12);
                ImGui::SliderInt("GTAO Steps", &sceneInfo.gtaoStepCount, 1, 16);
                ImGui::Unindent();
            }

            ImGui::Separator();
            ImGui::DragFloat("Shadow Bias", &m_viewportRenderer->getSceneInfo().shadowBias, 0.0001f, 0.0f, 0.1f);
            ImGui::Separator();
            static const float kShadowSoftnessLevels[] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
            static int softnessIndex = 0;

            ImGui::Text("Shadow Softness = %.1f", sceneInfo.shadowSoftness);
            if (ImGui::SliderInt(
                    "##Shadow Softness",
                    &softnessIndex,
                    0,
                    IM_ARRAYSIZE(kShadowSoftnessLevels) - 1))
            {
                sceneInfo.shadowSoftness = kShadowSoftnessLevels[softnessIndex];
            }
            ImGui::Separator();
            static const float kShadowMapSizeLevels[] = {1024, 2048, 3072, 4096, 5120};
            static int shadowMapSizeIndex = 0;
            ImGui::Text("Shadow MapSize = %.u", sceneInfo.shadowMapSize);
            if (ImGui::SliderInt(
                    "##ShadowMapSize",
                    &shadowMapSizeIndex,
                    0,
                    IM_ARRAYSIZE(kShadowMapSizeLevels) - 1))
            {
                sceneInfo.shadowMapSize = kShadowMapSizeLevels[shadowMapSizeIndex];
            }
            ImGui::End();
        }

        {
            std::string name = "None";
            if (m_hoveredEntity && m_hoveredEntity.hasComponent<TagComponent>())
            {
                name = m_hoveredEntity.getComponent<TagComponent>().tag;
            }
            ImGui::Begin("Debug Settings");

            ImGui::Text("hovered entity: %s", name.c_str());
            ImGui::Text("m_viewportFocused: %s", m_viewportFocused ? "yes" : "no");

            ImGui::Separator();
            auto &sceneInfo = m_viewportRenderer->getSceneInfo();
            const char *renderModes[] = {"Forward", "Deferred Hybrid"};
            int renderModeIndex = static_cast<int>(sceneInfo.renderMode);
            if (ImGui::Combo("Render Mode", &renderModeIndex, renderModes, IM_ARRAYSIZE(renderModes)))
            {
                sceneInfo.renderMode = static_cast<SceneRenderer::RenderMode>(renderModeIndex);
                if (sceneInfo.renderMode == SceneRenderer::RenderMode::Forward)
                    sceneInfo.gbufferDebug = SceneRenderer::GBufferDebugMode::None;
            }

            const char *gbufferModes[] = {"None", "Albedo", "Normal", "Material", "Roughness", "Metallic", "AO", "Emissive", "Depth", "ObjectID", "SSGI", "GTAO"};
            int gbufferModeIndex = static_cast<int>(sceneInfo.gbufferDebug);
            if (ImGui::Combo("GBuffer Debug", &gbufferModeIndex, gbufferModes, IM_ARRAYSIZE(gbufferModes)))
            {
                sceneInfo.gbufferDebug = static_cast<SceneRenderer::GBufferDebugMode>(gbufferModeIndex);
                if (sceneInfo.gbufferDebug != SceneRenderer::GBufferDebugMode::None)
                    sceneInfo.renderMode = SceneRenderer::RenderMode::DeferredHybrid;
            }

            ImGui::Checkbox("show depth buffer", &m_viewportRenderer->getSceneInfo().enableDepthView);
            if (m_viewportRenderer->getSceneInfo().enableDepthView)
            {
                ImGui::Indent();
                ImGui::SliderFloat("Depth Power", &m_viewportRenderer->getSceneInfo().depthViewPower, 0.1f, 10.0f, "%.2f");
                ImGui::Unindent();
            }
            ImGui::Checkbox("showPhysicsColliders", &m_showPhysicsColliders);
            ImGui::Checkbox("showRenderEntities", &m_showRenderEntities);

            ImGui::SeparatorText("Outline Settings");
            ImGui::ColorEdit4("Outline Color", glm::value_ptr(sceneInfo.meshOutlineColor));
            ImGui::Separator();
            ImGui::Image((ImTextureID)s_Font->getAtlasTexture()->getRendererID(),
                         {512, 512}, {0, 1}, {1, 0});
            ImGui::End();
        }
    }

    void BosonLayer::onViewportPanel()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0.117f, 0.117f, 0.117f, 1.0f});
        ImGui::Begin("Viewport");
        const auto viewportOffset = ImGui::GetCursorPos();
        m_viewportTabBarHeight = viewportOffset.y;
        m_viewportFocused = ImGui::IsWindowFocused();
        m_viewportHovered = ImGui::IsWindowHovered();

        const bool fpsMode = m_editorCamera.isFPSMode();
        if (fpsMode)
        {
            m_viewportFocused = true;
            m_viewportHovered = true;
        }

        if (m_viewportHovered)
        {
            ImGui::SetWindowFocus();
        }
        ImGuiIO &io = ImGui::GetIO();

        if (fpsMode)
        {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
            Application::get().getImGuiLayer()->blockEvents(false);
        }
        else
        {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            Application::get().getImGuiLayer()->blockEvents(!m_viewportFocused || !m_viewportHovered);
        }

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

        m_viewportSize = {viewportPanelSize.x, viewportPanelSize.y};

        // 接收framebuffer到imgui image进行渲染
        const uint32_t textureID = m_framebuffer->getColorAttachmentRendererID(0);
        ImGui::Image(reinterpret_cast<void *>(static_cast<intptr_t>(textureID)),
                     ImVec2(viewportPanelSize.x, viewportPanelSize.y),
                     ImVec2(0, 1), ImVec2(1, 0));

        // 接收 .fmscene 拖放
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_SCENE"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0])
                {
                    openScene(std::string(path));
                }
            }
            ImGui::EndDragDropTarget();
        }
        // 接受 .fmproj 拖放
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_PROJECT"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0])
                {
                    openProject(std::string(path));
                }
            }
            ImGui::EndDragDropTarget();
        }

        const ImVec2 windowPos = ImGui::GetWindowPos();

        m_viewport.min = {
            windowPos.x + viewportOffset.x,
            windowPos.y + viewportOffset.y};

        m_viewport.max = {
            m_viewport.min.x + viewportPanelSize.x,
            m_viewport.min.y + viewportPanelSize.y};

        // ImGuiZmo
        Entity selectedEntity = m_sceneHierarchyPanel.getSelectedEntity();

        if (selectedEntity && m_gizmoType != -1)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(m_viewport.min.x, m_viewport.min.y, m_viewport.size().x, m_viewport.size().y);
            // editorCamera
            const glm::mat4 &cameraProjection = m_editorCamera.getProjection();
            const glm::mat4 &cameraView = m_editorCamera.getViewMatrix();
            // Entity
            auto &transformComponent = selectedEntity.getComponent<TransformComponent>();
            glm::mat4 worldTransform = m_activeScene->getEntityManager().getWorldSpaceTransformMatrix(selectedEntity);

            bool snap = Input::isKeyPressed(KeyCode::LeftAlt);
            float snapValue = 0.5f;
            if (m_gizmoType == ImGuizmo::OPERATION::ROTATE)
                snapValue = 45.0f;

            float snapValues[3] = {snapValue, snapValue, snapValue};
            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                                 (ImGuizmo::OPERATION)m_gizmoType, ImGuizmo::LOCAL, glm::value_ptr(worldTransform), nullptr,
                                 snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                glm::mat4 localTransform = worldTransform;
                Entity parent = m_activeScene->getEntityManager().tryGetEntityByUUID(selectedEntity.getParentUUID());
                if (parent)
                {
                    glm::mat4 parentTransform = m_activeScene->getEntityManager().getWorldSpaceTransformMatrix(parent);
                    localTransform = glm::inverse(parentTransform) * worldTransform;
                }

                glm::vec3 translation, rotation, scale;
                Math::decomposeTransform(localTransform, translation, rotation, scale);

                switch ((ImGuizmo::OPERATION)m_gizmoType)
                {
                case ImGuizmo::TRANSLATE:
                    transformComponent.translation = translation;
                    break;
                case ImGuizmo::ROTATE:
                    transformComponent.setRotationEuler(rotation);
                    break;
                case ImGuizmo::SCALE:
                    transformComponent.scale = scale;
                    break;
                default:
                    break;
                }
            }
        }
        onOverlayViewportUI();

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
    void BosonLayer::onOverlayViewportUI()
    {
        const float iconSize = 28.0f;
        const float padding = 6.0f;
        const ImU32 backgroundColor = IM_COL32(30, 30, 30, 140);
        const ImVec4 orangeMain = ImVec4(0.92f, 0.45f, 0.11f, 0.8f);

        if (m_editorCamera.isFPSMode())
        {
            float centerY = m_viewport.size().y / 2.0f;
            ImGui::SetCursorPos(ImVec2(12.0f, centerY - 150.0f));
            ui::verticalProgressBar(m_editorCamera.getFPSSpeed(), 0.0f, 30.0f, ImVec2(12, 300.0f));
        }

        ImGuiChildFlags child_flags = ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

        ImGui::SetCursorPos(ImVec2(10.0f, m_viewportTabBarHeight + 10.0f));
        if (ImGui::BeginChild("GizmoToolbar", ImVec2(0, 0), child_flags, window_flags))
        {
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(ImGui::GetWindowPos(),
                                     ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight()),
                                     backgroundColor, 6.0f);

            auto drawGizmoBtn = [&](const char *label, int type, const char *tooltip)
            {
                bool isActive = (m_gizmoType == type);
                if (isActive)
                    ImGui::PushStyleColor(ImGuiCol_Button, orangeMain);
                else
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

                if (ImGui::Button(label, ImVec2(iconSize, iconSize)))
                    m_gizmoType = type;
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                    ImGui::SetTooltip("%s", tooltip);
                ImGui::PopStyleColor();
            };

            drawGizmoBtn("Q", -1, "Select (Q)");
            ImGui::SameLine();
            drawGizmoBtn("W", (int)ImGuizmo::TRANSLATE, "Translate (W)");
            ImGui::SameLine();
            drawGizmoBtn("E", (int)ImGuizmo::ROTATE, "Rotate (E)");
            ImGui::SameLine();
            drawGizmoBtn("R", (int)ImGuizmo::SCALE, "Scale (R)");
        }
        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(m_viewport.size().x - 120.0f, m_viewportTabBarHeight + 10.0f));

        if (ImGui::BeginChild("SceneControl", ImVec2(0, 0), child_flags, window_flags))
        {
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(ImGui::GetWindowPos(),
                                     ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight()),
                                     backgroundColor, 6.0f);

            const bool toolbarEnabled = (bool)m_activeScene;
            ImVec4 tintColor = toolbarEnabled ? ImVec4(1, 1, 1, 1) : ImVec4(1, 1, 1, 0.5f);

            bool isEdit = m_sceneState == SceneState::Edit;
            bool isPlay = m_sceneState == SceneState::Play;
            bool isSim = m_sceneState == SceneState::Simulate;

            auto drawImgBtn = [&](const char *id, auto &iconRef, ImVec4 bg, auto func)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, bg);

                auto textureID = (ImTextureID)(uintptr_t)iconRef->getRendererID();
                if (ImGui::ImageButton(id, textureID, ImVec2(iconSize - 6, iconSize - 6), {0, 0}, {1, 1}, {0, 0, 0, 0}, tintColor))
                {
                    if (toolbarEnabled)
                        func();
                }
                ImGui::PopStyleColor();
            };

            if (isEdit)
            {
                drawImgBtn("##Play", m_iconPlay, ImVec4(0, 0, 0, 0), [this]()
                           { onScenePlay(); });
            }
            else if (isPlay)
            {
                drawImgBtn("##Stop", m_iconStop, ImVec4(0.8f, 0.1f, 0.1f, 0.6f), [this]()
                           { onSceneStop(); });
            }

            ImGui::SameLine();

            if (isEdit)
            {
                drawImgBtn("##Sim", m_iconSimulate, ImVec4(0, 0, 0, 0), [this]()
                           { onSceneSimulate(); });
            }
            else if (isSim)
            {
                drawImgBtn("##StopSim", m_iconStop, ImVec4(0.8f, 0.1f, 0.1f, 0.6f), [this]()
                           { onSceneStop(); });
            }

            if (isSim && m_activeScene)
            {
                ImGui::SameLine();
                bool isPaused = m_activeScene->isPaused();
                drawImgBtn("##Pause", m_iconPause, isPaused ? orangeMain : ImVec4(0, 0, 0, 0), [this, isPaused]()
                           { m_activeScene->setPaused(!isPaused); });

                if (isPaused)
                {
                    ImGui::SameLine();
                    drawImgBtn("##Step", m_iconStep, ImVec4(0, 0, 0, 0), [this]()
                               { m_activeScene->step(); });
                }
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(3);
    }
    void BosonLayer::updateMousePicking()
    {
        m_hoveredEntity = {};

        if (!m_viewport.isValid())
            return;

        const auto [mx, my] = ImGui::GetMousePos();
        const glm::vec2 mouseScreen{mx, my};

        if (!m_viewport.contains(mouseScreen))
            return;

        // screen to viewport local
        const glm::vec2 local = mouseScreen - m_viewport.min;
        const glm::vec2 size = m_viewport.size();

        const int pixelX = static_cast<int>(local.x);
        const int pixelY = static_cast<int>(size.y - local.y);

        int entityID = -1;
        if (m_framebuffer)
            entityID = m_framebuffer->readPixel(1, pixelX, pixelY);

        Entity hovered{static_cast<entt::entity>(entityID), m_activeScene.get()};
        if (!hovered && m_viewportRenderer)
        {
            const auto &sceneInfo = m_viewportRenderer->getSceneInfo();
            if (sceneInfo.renderMode == SceneRenderer::RenderMode::DeferredHybrid)
            {
                if (auto gbuffer = m_viewportRenderer->getGBufferFramebuffer())
                {
                    entityID = gbuffer->readPixel(static_cast<uint32_t>(SceneRenderer::GBufferAttachment::ObjectID), pixelX, pixelY);
                    hovered = Entity{static_cast<entt::entity>(entityID), m_activeScene.get()};
                }
            }
        }

        if (hovered.isValid())
            m_hoveredEntity = hovered;
    }

    bool BosonLayer::beginOverlayPass() const
    {
        if (m_sceneState == SceneState::Play)
        {
            Entity camera = m_activeScene->getEntityManager().getPrimaryCameraEntity();
            if (!camera)
                return false;

            m_viewportRenderer->beginOverlay(camera.getComponent<CameraComponent>().camera,
                                             camera.getComponent<TransformComponent>().getTransform());
        }
        else
        {
            m_viewportRenderer->beginOverlay(m_editorCamera);
        }

        return true;
    }

    void BosonLayer::renderPhysicsColliders() const
    {
        renderPhysics2DColliders();
        renderPhysics3DColliders();
    }

    void BosonLayer::renderPhysics2DColliders() const
    {
        auto getParentTransform = [&](Entity entity)
        {
            Entity parent = m_activeScene->getEntityManager().tryGetEntityByUUID(entity.getParentUUID());
            if (parent)
                return m_activeScene->getEntityManager().getWorldSpaceTransformMatrix(parent);
            return glm::mat4(1.0f);
        };

        // Box Colliders
        {
            auto view = m_activeScene->getAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, m_activeScene.get()};
                auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

                glm::vec3 translation = tc.translation + glm::vec3(bc2d.offset, 0.001f);
                glm::vec3 scale = tc.scale * glm::vec3(bc2d.size * 2.0f, 1.0f);

                glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), tc.translation) *
                                           glm::rotate(glm::mat4(1.0f), tc.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                           glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.offset, 0.001f)) * glm::scale(glm::mat4(1.0f), scale);
                glm::mat4 transform = getParentTransform(colliderEntity) * localTransform;

                m_viewportRenderer->drawRect(transform, glm::vec4(0, 1, 0, 1));
            }
        }

        // Circle Colliders
        {
            auto view = m_activeScene->getAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, m_activeScene.get()};
                auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

                // 单位 quad 半径 0.5 * scale.x = cc2d.radius * tc.scale.x
                glm::vec3 scale = tc.scale * glm::vec3(cc2d.radius * 2.0f, cc2d.radius * 2.0f, 1.0f);

                // 先平移到实体，再旋转，再平移 offset，再缩放
                glm::mat4 localTransform =
                    glm::translate(glm::mat4(1.0f), tc.translation) * glm::rotate(glm::mat4(1.0f), tc.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(cc2d.offset, 0.001f)) * glm::scale(glm::mat4(1.0f), scale);
                glm::mat4 transform = getParentTransform(colliderEntity) * localTransform;

                m_viewportRenderer->drawCircle(transform, glm::vec4(0, 1, 0, 1), 0.1f);
            }
        }
        // Box Sensor
        {
            auto view = m_activeScene->getAllEntitiesWith<TransformComponent, BoxSensor2DComponent>();
            for (auto entity : view)
            {
                Entity sensor = {entity, m_activeScene.get()};
                auto [tc, bs2d] = view.get<TransformComponent, BoxSensor2DComponent>(entity);
                glm::vec3 translation = tc.translation + glm::vec3(bs2d.offset, 0.001f);
                glm::vec3 scale = tc.scale * glm::vec3(bs2d.size * 2.0f, 1.0f);

                glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), tc.translation) *
                                           glm::rotate(glm::mat4(1.0f), tc.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                           glm::translate(glm::mat4(1.0f), glm::vec3(bs2d.offset, 0.001f)) * glm::scale(glm::mat4(1.0f), scale);
                glm::mat4 transform = getParentTransform(sensor) * localTransform;

                m_viewportRenderer->drawRect(transform, glm::vec4(0, 1, 1, 1));
            }
        }
    }

    void BosonLayer::renderPhysics3DColliders() const
    {
        constexpr float kTwoPi = 6.28318530718f;
        constexpr int kCircleSegments = 24;
        constexpr float kMinRadius = 0.001f;
        constexpr float kMinHalfHeight = 0.001f;
        const glm::vec4 collider3DColor{0.0f, 1.0f, 0.0f, 1.0f};

        auto getParentTransform = [&](Entity entity)
        {
            Entity parent = m_activeScene->getEntityManager().tryGetEntityByUUID(entity.getParentUUID());
            if (parent)
                return m_activeScene->getEntityManager().getWorldSpaceTransformMatrix(parent);
            return glm::mat4(1.0f);
        };

        auto getParentRotationScale = [&](Entity entity, glm::mat3 &rotation, glm::vec3 &scale)
        {
            glm::mat4 parentTransform = getParentTransform(entity);
            glm::mat3 basis = glm::mat3(parentTransform);

            scale = {
                glm::length(basis[0]),
                glm::length(basis[1]),
                glm::length(basis[2])};

            rotation = glm::mat3(1.0f);
            if (scale.x > 0.0f)
                rotation[0] = basis[0] / scale.x;
            if (scale.y > 0.0f)
                rotation[1] = basis[1] / scale.y;
            if (scale.z > 0.0f)
                rotation[2] = basis[2] / scale.z;
        };

        auto drawArc = [&](const glm::vec3 &center, const glm::vec3 &axisA, const glm::vec3 &axisB, float radius,
                           float startAngle, float endAngle, int segments, const glm::vec4 &color)
        {
            if (radius <= 0.0f || segments < 1)
                return;

            glm::vec3 u = glm::normalize(axisA);
            glm::vec3 v = glm::normalize(axisB);
            float step = (endAngle - startAngle) / static_cast<float>(segments);
            glm::vec3 prev = center + (u * std::cos(startAngle) + v * std::sin(startAngle)) * radius;

            for (int i = 1; i <= segments; ++i)
            {
                float angle = startAngle + step * static_cast<float>(i);
                glm::vec3 point = center + (u * std::cos(angle) + v * std::sin(angle)) * radius;
                m_viewportRenderer->drawLine(prev, point, color);
                prev = point;
            }
        };

        auto drawCircle = [&](const glm::vec3 &center, const glm::vec3 &axisA, const glm::vec3 &axisB, float radius,
                              const glm::vec4 &color)
        {
            drawArc(center, axisA, axisB, radius, 0.0f, kTwoPi, kCircleSegments, color);
        };

        auto drawBox = [&](const glm::mat4 &transform, const glm::vec4 &color)
        {
            const glm::vec3 localCorners[8] = {
                {-0.5f, -0.5f, -0.5f},
                {0.5f, -0.5f, -0.5f},
                {0.5f, 0.5f, -0.5f},
                {-0.5f, 0.5f, -0.5f},
                {-0.5f, -0.5f, 0.5f},
                {0.5f, -0.5f, 0.5f},
                {0.5f, 0.5f, 0.5f},
                {-0.5f, 0.5f, 0.5f}};

            glm::vec3 corners[8];
            for (int i = 0; i < 8; ++i)
            {
                corners[i] = transform * glm::vec4(localCorners[i], 1.0f);
            }

            const int edges[12][2] = {
                {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

            for (const auto &edge : edges)
            {
                m_viewportRenderer->drawLine(corners[edge[0]], corners[edge[1]], color);
            }
        };

        // 3D Box Colliders
        {
            auto view = m_activeScene->getAllEntitiesWith<TransformComponent, BoxCollider3DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, m_activeScene.get()};
                auto [tc, bc3d] = view.get<TransformComponent, BoxCollider3DComponent>(entity);

                glm::mat4 rotation = glm::toMat4(glm::quat(tc.rotation));
                glm::vec3 scale = tc.scale * (bc3d.size * 2.0f);
                glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), tc.translation) *
                                           rotation *
                                           glm::translate(glm::mat4(1.0f), bc3d.offset) *
                                           glm::scale(glm::mat4(1.0f), scale);
                glm::mat4 transform = getParentTransform(colliderEntity) * localTransform;

                drawBox(transform, collider3DColor);
            }
        }

        // 3D Sphere Colliders
        {
            auto view = m_activeScene->getAllEntitiesWith<TransformComponent, CircleCollider3DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, m_activeScene.get()};
                auto [tc, cc3d] = view.get<TransformComponent, CircleCollider3DComponent>(entity);

                glm::mat3 parentRotation;
                glm::vec3 parentScale;
                getParentRotationScale(colliderEntity, parentRotation, parentScale);

                glm::mat3 localRotation = glm::toMat3(glm::quat(tc.rotation));
                glm::mat3 worldRotation = parentRotation * localRotation;
                glm::vec3 worldScale = parentScale * tc.scale;

                glm::vec3 localCenter = tc.translation + (localRotation * cc3d.offset);
                glm::vec3 center = getParentTransform(colliderEntity) * glm::vec4(localCenter, 1.0f);
                float maxScale = std::max(worldScale.x, std::max(worldScale.y, worldScale.z));
                float radius = cc3d.radius * maxScale;
                if (radius <= 0.0f)
                    continue;

                glm::vec3 right = worldRotation * glm::vec3(1.0f, 0.0f, 0.0f);
                glm::vec3 up = worldRotation * glm::vec3(0.0f, 1.0f, 0.0f);
                glm::vec3 forward = worldRotation * glm::vec3(0.0f, 0.0f, 1.0f);

                drawCircle(center, right, up, radius, collider3DColor);
                drawCircle(center, right, forward, radius, collider3DColor);
                drawCircle(center, up, forward, radius, collider3DColor);
            }
        }

        // 3D Capsule Colliders
        {
            auto view = m_activeScene->getAllEntitiesWith<TransformComponent, CapsuleCollider3DComponent>();
            for (auto entity : view)
            {
                Entity colliderEntity{entity, m_activeScene.get()};
                auto [tc, cap3d] = view.get<TransformComponent, CapsuleCollider3DComponent>(entity);

                glm::mat3 parentRotation;
                glm::vec3 parentScale;
                getParentRotationScale(colliderEntity, parentRotation, parentScale);

                glm::mat3 localRotation = glm::toMat3(glm::quat(tc.rotation));
                glm::mat3 worldRotation = parentRotation * localRotation;
                glm::vec3 worldScale = parentScale * tc.scale;

                glm::vec3 localCenter = tc.translation + (localRotation * cap3d.offset);
                glm::vec3 center = getParentTransform(colliderEntity) * glm::vec4(localCenter, 1.0f);

                float radiusScale = std::max(worldScale.x, worldScale.z);
                float scaledRadius = cap3d.radius * radiusScale;
                float scaledHeight = cap3d.height * worldScale.y;

                if (scaledRadius < kMinRadius)
                    scaledRadius = kMinRadius;

                float halfHeight = scaledHeight * 0.5f;
                float halfCylinderHeight = halfHeight - scaledRadius;
                if (halfCylinderHeight < kMinHalfHeight)
                    halfCylinderHeight = kMinHalfHeight;

                glm::vec3 right = worldRotation * glm::vec3(1.0f, 0.0f, 0.0f);
                glm::vec3 up = worldRotation * glm::vec3(0.0f, 1.0f, 0.0f);
                glm::vec3 forward = worldRotation * glm::vec3(0.0f, 0.0f, 1.0f);

                glm::vec3 topCenter = center + up * halfCylinderHeight;
                glm::vec3 bottomCenter = center - up * halfCylinderHeight;

                drawCircle(topCenter, right, forward, scaledRadius, collider3DColor);
                drawCircle(bottomCenter, right, forward, scaledRadius, collider3DColor);

                m_viewportRenderer->drawLine(topCenter + right * scaledRadius, bottomCenter + right * scaledRadius,
                                             collider3DColor);
                m_viewportRenderer->drawLine(topCenter - right * scaledRadius, bottomCenter - right * scaledRadius,
                                             collider3DColor);
                m_viewportRenderer->drawLine(topCenter + forward * scaledRadius, bottomCenter + forward * scaledRadius,
                                             collider3DColor);
                m_viewportRenderer->drawLine(topCenter - forward * scaledRadius, bottomCenter - forward * scaledRadius,
                                             collider3DColor);

                const int arcSegments = kCircleSegments / 2;
                drawArc(topCenter, right, up, scaledRadius, 0.0f, kTwoPi * 0.5f, arcSegments, collider3DColor);
                drawArc(bottomCenter, right, up, scaledRadius, kTwoPi * 0.5f, kTwoPi, arcSegments, collider3DColor);
                drawArc(topCenter, forward, up, scaledRadius, 0.0f, kTwoPi * 0.5f, arcSegments, collider3DColor);
                drawArc(bottomCenter, forward, up, scaledRadius, kTwoPi * 0.5f, kTwoPi, arcSegments, collider3DColor);
            }
        }
    }

    void BosonLayer::renderSelectedEntityOutline() const
    {
        if (Entity selectedEntity = m_sceneHierarchyPanel.getSelectedEntity(); selectedEntity)
        {
            glm::mat4 worldTransform = m_activeScene->getEntityManager().getWorldSpaceTransformMatrix(selectedEntity);
            if (selectedEntity.hasComponent<MeshComponent>())
            {
                if (m_viewportRenderer->getSceneInfo().renderMode == SceneRenderer::RenderMode::Forward)
                {
                    m_viewportRenderer->submitMesh(selectedEntity.getComponent<MeshComponent>(),
                                                   worldTransform, (int)selectedEntity, true);
                }
            }
            else
            {
                m_viewportRenderer->drawRect(worldTransform, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
        }
    }

    void BosonLayer::onOverlayRender() const
    {
        if (!beginOverlayPass())
            return;

        if (m_showPhysicsColliders)
        {
            renderPhysicsColliders();
        }

        renderSelectedEntityOutline();
        m_viewportRenderer->endOverlay();
    }

    void BosonLayer::newProject()
    {
        std::filesystem::path path = FileDialogs::saveFile(
            "Project (*.fmproj)\0*.fmproj\0", "../Boson/assets/project/");
        if (path.empty())
        {
            Log::Warn("Project Selection directory is empty!");
            return;
        }

        if (std::filesystem::create_directories(path.parent_path() / "Assets"))
            Log::Info(std::format("Assets directory created successfully! Path: {}",
                                  (path.parent_path() / "Assets").string()));
        else
            Log::Warn(std::format("Assets directory already exists! Path: {}",
                                  (path.parent_path() / "Assets").string()));

        Project::newProject();

        auto &config = Project::getActive()->getConfig();
        config.name = path.stem().string();
        config.startScene = "";
        config.assetDirectory = path.parent_path() / "Assets";
        config.scriptDirectory = config.assetDirectory / "scripts";

        if (Project::saveActive(path))
            Log::Info(std::format("Project created successfully! Path: {}",
                                  Project::getActive()->getProjectPath().string()));
        else
            Log::Error("Project create failed!");

        m_contentBrowserPanel.setBaseDirectory(Project::getActive()->getConfig().assetDirectory);
    }

    void BosonLayer::openProject()
    {
        std::filesystem::path path = FileDialogs::openFile(
            "Project (*.fmproj)\0*.fmproj\0", "../Boson/assets/project/");
        if (path.empty())
        {
            Log::Warn("Project Selection directory is empty!");
            return;
        }

        openProject(path);
    }

    void BosonLayer::openProject(const std::filesystem::path &path)
    {
        std::filesystem::path projectPath = path;
        if (projectPath.empty())
        {
            Log::Warn("Empty project path supplied.");
            return;
        }

        if (!m_isInitialized)
        {
            m_pendingProjectPath = projectPath;
            return;
        }

        std::error_code ec;
        if (std::filesystem::is_directory(projectPath, ec) && !ec)
        {
            auto descriptor = findProjectFileInDirectory(projectPath);
            if (descriptor.empty())
            {
                Log::Error(std::format("No .fmproj/.fproject found inside directory: {}", projectPath.string()));
                return;
            }
            projectPath = descriptor;
        }

        auto project = Project::loadProject(projectPath);
        FERMION_ASSERT(project != nullptr, "Failed to load project!");
        auto lastScene = Project::getActive()->getConfig().startScene;
        if (!lastScene.empty())
            openScene(lastScene);
        m_contentBrowserPanel.setBaseDirectory(Project::getActive()->getProjectDirectory());
    }

    void BosonLayer::saveProject()
    {
        saveScene();
        auto &config = Project::getActive()->getConfig();
        config.startScene = m_editorScenePath;
        if (Project::saveActive(Fermion::Project::getProjectPath()))
        {
            Log::Info("Project save successfully!");
        }
        else
        {
            Log::Error("Project save failed!");
        }
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

    void BosonLayer::newScene()
    {
        m_activeScene = std::make_shared<Scene>();
        m_activeScene->onViewportResize(static_cast<uint32_t>(m_viewportSize.x),
                                        static_cast<uint32_t>(m_viewportSize.y));
        m_editorScene = m_activeScene;
        m_editorScenePath.clear();
        m_editorSceneHandle = AssetHandle{};
        m_sceneHierarchyPanel.setContext(m_activeScene);
        m_viewportRenderer->setScene(m_activeScene);
    }

    void BosonLayer::saveSceneAs()
    {
        std::string defaultDir = "../Boson/assets/scenes/";
        if (auto project = Project::getActive())
        {
            const auto &assetDir = project->getConfig().assetDirectory;
            if (!assetDir.empty())
                defaultDir = assetDir.string();
        }

        auto path = FileDialogs::saveFile(
            "Scene (*.fmscene)\0*.fmscene\0", defaultDir);
        if (path.empty())
            return;

        SceneSerializer serializer(m_editorScene);
        serializer.serialize(path);
        m_editorScenePath = path;

        auto editorAssets = Project::getEditorAssetManager();
        AssetHandle handle = editorAssets->importAsset(path);
        if (static_cast<uint64_t>(handle) != 0)
            m_editorSceneHandle = handle;

        Log::Info(std::format("Scene saved successfully! Path: {}",
                              path.string()));
    }

    void BosonLayer::saveScene()
    {
        if (!m_editorScenePath.empty())
        {
            SceneSerializer serializer(m_editorScene);
            serializer.serialize(m_editorScenePath);

            auto editorAssets = Project::getEditorAssetManager();
            AssetHandle handle = editorAssets->importAsset(m_editorScenePath);
            if (static_cast<uint64_t>(handle) != 0)
                m_editorSceneHandle = handle;

            Log::Info(std::format("Scene saved successfully! Path: {}",
                                  m_editorScenePath.string()));
        }
        else
        {
            saveSceneAs();
        }
    }

    void BosonLayer::openScene()
    {
        std::string defaultDir = "../Boson/assets/scenes/";
        if (auto project = Project::getActive())
        {
            const auto &assetDir = project->getConfig().assetDirectory;
            if (!assetDir.empty())
                defaultDir = assetDir.string();
        }

        std::filesystem::path path = FileDialogs::openFile(
            "Scene (*.fmscene)\0*.fmscene\0", defaultDir);
        if (!path.empty())
        {
            openScene(path);
        }
    }

    void BosonLayer::openScene(const std::filesystem::path &path)
    {
        if (path.empty())
        {
            Log::Warn("openScene called with empty path");
            return;
        }

        if (m_sceneState != SceneState::Edit)
        {
            onSceneStop();
        }
        auto editorAssets = Project::getEditorAssetManager();
        AssetHandle handle = editorAssets->importAsset(path);
        if (static_cast<uint64_t>(handle) == 0)
        {
            Log::Error(std::format("Scene open failed (invalid asset handle)! Path: {}",
                                   path.string()));
            return;
        }

        const auto sceneAsset = editorAssets->getAsset<SceneAsset>(handle);
        if (!sceneAsset || !sceneAsset->scene)
        {
            Log::Error(std::format("Scene open failed (asset load failed)! Path: {}",
                                   path.string()));
            return;
        }

        if (std::shared_ptr<Scene> newScene = sceneAsset->scene)
        {
            m_editorSceneHandle = handle;
            m_editorScene = newScene;
            m_editorScene->onViewportResize(static_cast<uint32_t>(m_viewportSize.x),
                                            static_cast<uint32_t>(m_viewportSize.y));
            m_activeScene = m_editorScene;
            m_editorScenePath = path;
            m_sceneHierarchyPanel.setContext(m_activeScene);
            m_viewportRenderer->setScene(m_activeScene);
            Log::Info(std::format("Scene opened successfully! Path: {}",
                                  path.string()));
        }
        else
        {
            Log::Error(std::format("Scene open failed! Path: {}",
                                   path.string()));
        }
    }

    void BosonLayer::onScenePlay()
    {
        m_hoveredEntity = {};
        if (m_sceneState == SceneState::Simulate)
            onSceneStop();
        m_sceneState = SceneState::Play;

        m_activeScene = Scene::copy(m_editorScene);
        m_activeScene->onRuntimeStart();
        m_sceneHierarchyPanel.setEditingEnabled(false);
        m_viewportRenderer->setScene(m_activeScene);
        m_sceneHierarchyPanel.setContext(m_activeScene);
    }

    void BosonLayer::onSceneSimulate()
    {
        if (m_sceneState == SceneState::Play)
            onSceneStop();
        m_sceneState = SceneState::Simulate;
        m_activeScene = Scene::copy(m_editorScene);
        m_activeScene->onSimulationStart();
        m_sceneHierarchyPanel.setEditingEnabled(false);
        m_viewportRenderer->setScene(m_activeScene);
        m_sceneHierarchyPanel.setContext(m_activeScene);
    }

    void BosonLayer::onSceneStop()
    {
        if (m_sceneState == SceneState::Play)
            m_activeScene->onRuntimeStop();
        else if (m_sceneState == SceneState::Simulate)
            m_activeScene->onSimulationStop();

        m_sceneState = SceneState::Edit;
        m_activeScene = m_editorScene;
        m_viewportRenderer->setScene(m_activeScene);
        m_sceneHierarchyPanel.setEditingEnabled(true);
        m_sceneHierarchyPanel.setContext(m_activeScene);

        m_hoveredEntity = {};
    }
} // namespace Fermion
