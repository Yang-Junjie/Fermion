#pragma once
#include "Fermion.hpp"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SandBox2D : public Fermion::Layer
{
public:
    SandBox2D(const std::string &name = "SandBox2D") : Layer(name), m_cameraController(1600.0f / 900.0f)
    {
    }
    virtual ~SandBox2D() = default;

    virtual void onAttach() override
    {
        FM_PROFILE_FUNCTION();

        m_checkerboardTexture = Fermion::Texture2D::create("../assets/textures/Checkerboard.png");
    }
    virtual void onDetach() override
    {
        FM_PROFILE_FUNCTION();
    }
    virtual void onUpdate(Fermion::Timestep dt) override
    {
        FM_PROFILE_FUNCTION();
        m_cameraController.onUpdate(dt);

        Fermion::RenderCommand::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});
        Fermion::RenderCommand::clear();
        {
            FM_PROFILE_SCOPE("Renderer Draw");
            Fermion::Renderer2D::beginScene(m_cameraController.getCamera());
            Fermion::Renderer2D::drawRotatedQuad(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.8f, 0.8f), glm::radians(45.0f), glm::vec4(0.8f, 0.2f, 0.3f, 1.0f));
            Fermion::Renderer2D::drawQuad(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec2(0.5f, 0.75f), m_squareColor);
            Fermion::Renderer2D::drawQuad(glm::vec3(0.0f, 0.0f, -0.1f), glm::vec2(10.0f, 10.0f), m_checkerboardTexture, 10, glm::vec4(1.0f, 0.9f, 0.9f, 1.0f));
            Fermion::Renderer2D::endScene();
        }
    }
    virtual void onEvent(Fermion::IEvent &event) override
    {
        m_cameraController.onEvent(event);
    }
    virtual void onImGuiRender() override
    {
        FM_PROFILE_FUNCTION();

        ImGui::Begin("Settings");
        ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));
        ImGui::End();
    }

private:
    std::shared_ptr<Fermion::VertexArray> m_squareVA;
    std::shared_ptr<Fermion::Shader> m_flatColorShader;
    glm::vec4 m_squareColor = {0.2, 0.3, 0.8, 1.0};

    std::shared_ptr<Fermion::Texture2D> m_checkerboardTexture;

    Fermion::OrthographicCameraController m_cameraController;
};
