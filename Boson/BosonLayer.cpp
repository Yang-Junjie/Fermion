#pragma once
#include "BosonLayer.hpp"
#include "Fermion.hpp"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
namespace Fermion
{
    class CameraController : public ScriptableEntity
    {
    public:
        void onCreate()
        {
            auto &translation = getComponent<TransformComponent>().translation;
            translation.x = rand() % 10 - 5.0f;
        }
        void onDestroy()
        {
        }
        void onUpdate(Timestep ts)
        {
            auto &translation = getComponent<TransformComponent>().translation;
            float speed = 5.0f;
            if (Input::isKeyPressed(KeyCode::A))
                translation.x += speed * ts;
            if (Input::isKeyPressed(KeyCode::D))
                translation.x -= speed * ts;
            if (Input::isKeyPressed(KeyCode::W))
                translation.y -= speed * ts;
            if (Input::isKeyPressed(KeyCode::S))
                translation.y += speed * ts;
        }
    };
    BosonLayer::BosonLayer(const std::string &name) : Layer(name), m_cameraController(1280.0f / 720.0f)
    {
    }
    void BosonLayer::onAttach()
    {
        FM_PROFILE_FUNCTION();

        FramebufferSpecification fbSpec;
        fbSpec.width = 1280;
        fbSpec.height = 720;
        fbSpec.attachments = {FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH24STENCIL8};

        m_framebuffer = Framebuffer::create(fbSpec);

        m_activeScene = std::make_shared<Scene>();
        auto greenSquare = m_activeScene->createEntity("green square");
        greenSquare.addComponent<SpriteRendererComponent>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

        auto redSquare = m_activeScene->createEntity("red square");
        redSquare.addComponent<SpriteRendererComponent>(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        m_cameraEntity = m_activeScene->createEntity("camera");
        m_cameraEntity.addComponent<CameraComponent>();

        m_secondCameraEntity = m_activeScene->createEntity("camera2");
        auto &cc = m_secondCameraEntity.addComponent<CameraComponent>();
        cc.primary = false;

        m_cameraEntity.addComponent<NativeScriptComponent>().bind<CameraController>();
        m_secondCameraEntity.addComponent<NativeScriptComponent>().bind<CameraController>();

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
            m_activeScene->onViewportResize(m_viewportSize.x, m_viewportSize.y);
        }

        if (m_viewportFocused)
            m_cameraController.onUpdate(dt);

        Renderer2D::resetStatistics();
        m_framebuffer->bind();
        RenderCommand::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});
        RenderCommand::clear();

        // Renderer2D::beginScene(m_cameraController.getCamera());

        m_activeScene->onUpdate(dt);

        // Renderer2D::endScene();

        m_framebuffer->unbind();
    }

    void BosonLayer::onEvent(IEvent &event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowResizeEvent>([](WindowResizeEvent &e)
                                               { return true; });
        if (!event.handled)
            m_cameraController.onEvent(event);
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

            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            // because it would be confusing to have two docking targets within each others.
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
                if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
                {
                    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                }

                // Show demo options and help
                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::BeginMenu("File"))
                    {
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

            // Statistics
            {
                ImGui::Begin("Settings");
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

                m_viewportFocused = ImGui::IsWindowFocused();
                m_viewportHovered = ImGui::IsWindowHovered();
                Engine::get().getImGuiLayer()->blockEvents(!m_viewportFocused || !m_viewportHovered);

                ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

                m_viewportSize = {viewportPanelSize.x, viewportPanelSize.y};

                uint32_t textureID = m_framebuffer->getColorAttachmentRendererID();
                ImGui::Image(reinterpret_cast<void *>(static_cast<intptr_t>(textureID)), ImVec2(viewportPanelSize.x, viewportPanelSize.y), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::End();
                ImGui::PopStyleVar();
            }
        }
    }

}
