#pragma once
#include "BosonLayer.hpp"
#include "Fermion.hpp"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace Fermion
{
    BosonLayer::BosonLayer(const std::string &name) : Layer(name), m_cameraController(1280.0f / 720.0f)
    {
    }
    void BosonLayer::onAttach()
    {
        FM_PROFILE_FUNCTION();

        m_checkerboardTexture = Texture2D::create("../game/assets/textures/Checkerboard.png");
        m_spriteSheet = Texture2D::create("../game/assets/game/RPGpack_sheet_2X.png");

        FramebufferSpecification fbSpec;
        fbSpec.width = 1280;
        fbSpec.height = 720;
        fbSpec.attachments = {FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH24STENCIL8};
        m_framebuffer = Framebuffer::create(fbSpec);

        m_activeScene = std::make_shared<Scene>();
        auto squre = m_activeScene->createEntity();
        m_squareEntity = squre;
        m_activeScene->getRegistry().emplace<TransformComponent>(squre);
        m_activeScene->getRegistry().emplace<SpriteRendererComponent>(squre, m_squareColor);
    }
    void BosonLayer::onDetach()
    {
        FM_PROFILE_FUNCTION();
    }
    void BosonLayer::onUpdate(Timestep dt)
    {
        FM_PROFILE_FUNCTION();
        applyPendingViewportResize();

        if (m_viewportFocused)
            m_cameraController.onUpdate(dt);

        Renderer2D::resetStatistics();
        m_framebuffer->bind();
        RenderCommand::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});
        RenderCommand::clear();

        Renderer2D::beginScene(m_cameraController.getCamera());

        m_activeScene->onUpdate(dt);

        Renderer2D::endScene();

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

            ImGui::ShowDemoWindow();
            ImGui::Begin("Settings");
            ImGui::Text("Statistics");
            Renderer2D::Satistics stats = Renderer2D::getStatistics();
            ImGui::Text("Draw Calls: %d", stats.drawCalls);
            ImGui::Text("Quads: %d", stats.quadCount);
            ImGui::Text("Vertices: %d", stats.getTotalVertexCount());
            ImGui::Text("Indices: %d", stats.getTotalIndexCount());

            auto& m_squareColor = m_activeScene->getRegistry().get<SpriteRendererComponent>(m_squareEntity).color;
            ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));

            ImGui::End();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
            ImGui::Begin("Viewport");

            m_viewportFocused = ImGui::IsWindowFocused();
            m_viewportHovered = ImGui::IsWindowHovered();
            Engine::get().getImGuiLayer()->blockEvents(!m_viewportFocused || !m_viewportHovered);

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            setViewportSize({viewportPanelSize.x, viewportPanelSize.y});

            uint32_t textureID = m_framebuffer->getColorAttachmentRendererID();
            ImGui::Image(reinterpret_cast<void *>(static_cast<intptr_t>(textureID)), ImVec2(viewportPanelSize.x, viewportPanelSize.y), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::End();
            ImGui::PopStyleVar();
        }
    }

    void BosonLayer::setViewportSize(const glm::vec2 &newSize)
    {
        if (newSize.x <= 0.0f || newSize.y <= 0.0f)
            return;
        // Defer real resize to beginning of next onUpdate
        if (m_viewportSize == newSize && !m_hasPendingViewportResize)
            return;

        m_pendingViewportSize = newSize;
        m_hasPendingViewportResize = true;
    }

    void BosonLayer::applyPendingViewportResize()
    {
        if (!m_hasPendingViewportResize)
            return;

        m_viewportSize = m_pendingViewportSize;

        if (m_framebuffer)
        {
            const auto &spec = m_framebuffer->getSpecification();
            if (spec.width != static_cast<uint32_t>(m_viewportSize.x) ||
                spec.height != static_cast<uint32_t>(m_viewportSize.y))
            {
                m_framebuffer->resize(static_cast<uint32_t>(m_viewportSize.x),
                                      static_cast<uint32_t>(m_viewportSize.y));
            }
        }
        m_cameraController.onResize(m_viewportSize.x, m_viewportSize.y);
        m_hasPendingViewportResize = false;
    }
}
