#pragma once
#include "BosonLayer.hpp"
#include "Fermion.hpp"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

BosonLayer::BosonLayer(const std::string &name) : Layer(name), m_cameraController(1290.0f / 720.0f)
{
}
void BosonLayer::onAttach()
{
    FM_PROFILE_FUNCTION();

    m_checkerboardTexture = Fermion::Texture2D::create("../game/assets/textures/Checkerboard.png");
    m_spriteSheet = Fermion::Texture2D::create("../game/assets/game/RPGpack_sheet_2X.png");

    Fermion::FramebufferSpecification fbSpec;
    fbSpec.width = 1280;
    fbSpec.height = 720;
    fbSpec.attachments = {Fermion::FramebufferTextureFormat::RGBA8, Fermion::FramebufferTextureFormat::DEPTH24STENCIL8};
    m_framebuffer = Fermion::Framebuffer::create(fbSpec);
}
void BosonLayer::onDetach()
{
    FM_PROFILE_FUNCTION();
}
void BosonLayer::onUpdate(Fermion::Timestep dt)
{
    FM_PROFILE_FUNCTION();
    m_cameraController.onUpdate(dt);

    // Ensure framebuffer matches current viewport size before rendering
    if (m_framebuffer)
    {
        const auto &spec = m_framebuffer->getSpecification();
        if (m_viewportSize.x > 0.0f && m_viewportSize.y > 0.0f &&
            (spec.width != static_cast<uint32_t>(m_viewportSize.x) || spec.height != static_cast<uint32_t>(m_viewportSize.y)))
        {
            m_framebuffer->resize(static_cast<uint32_t>(m_viewportSize.x), static_cast<uint32_t>(m_viewportSize.y));
            m_cameraController.onResize(m_viewportSize.x, m_viewportSize.y);
        }
    }

    Fermion::Renderer2D::resetStatistics();
    m_framebuffer->bind();
    Fermion::RenderCommand::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});
    Fermion::RenderCommand::clear();

    {
        FM_PROFILE_SCOPE("Renderer Draw");
        static float rotation = 0.0f;
        rotation += dt * 10.0f;

        Fermion::Renderer2D::beginScene(m_cameraController.getCamera());
        Fermion::Renderer2D::drawQuad(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.8f, 0.8f), glm::vec4(0.8f, 0.2f, 0.3f, 1.0f));
        Fermion::Renderer2D::drawRotatedQuad(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.8f, 0.8f), glm::radians(45.0f), glm::vec4(0.2f, 0.3f, 0.8f, 1.0f));
        Fermion::Renderer2D::drawQuad(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec2(0.5f, 0.75f), m_squareColor);
        Fermion::Renderer2D::drawQuad(glm::vec3(0.0f, 0.0f, -0.1f), glm::vec2(20.0f, 20.0f), m_checkerboardTexture, 10);
        Fermion::Renderer2D::drawRotatedQuad(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::radians(rotation), m_checkerboardTexture, 20);
        Fermion::Renderer2D::endScene();

        Fermion::Renderer2D::beginScene(m_cameraController.getCamera());
        for (float y = -5.0f; y < 5.0f; y += 0.5f)
        {
            for (float x = -5.0f; x < 5.0f; x += 0.5f)
            {
                glm::vec4 color = {(x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f};
                Fermion::Renderer2D::drawQuad({x + 0.25f, y + 0.25f}, glm::vec2(0.45f, 0.45f), color);
            }
        }
        Fermion::Renderer2D::endScene();
    }
    m_framebuffer->unbind();
}

void BosonLayer::onEvent(Fermion::IEvent &event)
{
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
                    Fermion::Engine::get().close();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        ImGui::ShowDemoWindow();
        ImGui::Begin("Settings");
        ImGui::Text("Statistics");
        Fermion::Renderer2D::Satistics stats = Fermion::Renderer2D::getStatistics();
        ImGui::Text("Draw Calls: %d", stats.drawCalls);
        ImGui::Text("Quads: %d", stats.quadCount);
        ImGui::Text("Vertices: %d", stats.getTotalVertexCount());
        ImGui::Text("Indices: %d", stats.getTotalIndexCount());
        ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));

        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::Begin("Viewport");
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (m_viewportSize.x != viewportPanelSize.x || m_viewportSize.y != viewportPanelSize.y)
        {
            m_viewportSize = {viewportPanelSize.x, viewportPanelSize.y};
            m_cameraController.onResize(m_viewportSize.x, m_viewportSize.y);
        } 
        
        // Fermion::Log::Info(std::format("viewportPanelSize: ({0}, {1})", viewportPanelSize.x, viewportPanelSize.y));
        uint32_t textureID = m_framebuffer->getColorAttachmentRendererID();
        ImGui::Image(reinterpret_cast<void *>(static_cast<intptr_t>(textureID)), ImVec2(m_viewportSize.x, m_viewportSize.y), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
        ImGui::PopStyleVar();
    }
}
