#pragma once
#include "Fermion.hpp"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SandBox2D : public Fermion::Layer
{
public:
    SandBox2D(const std::string &name = "SandBox2D") : Layer(name), m_cameraController(1290.0f / 720.0f)
    {
    }
    virtual ~SandBox2D() = default;

    virtual void onAttach() override
    {
        FM_PROFILE_FUNCTION();

        m_checkerboardTexture = Fermion::Texture2D::create("../game/assets/textures/Checkerboard.png");
        m_spriteSheet = Fermion::Texture2D::create("../game/assets/game/RPGpack_sheet_2X.png");

        // m_textureStairs = Fermion::SubTexture2D::createFromCoords(m_spriteSheet, {7, 6}, {128, 128});
        // m_textureBarrel = Fermion::SubTexture2D::createFromCoords(m_spriteSheet, {8, 2}, {128, 128});
        // m_textureTree = Fermion::SubTexture2D::createFromCoords(m_spriteSheet, {2, 1}, {128, 128}, {1, 2});
    }
    virtual void onDetach() override
    {
        FM_PROFILE_FUNCTION();
    }
    virtual void onUpdate(Fermion::Timestep dt) override
    {
        FM_PROFILE_FUNCTION();
        m_cameraController.onUpdate(dt);

        Fermion::Renderer2D::resetStatistics();

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
            for (float y = -5.0f; y < 5.0f; y += 0.1f)
            {
                for (float x = -5.0f; x < 5.0f; x += 0.1f)
                {
                    glm::vec4 color = {(x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f};
                    Fermion::Renderer2D::drawQuad({x + 0.25f, y + 0.25f}, glm::vec2(0.45f, 0.45f), color);
                }
            }
            Fermion::Renderer2D::endScene();
        }

        // Fermion::Renderer2D::beginScene(m_cameraController.getCamera());
        // Fermion::Renderer2D::drawQuad(glm::vec3(0.0f, 0.0f, -0.1f), glm::vec2(1.0f, 1.0f), m_textureStairs);
        // Fermion::Renderer2D::drawQuad(glm::vec3(2.0f, -2.0f, -0.1f), glm::vec2(1.0f, 1.0f), m_textureBarrel);
        // Fermion::Renderer2D::drawQuad(glm::vec3(0.0f, -2.0f, -0.1f), glm::vec2(1.0f, 2.0f), m_textureTree);
        // Fermion::Renderer2D::endScene();
    }

    virtual void onEvent(Fermion::IEvent &event) override
    {
        m_cameraController.onEvent(event);
    }

    virtual void onImGuiRender() override
    {
        FM_PROFILE_FUNCTION();

        ImGui::Begin("Settings");
        ImGui::Text("Statistics");
        Fermion::Renderer2D::Satistics stats = Fermion::Renderer2D::getStatistics();
        ImGui::Text("Draw Calls: %d", stats.drawCalls);
        ImGui::Text("Quads: %d", stats.quadCount);
        ImGui::Text("Vertices: %d", stats.getTotalVertexCount());
        ImGui::Text("Indices: %d", stats.getTotalIndexCount());
        ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));

        ImGui::End();
    }

private:
    std::shared_ptr<Fermion::VertexArray> m_squareVA;
    std::shared_ptr<Fermion::Shader> m_flatColorShader;
    glm::vec4 m_squareColor = {0.2, 0.3, 0.8, 1.0};

    std::shared_ptr<Fermion::Texture2D> m_checkerboardTexture;
    std::shared_ptr<Fermion::Texture2D> m_spriteSheet;
    std::shared_ptr<Fermion::SubTexture2D> m_textureStairs;
    std::shared_ptr<Fermion::SubTexture2D> m_textureBarrel;
    std::shared_ptr<Fermion::SubTexture2D> m_textureTree;

    Fermion::OrthographicCameraController m_cameraController;
};
