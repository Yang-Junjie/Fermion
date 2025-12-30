#include "Fermion.hpp"
#include "BosonLayer.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Renderer/Font/Font.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Asset/SceneAsset.hpp"
#include "Script/ScriptManager.hpp"

#include "imgui/ConsolePanel.hpp"
#include "Math/Math.hpp"

#include <imgui.h>
#include <ImGuizmo.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
namespace Fermion {

static std::shared_ptr<Font> s_Font; // TODO:  Temporary
BosonLayer::BosonLayer(const std::string &name) : Layer(name) {
    s_Font = Font::getDefault();
}

void BosonLayer::onAttach() {
    FM_PROFILE_FUNCTION();

    m_iconPlay = Texture2D::create("../Boson/Resources/Icons/PlayButton.png");
    m_iconStop = Texture2D::create("../Boson/Resources/Icons/StopButton.png");
    m_iconPause = Texture2D::create("../Boson/Resources/Icons/PauseButton.png");
    m_iconSimulate = Texture2D::create("../Boson/Resources/Icons/SimulateButton.png");
    m_iconStep = Texture2D::create("../Boson/Resources/Icons/StepButton.png");
    FramebufferSpecification fbSpec;

    fbSpec.width = 1280;
    fbSpec.height = 720;
    fbSpec.attachments = {FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::DEPTH24STENCIL8};

    m_framebuffer = Framebuffer::create(fbSpec);

    m_activeScene = std::make_shared<Scene>();
    m_viewportRenderer = std::make_shared<SceneRenderer>();

    m_editorScene = m_activeScene;
    m_editorCamera = EditorCamera(45.0f, static_cast<float>(fbSpec.width) / static_cast<float>(fbSpec.height), 0.1f, 1000.0f);

    m_sceneHierarchyPanel.setContext(m_activeScene);
    m_viewportRenderer->setScene(m_activeScene);
    ScriptManager::get()->setSceneRenderer(m_viewportRenderer);

    MenuBarCallbacks callbacks;
    callbacks.NewScene = [this]() { newScene(); };
    callbacks.OpenScene = [this]() { openScene(); };
    callbacks.SaveScene = [this]() { saveScene(); };
    callbacks.SaveSceneAs = [this]() { saveSceneAs(); };
    callbacks.NewProject = [this]() { newProject(); };
    callbacks.OpenProject = [this]() { openProject(); };
    callbacks.SaveProject = [this]() { saveProject(); };
    callbacks.ExitApplication = [this]() {
        saveProject();
        Application::get().close();
    };
    callbacks.ShowAbout = [this]() { m_showAboutWindow = true; };
    m_menuBarPanel.SetCallbacks(callbacks);
}

void BosonLayer::onDetach() {
    FM_PROFILE_FUNCTION();
}

void BosonLayer::onUpdate(Timestep dt) {
    FM_PROFILE_FUNCTION();

    // Resize
    if (FramebufferSpecification spec = m_framebuffer->getSpecification();
        m_viewportSize.x > 0 && m_viewportSize.y > 0 && (spec.width != m_viewportSize.x || spec.height != m_viewportSize.y)) {
        m_framebuffer->resize(m_viewportSize.x, m_viewportSize.y);
        m_editorCamera.setViewportSize(m_viewportSize.x, m_viewportSize.y);
        m_activeScene->onViewportResize(m_viewportSize.x, m_viewportSize.y);
    }

    Renderer2D::resetStatistics();
    m_framebuffer->bind();
    RenderCommand::setClearColor({0.1f, 0.1f, 0.1f, 1.0f});
    RenderCommand::clear();
    m_framebuffer->clearAttachment(1, -1);

    if (m_sceneState == SceneState::Play) {
        m_activeScene->onUpdateRuntime(m_viewportRenderer, dt, m_showRenderEntities);
        m_sceneHierarchyPanel.setSelectedEntity({});
    } else if (m_sceneState == SceneState::Simulate) {
        m_editorCamera.onUpdate(dt);

        m_activeScene->onUpdateSimulation(m_viewportRenderer, dt, m_editorCamera, m_showRenderEntities);
    } else // Edit
    {
        m_editorCamera.onUpdate(dt);

        m_activeScene->onUpdateEditor(m_viewportRenderer, dt, m_editorCamera, m_showRenderEntities);
    }

    // mouse picking
    auto [mx, my] = ImGui::GetMousePos();
    mx -= (float)m_viewportBounds[0].x;
    my -= (float)m_viewportBounds[0].y;
    const glm::vec2 viewportSize = m_viewportBounds[1] - m_viewportBounds[0];
    if (viewportSize.x > 0.0f && viewportSize.y > 0.0f) {
        my = viewportSize.y - my;
        const int mouseX = static_cast<int>(mx);
        const int mouseY = static_cast<int>(my);
        if (mouseX >= 0 && mouseY >= 0 && mouseX < static_cast<int>(viewportSize.x) && mouseY < static_cast<int>(viewportSize.y)) {
            int pixelData = m_framebuffer->readPixel(1, mouseX, mouseY);
            Entity hovered = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_activeScene.get());
            m_hoveredEntity = hovered && hovered.isValid() ? hovered : Entity();
        } else {
            m_hoveredEntity = {};
        }
    } else {
        m_hoveredEntity = {};
    }
    onOverlayRender();

    m_framebuffer->unbind();
}

void BosonLayer::onImGuiRender() {
    FM_PROFILE_FUNCTION();

    static bool dockspaceOpen = true;
    if (dockspaceOpen) {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
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
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        } else {
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
            style.WindowMinSize.x = 370.0f;
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }
            style.WindowMinSize.x = minWinSizeX;
            ImGui::End();
        }
        // Scene Hierarchy

        m_sceneHierarchyPanel.onImGuiRender();
        m_contentBrowserPanel.onImGuiRender();
        m_assetManagerPanel.onImGuiRender();
        m_menuBarPanel.OnImGuiRender();
        ConsolePanel::get().onImGuiRender();
        //ImGui::ShowDemoWindow();

        onUIToolPanel();
        onHelpPanel();
        onSettingsPanel();
        onViewportPanel();
    }
}

void BosonLayer::onEvent(IEvent &event) {
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<WindowResizeEvent>([](WindowResizeEvent &e) { return false; });
    dispatcher.dispatch<KeyPressedEvent>([this](KeyPressedEvent &e) { return this->onKeyPressedEvent(e); });
    dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent &e) { return this->onMouseButtonPressedEvent(e); });
    // m_cameraController.onEvent(event);
    m_editorCamera.onEvent(event);
}

bool BosonLayer::onKeyPressedEvent(KeyPressedEvent &e) {
    if (e.isRepeat()) {
        return false;
    }

    bool control = Input::isKeyPressed(KeyCode::LeftControl) || Input::isKeyPressed(KeyCode::RightControl);
    bool shift = Input::isKeyPressed(KeyCode::LeftShift) || Input::isKeyPressed(KeyCode::RightShift);

    switch (e.getKeyCode()) {
    case KeyCode::S:
        if (control && shift) {
            saveSceneAs();
            return true;
        } else if (control) {
            saveScene();
            return true;
        }
        break;
    case KeyCode::N:
        if (control) {
            newScene();
            return true;
        }
        break;
    case KeyCode::O:
        if (control) {
            openScene();
            return true;
        }
        break;
    case KeyCode::D:
        if (control) {
            if (Entity selected = m_sceneHierarchyPanel.getSelectedEntity(); selected) {
                m_activeScene->destroyEntity(selected);
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
        m_gizmoType = ImGuizmo::OPERATION::SCALE;
        break;
    }

    return false;
}

bool BosonLayer::onMouseButtonPressedEvent(MouseButtonPressedEvent &e) {
    if (e.getMouseButton() == MouseCode::Left) {
        if (m_viewportHovered && !ImGuizmo::IsOver())
            m_sceneHierarchyPanel.setSelectedEntity(m_hoveredEntity && m_hoveredEntity.isValid() ? m_hoveredEntity : Entity());
    }
    return false;
}

void BosonLayer::onUIToolPanel() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0.117f, 0.117f, 0.117f, 1.0f});

    auto &colors = ImGui::GetStyle().Colors;
    const auto &buttonHovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
    const auto &buttonActive = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

    ImGui::Begin("toolbar", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    std::shared_ptr<Texture2D> icon = m_sceneState == SceneState::Edit ? m_iconPlay : m_iconStop;
    bool toolbarEnabled = (bool)m_activeScene;

    ImVec4 tintColor = ImVec4(1, 1, 1, 1);
    if (!toolbarEnabled)
        tintColor.w = 0.5f;

    float size = ImGui::GetWindowHeight() - 10.0f;
    ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

    bool hasPlayButton = m_sceneState == SceneState::Edit || m_sceneState == SceneState::Play;
    bool hasSimulateButton = m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate;
    bool hasPauseButton = m_sceneState != SceneState::Edit && m_sceneState != SceneState::Play;

    if (hasPlayButton) {
        std::shared_ptr<Texture2D> icon = (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate) ? m_iconPlay : m_iconStop;
        if (ImGui::ImageButton("##toolbar_playbtn",
                               (ImTextureID) static_cast<uint64_t>(icon->getRendererID()),
                               ImVec2(size, size),
                               ImVec2(0, 0), ImVec2(1, 1),
                               ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)) {
            if (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Simulate)
                onScenePlay();
            else if (m_sceneState == SceneState::Play)
                onSceneStop();
        }
    }

    if (hasSimulateButton) {
        if (hasPlayButton)
            ImGui::SameLine();

        std::shared_ptr<Texture2D> icon = (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Play) ? m_iconSimulate : m_iconStop;
        if (ImGui::ImageButton("##toolbar_simulatebtn",
                               (ImTextureID)(uint64_t)icon->getRendererID(),
                               ImVec2(size, size),
                               ImVec2(0, 0), ImVec2(1, 1),
                               ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)) {
            if (m_sceneState == SceneState::Edit || m_sceneState == SceneState::Play)
                onSceneSimulate();
            else if (m_sceneState == SceneState::Simulate)
                onSceneStop();
        }
    }
    if (hasPauseButton) {
        bool isPaused = m_activeScene->isPaused();
        ImGui::SameLine();
        {
            std::shared_ptr<Texture2D> icon = m_iconPause;
            if (ImGui::ImageButton("##toolbar_pausebtn",
                                   (ImTextureID)(uint64_t)icon->getRendererID(),
                                   ImVec2(size, size),
                                   ImVec2(0, 0), ImVec2(1, 1),
                                   ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)) {
                m_activeScene->setPaused(!isPaused);
            }
        }

        // Step button
        if (isPaused) {
            ImGui::SameLine();
            {
                std::shared_ptr<Texture2D> icon = m_iconStep;
                bool isPaused = m_activeScene->isPaused();
                if (ImGui::ImageButton("##toolbar_stepbtn",
                                       (ImTextureID)(uint64_t)icon->getRendererID(),
                                       ImVec2(size, size),
                                       ImVec2(0, 0), ImVec2(1, 1),
                                       ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)) {
                    m_activeScene->step();
                }
            }
        }
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);
    ImGui::End();
}

void BosonLayer::onHelpPanel() {
    if (m_showAboutWindow) {
        ImGui::SetNextWindowSize(ImVec2(350, 0), ImGuiCond_FirstUseEver);

        ImGui::Begin("About Fermion", &m_showAboutWindow, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking);

        ImGui::SeparatorText("About Fermion");
        ImGui::Text("Fermion Engine v0.1");
        ImGui::Text("Copyright (c) 2025, Beisent");
        ImGui::Text("This is free and open-source software under the MIT License.");
        ImGui::SeparatorText("From");
        ImGui::Text("This engine was written based on TheCherno's game engine tutorial series.");
        ImGui::Text("Hazel Engine: ");
        ImGui::TextLinkOpenURL("https: // github.com/TheCherno/Hazel");
        ImGui::Text("TheCherno's game engine tutorial series:");
        ImGui::TextLinkOpenURL("https://www.youtube.com/watch?v=JxIZbV_XjAs&list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT");

        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(-1, 0)))
            m_showAboutWindow = false;

        ImGui::End();
    }
}

void BosonLayer::onSettingsPanel() {
    std::string name = "None";
    if (m_hoveredEntity && m_hoveredEntity.hasComponent<TagComponent>()) {
        name = m_hoveredEntity.getComponent<TagComponent>().tag;
    }

    ImGui::Begin("Settings");
    ImGui::Text("hoverd entity: %s", name.c_str());
    ImGui::Text("Statistics");
    SceneRenderer::Statistics stats = m_viewportRenderer ? m_viewportRenderer->getStatistics() : SceneRenderer::Statistics{};
    ImGui::Text("Draw Calls: %d", stats.drawCalls);
    ImGui::Text("Quads: %d", stats.quadCount);
    ImGui::Text("Lines: %d", stats.lineCount);
    ImGui::Text("Circles: %d", stats.circleCount);
    ImGui::Text("Vertices: %d", stats.getTotalVertexCount());
    ImGui::Text("Indices: %d", stats.getTotalIndexCount());

    ImGui::Separator();
    ImGui::Checkbox("showPhysicsColliders", &m_showPhysicsColliders);
    ImGui::Checkbox("showRenderEntities", &m_showRenderEntities);
    ImGui::Checkbox("showSkybox", &m_viewportRenderer->getSceneInfo().showSkybox);
    ImGui::Image((ImTextureID)s_Font->getAtlasTexture()->getRendererID(), {512, 512}, {0, 1}, {1, 0});
    ImGui::End();
}

void BosonLayer::onViewportPanel() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0.117f, 0.117f, 0.117f, 1.0f});
    ImGui::Begin("Viewport");
    auto viewportOffset = ImGui::GetCursorPos();

    m_viewportFocused = ImGui::IsWindowFocused();
    m_viewportHovered = ImGui::IsWindowHovered();

    ImGuiIO &io = ImGui::GetIO();

    if (m_viewportFocused && m_viewportHovered)
        io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
    else
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    Application::get().getImGuiLayer()->blockEvents(!m_viewportFocused || !m_viewportHovered);

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

    m_viewportSize = {viewportPanelSize.x, viewportPanelSize.y};

    // 接收framebuffer到imgui image进行渲染
    uint32_t textureID = m_framebuffer->getColorAttachmentRendererID(0);
    ImGui::Image(reinterpret_cast<void *>(static_cast<intptr_t>(textureID)),
                 ImVec2(viewportPanelSize.x, viewportPanelSize.y),
                 ImVec2(0, 1), ImVec2(1, 0));

    // 接收 .fmscene 拖放
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_SCENE")) {
            const char *path = static_cast<const char *>(payload->Data);
            if (path && path[0]) {
                openScene(std::string(path));
            }
        }
        ImGui::EndDragDropTarget();
    }
    // 接受 .fmproj 拖放
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_PROJECT")) {
            const char *path = static_cast<const char *>(payload->Data);
            if (path && path[0]) {
                openProject(std::string(path));
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImVec2 minBound = ImGui::GetWindowPos();
    minBound.x += viewportOffset.x;
    minBound.y += viewportOffset.y;
    ImVec2 maxBound = ImVec2(minBound.x + viewportPanelSize.x, minBound.y + viewportPanelSize.y);
    m_viewportBounds[0] = {minBound.x, minBound.y};
    m_viewportBounds[1] = {maxBound.x, maxBound.y};

    // ImGuiZmo
    Entity selectedEntity = m_sceneHierarchyPanel.getSelectedEntity();
    if (selectedEntity && m_gizmoType != -1) {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x,
                          ImGui::GetWindowPos().y,
                          viewportPanelSize.x,
                          viewportPanelSize.y);

        // editorCamera
        const glm::mat4 &cameraProjection = m_editorCamera.getProjection();
        const glm::mat4 &cameraView = m_editorCamera.getViewMatrix();

        // Entity
        auto &transformComponent = selectedEntity.getComponent<TransformComponent>();
        glm::mat4 transform = transformComponent.getTransform();

        bool snap = Input::isKeyPressed(KeyCode::LeftAlt);
        float snapValue = 0.5f;
        if (m_gizmoType == ImGuizmo::OPERATION::ROTATE)
            snapValue = 45.0f;

        float snapValues[3] = {snapValue, snapValue, snapValue};
        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                             (ImGuizmo::OPERATION)m_gizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

        if (ImGuizmo::IsUsing()) {
            glm::vec3 translation, rotation, scale;
            Math::decomposeTransform(transform, translation, rotation, scale);

            switch ((ImGuizmo::OPERATION)m_gizmoType) {
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

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void BosonLayer::onOverlayRender() {
    if (m_sceneState == SceneState::Play) {
        Entity camera = m_activeScene->getPrimaryCameraEntity();
        if (!camera)
            return;

        m_viewportRenderer->beginScene(camera.getComponent<CameraComponent>().camera, camera.getComponent<TransformComponent>().getTransform());
    } else {
        m_viewportRenderer->beginScene(m_editorCamera);
    }

    if (m_showPhysicsColliders) {
        // Box Colliders
        {
            auto view = m_activeScene->getAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
            for (auto entity : view) {
                auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

                glm::vec3 translation = tc.translation + glm::vec3(bc2d.offset, 0.001f);
                glm::vec3 scale = tc.scale * glm::vec3(bc2d.size * 2.0f, 1.0f);

                glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.translation) * glm::rotate(glm::mat4(1.0f), tc.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.offset, 0.001f)) * glm::scale(glm::mat4(1.0f), scale);

                m_viewportRenderer->drawRect(transform, glm::vec4(0, 1, 0, 1));
            }
        }

        // Circle Colliders
        {
            auto view = m_activeScene->getAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
            for (auto entity : view) {
                auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

                // 单位 quad 半径 0.5 * scale.x = cc2d.radius * tc.scale.x
                glm::vec3 scale = tc.scale * glm::vec3(cc2d.radius * 2.0f, cc2d.radius * 2.0f, 1.0f);

                // 先平移到实体，再旋转，再平移 offset，再缩放
                glm::mat4 transform =
                    glm::translate(glm::mat4(1.0f), tc.translation) * glm::rotate(glm::mat4(1.0f), tc.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(cc2d.offset, 0.001f)) * glm::scale(glm::mat4(1.0f), scale);

                m_viewportRenderer->drawCircle(transform, glm::vec4(0, 1, 0, 1), 0.1f);
            }
        }
        // Box Sensor
        {
            auto view = m_activeScene->getAllEntitiesWith<TransformComponent, BoxSensor2DComponent>();
            for (auto entity : view) {
                Entity sensor = {entity, m_activeScene.get()};
                auto [tc, bs2d] = view.get<TransformComponent, BoxSensor2DComponent>(entity);
                glm::vec3 translation = tc.translation + glm::vec3(bs2d.offset, 0.001f);
                glm::vec3 scale = tc.scale * glm::vec3(bs2d.size * 2.0f, 1.0f);

                glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.translation) * glm::rotate(glm::mat4(1.0f), tc.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(bs2d.offset, 0.001f)) * glm::scale(glm::mat4(1.0f), scale);

                m_viewportRenderer->drawRect(transform, glm::vec4(0, 1, 1, 1));
            }
        }
    }

    // Draw selected entity outline
    {
        if (Entity selectedEntity = m_sceneHierarchyPanel.getSelectedEntity(); selectedEntity) {
            const TransformComponent &transform = selectedEntity.getComponent<TransformComponent>();
            if (selectedEntity.hasComponent<MeshComponent>()) {
                m_viewportRenderer->SubmitMesh(selectedEntity.getComponent<MeshComponent>(), transform.getTransform(), -1, true);
            }

            m_viewportRenderer->drawRect(transform.getTransform(), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }
    // Renderer2D::endScene();
    m_viewportRenderer->endScene();
}

void BosonLayer::newProject() {
    std::filesystem::path path = FileDialogs::saveFile(
        "Project (*.fmproj)\0*.fmproj\0", "../Boson/assets/project/");
    if (path.empty()) {
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

void BosonLayer::openProject() {
    std::filesystem::path path = FileDialogs::openFile(
        "Project (*.fmproj)\0*.fmproj\0", "../Boson/assets/project/");
    if (path.empty()) {
        Log::Warn("Project Selection directory is empty!");
        return;
    }

    openProject(path);
}

void BosonLayer::openProject(const std::filesystem::path &path) {
    auto project = Project::loadProject(path);
    FERMION_ASSERT(project != nullptr, "Failed to load project!");
    auto lastScene = Project::getActive()->getConfig().startScene;
    if (!lastScene.empty())
        openScene(lastScene);
    m_contentBrowserPanel.setBaseDirectory(Project::getActive()->getConfig().assetDirectory);
}

void BosonLayer::saveProject() {
    saveScene();
    auto &config = Project::getActive()->getConfig();
    config.startScene = m_editorScenePath;
    if (Project::saveActive(Project::getActive()->getProjectPath())) {
        Log::Info("Project save successfully!");
    } else {
        Log::Error("Project save failed!");
    }
}

void BosonLayer::onDuplicateEntity() {
    if (m_sceneState != SceneState::Edit)
        return;

    Entity selectedEntity = m_sceneHierarchyPanel.getSelectedEntity();
    if (selectedEntity) {
        const Entity newEntity = m_editorScene->duplicateEntity(selectedEntity);
        m_sceneHierarchyPanel.setSelectedEntity(newEntity);
    }
}

void BosonLayer::newScene() {
    m_activeScene = std::make_shared<Scene>();
    m_activeScene->onViewportResize(static_cast<uint32_t>(m_viewportSize.x), static_cast<uint32_t>(m_viewportSize.y));
    m_editorScene = m_activeScene;
    m_editorScenePath.clear();
    m_editorSceneHandle = AssetHandle{};
    m_sceneHierarchyPanel.setContext(m_activeScene);
    m_viewportRenderer->setScene(m_activeScene);
}

void BosonLayer::saveSceneAs() {
    std::string defaultDir = "../Boson/assets/scenes/";
    if (auto project = Project::getActive()) {
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

void BosonLayer::saveScene() {
    if (!m_editorScenePath.empty()) {
        SceneSerializer serializer(m_editorScene);
        serializer.serialize(m_editorScenePath);

        auto editorAssets = Project::getEditorAssetManager();
        AssetHandle handle = editorAssets->importAsset(m_editorScenePath);
        if (static_cast<uint64_t>(handle) != 0)
            m_editorSceneHandle = handle;

        Log::Info(std::format("Scene saved successfully! Path: {}",
                              m_editorScenePath.string()));
    } else {
        saveSceneAs();
    }
}

void BosonLayer::openScene() {
    std::string defaultDir = "../Boson/assets/scenes/";
    if (auto project = Project::getActive()) {
        const auto &assetDir = project->getConfig().assetDirectory;
        if (!assetDir.empty())
            defaultDir = assetDir.string();
    }

    std::filesystem::path path = FileDialogs::openFile(
        "Scene (*.fmscene)\0*.fmscene\0", defaultDir);
    if (!path.empty()) {
        openScene(path);
    }
}

void BosonLayer::openScene(const std::filesystem::path &path) {
    if (path.empty()) {
        Log::Warn("openScene called with empty path");
        return;
    }

    if (m_sceneState != SceneState::Edit) {
        onSceneStop();
    }

    auto editorAssets = Project::getEditorAssetManager();
    AssetHandle handle = editorAssets->importAsset(path);
    if (static_cast<uint64_t>(handle) == 0) {
        Log::Error(std::format("Scene open failed (invalid asset handle)! Path: {}",
                               path.string()));
        return;
    }

    auto sceneAsset = editorAssets->getAsset<SceneAsset>(handle);
    if (!sceneAsset || !sceneAsset->scene) {
        Log::Error(std::format("Scene open failed (asset load failed)! Path: {}",
                               path.string()));
        return;
    }

    std::shared_ptr<Scene> newScene = sceneAsset->scene;

    if (newScene) {
        m_editorSceneHandle = handle;
        m_editorScene = newScene;
        m_editorScene->onViewportResize(static_cast<uint32_t>(m_viewportSize.x), static_cast<uint32_t>(m_viewportSize.y));
        m_activeScene = m_editorScene;
        m_editorScenePath = path;
        m_sceneHierarchyPanel.setContext(m_activeScene);
        m_viewportRenderer->setScene(m_activeScene);
        Log::Info(std::format("Scene opened successfully! Path: {}",
                              path.string()));
    } else {
        Log::Error(std::format("Scene open failed! Path: {}",
                               path.string()));
    }
}

void BosonLayer::onScenePlay() {
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

void BosonLayer::onSceneSimulate() {
    if (m_sceneState == SceneState::Play)
        onSceneStop();
    m_sceneState = SceneState::Simulate;
    m_activeScene = Scene::copy(m_editorScene);
    m_activeScene->onSimulationStart();
    m_sceneHierarchyPanel.setEditingEnabled(false);
    m_viewportRenderer->setScene(m_activeScene);
    m_sceneHierarchyPanel.setContext(m_activeScene);
}

void BosonLayer::onSceneStop() {
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
