#pragma once
#include "BosonLayer.hpp"
#include "Fermion.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Utils/PlatformUtils.hpp"

#include "Math/Math.hpp"

#include <imgui.h>
#include <ImGuizmo.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
namespace Fermion
{

    BosonLayer::BosonLayer(const std::string &name) : Layer(name), m_cameraController(1280.0f / 720.0f)
    {
    }
    void BosonLayer::onAttach()
    {
        FM_PROFILE_FUNCTION();

        m_iconPlay = Texture2D::create("../Boson/Resources/Icons/PlayButton.png");
        m_iconStop = Texture2D::create("../Boson/Resources/Icons/StopButton.png");
        FramebufferSpecification fbSpec;

        fbSpec.width = 1280;
        fbSpec.height = 720;
        fbSpec.attachments = {FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::DEPTH24STENCIL8};

        m_framebuffer = Framebuffer::create(fbSpec);

        m_activeScene = std::make_shared<Scene>();

        m_editorCamera = EditorCamera(45.0f, (float)fbSpec.width / (float)fbSpec.height, 0.1f, 1000.0f);
        m_sceneHierarchyPanel.setContext(m_activeScene);
    }
    void BosonLayer::onDetach()
    {
        FM_PROFILE_FUNCTION();
    }
    void BosonLayer::onUpdate(Timestep dt)
    {
        FM_PROFILE_FUNCTION();

        // Resize
        if (FramebufferSpecification spec = m_framebuffer->getSpecification();
            m_viewportSize.x > 0 && m_viewportSize.y > 0 && (spec.width != m_viewportSize.x || spec.height != m_viewportSize.y))
        {
            m_framebuffer->resize(m_viewportSize.x, m_viewportSize.y);
            m_cameraController.onResize(m_viewportSize.x, m_viewportSize.y);
            m_editorCamera.setViewportSize(m_viewportSize.x, m_viewportSize.y);
            m_activeScene->onViewportResize(m_viewportSize.x, m_viewportSize.y);
        }

        Renderer2D::resetStatistics();
        m_framebuffer->bind();
        RenderCommand::setClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::clear();
        m_framebuffer->clearAttachment(1, -1);

        if (m_sceneState == SceneState::Play)
        {
            m_activeScene->onUpdateRuntime(dt);
            m_sceneHierarchyPanel.setSelectedEntity({});
        }
        else
        {
            if (m_viewportFocused)
            {
                m_cameraController.onUpdate(dt);
            }
            m_editorCamera.onUpdate(dt);

            m_activeScene->onUpdateEditor(dt, m_editorCamera);
        }

        auto [mx, my] = ImGui::GetMousePos();
        mx -= (float)m_viewportBounds[0].x;
        my -= (float)m_viewportBounds[0].y;
        glm::vec2 viewportSize = m_viewportBounds[1] - m_viewportBounds[0];
        my = viewportSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;
        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
            int pixelData = m_framebuffer->readPixel(1, mouseX, mouseY);
            // Log::Warn(std::format("pixel data: {0}", pixelData));
            m_hoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_activeScene.get());
        }

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

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
            {
                const ImGuiViewport *viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
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
                style.WindowMinSize.x = 370.0f;
                if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
                {
                    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                }
                style.WindowMinSize.x = minWinSizeX;
                // Show demo options and help
                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::BeginMenu("File"))
                    {
                        if (ImGui::MenuItem("New", "Ctrl+N"))
                        {
                            newScene();
                        }

                        if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                        {
                            saveScene();
                        }
                        if (ImGui::MenuItem("Open...", "Ctrl+O"))
                        {
                            openScene();
                        }
                        if (ImGui::MenuItem("Exit"))
                            Engine::get().close();

                        ImGui::EndMenu();
                    }

                    ImGui::EndMenuBar();
                }
                ImGui::End();
            }
            // Scene Hierarchy

            m_sceneHierarchyPanel.onImGuiRender();

            m_contentBrowserPanel.onImGuiRender();
            UIToolbar();
            // Statistics
            {
                std::string name = "None";
                if (m_hoveredEntity)
                {
                    name = m_hoveredEntity.getComponent<TagComponent>().tag;
                }

                ImGui::Begin("Settings");
                ImGui::Text("hoverd entity: %s", name.c_str());
                ImGui::Text("Statistics");
                Renderer2D::Satistics stats = Renderer2D::getStatistics();
                ImGui::Text("Draw Calls: %d", stats.drawCalls);
                ImGui::Text("Quads: %d", stats.quadCount);
                ImGui::Text("Vertices: %d", stats.getTotalVertexCount());
                ImGui::Text("Indices: %d", stats.getTotalIndexCount());

                ImGui::End();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

            // Viewport
            {
                ImGui::Begin("Viewport");
                auto viewportOffset = ImGui::GetCursorPos(); // include tab bar (local to window)

                m_viewportFocused = ImGui::IsWindowFocused();
                m_viewportHovered = ImGui::IsWindowHovered();
                Engine::get().getImGuiLayer()->blockEvents(!m_viewportFocused && !m_viewportHovered);

                ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

                m_viewportSize = {viewportPanelSize.x, viewportPanelSize.y};

                uint32_t textureID = m_framebuffer->getColorAttachmentRendererID(0);
                ImGui::Image(reinterpret_cast<void *>(static_cast<intptr_t>(textureID)),
                             ImVec2(viewportPanelSize.x, viewportPanelSize.y),
                             ImVec2(0, 1), ImVec2(1, 0));
                // 接收 .fermion 拖放
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

                ImVec2 minBound = ImGui::GetWindowPos();
                minBound.x += viewportOffset.x;
                minBound.y += viewportOffset.y;
                ImVec2 maxBound = ImVec2(minBound.x + viewportPanelSize.x, minBound.y + viewportPanelSize.y);
                m_viewportBounds[0] = {minBound.x, minBound.y};
                m_viewportBounds[1] = {maxBound.x, maxBound.y};

                // ImGuiZmo
                Entity selectedEntity = m_sceneHierarchyPanel.getSelectedEntity();
                if (selectedEntity && m_gizmoType != -1)
                {
                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::SetDrawlist();
                    ImGuizmo::SetRect(ImGui::GetWindowPos().x,
                                      ImGui::GetWindowPos().y,
                                      viewportPanelSize.x,
                                      viewportPanelSize.y);

                    // Camera entity
                    // auto cameraEntity = m_activeScene->getPrimaryCameraEntity();
                    // const auto &camera = cameraEntity.getComponent<CameraComponent>().camera;
                    // const glm::mat4 &cameraProjection = camera.getProjection();
                    // const glm::mat4 &cameraView = glm::inverse(cameraEntity.getComponent<TransformComponent>().getTransform());

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

                    if (ImGuizmo::IsUsing())
                    {
                        glm::vec3 translation, rotation, scale;
                        Math::decomposeTransform(transform, translation, rotation, scale);

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

                ImGui::End();
                ImGui::PopStyleVar();
            }
        }
    }
    void BosonLayer::UIToolbar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto &colors = ImGui::GetStyle().Colors;
        const auto &buttonHovered = colors[ImGuiCol_ButtonHovered];
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
        const auto &buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

        ImGui::Begin("toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        std::shared_ptr<Texture2D> icon = m_sceneState == SceneState::Edit ? m_iconPlay : m_iconStop;
        bool toolbarEnabled = (bool)m_activeScene;

        ImVec4 tintColor = ImVec4(1, 1, 1, 1);
        if (!toolbarEnabled)
            tintColor.w = 0.5f;

        float size = ImGui::GetWindowHeight() - 5.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

        if (!toolbarEnabled)
            ImGui::BeginDisabled();

        if (ImGui::ImageButton("##toolbar_btn",
                               ImTextureRef(static_cast<ImTextureID>(icon->getRendererID())),
                               ImVec2(size, size),
                               ImVec2(0, 1), ImVec2(1, 0),
                               ImVec4(0, 0, 0, 0), tintColor))
        {
            if (m_sceneState == SceneState::Edit)
                onScenePlay();
            else if (m_sceneState == SceneState::Play)
                onSceneStop();
        }

        if (!toolbarEnabled)
            ImGui::EndDisabled();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::End();
    }
    void BosonLayer::onScenePlay()
    {
        m_sceneState = SceneState::Play;
        m_sceneHierarchyPanel.setEditingEnabled(false);
    }
    void BosonLayer::onSceneStop()
    {
        m_sceneState = SceneState::Edit;
        m_sceneHierarchyPanel.setEditingEnabled(true);
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
        m_cameraController.onEvent(event);
        m_editorCamera.onEvent(event);
    }

    bool BosonLayer::onKeyPressedEvent(KeyPressedEvent &e)
    {
        if (e.isRepeat())
        {
            return false;
        }

        bool control = Input::isKeyPressed(KeyCode::LeftControl) || Input::isKeyPressed(KeyCode::RightControl);
        bool shift = Input::isKeyPressed(KeyCode::LeftShift) || Input::isKeyPressed(KeyCode::RightShift);

        switch (e.getKeyCode())
        {
        case KeyCode::S:
            if (control && shift)
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
            m_gizmoType = ImGuizmo::OPERATION::SCALE;
            break;
        }

        return false;
    }
    bool BosonLayer::onMouseButtonPressedEvent(MouseButtonPressedEvent &e)
    {
        if (e.getMouseButton() == MouseCode::Left)
        {
            if (m_viewportHovered && !ImGuizmo::IsOver())
                m_sceneHierarchyPanel.setSelectedEntity(m_hoveredEntity);
        }
        return false;
    }
    void BosonLayer::newScene()
    {
        m_activeScene = std::make_shared<Scene>();
        m_activeScene->onViewportResize(static_cast<uint32_t>(m_viewportSize.x), static_cast<uint32_t>(m_viewportSize.y));
        m_sceneHierarchyPanel.setContext(m_activeScene);
    }
    void BosonLayer::saveScene()
    {
        std::string path = FileDialogs::saveFile("Scene (*.fermion)\0*.fermion\0", "../Boson/assets/scenes/");
        if (!path.empty())
        {
            SceneSerializer serializer(m_activeScene);
            serializer.serialize(path);
        }
    }
    void BosonLayer::openScene()
    {
        std::string path = FileDialogs::openFile("Scene (*.fermion)\0*.fermion\0", "../Boson/assets/scenes/");
        if (!path.empty())
        {
            openScene(path);
        }
    }
    void BosonLayer::openScene(const std::string &path)
    {
        m_activeScene = std::make_shared<Scene>();
        m_activeScene->onViewportResize(static_cast<uint32_t>(m_viewportSize.x), static_cast<uint32_t>(m_viewportSize.y));
        m_sceneHierarchyPanel.setContext(m_activeScene);

        SceneSerializer serializer(m_activeScene);
        serializer.deserialize(path);
    }

}
