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
        m_squareVA = Fermion::VertexArray::create();

        float squareVertices[4 * 3] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f};

        std::shared_ptr<Fermion::VertexBuffer> squareVB = Fermion::VertexBuffer::create(squareVertices, sizeof(squareVertices));
        squareVB->setLayout({
            {Fermion::ShaderDataType::Float3, "a_Position"},
        });
        m_squareVA->addVertexBuffer(squareVB);

        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        std::shared_ptr<Fermion::IndexBuffer> squareIB = Fermion::IndexBuffer::create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
        m_squareVA->setIndexBuffer(squareIB);

        m_flatColorShader = Fermion::Shader::create("../game/assets/shaders/FlatColor.glsl");
    }
    virtual void onDetach() override {}
    virtual void onUpdate(Fermion::Timestep dt) override
    {
        m_cameraController.onUpdate(dt);

        Fermion::RenderCommand::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});
        Fermion::RenderCommand::clear();

        Fermion::Renderer2D::beginScene(m_cameraController.getCamera());
        Fermion::Renderer2D::drawQuad(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.8f, 0.8f),  glm::vec4(0.8f, 0.2f, 0.3f, 1.0f));
        Fermion::Renderer2D::drawQuad(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec2(0.5f, 0.75f), m_squareColor);
        Fermion::Renderer2D::endScene();
    }
    virtual void onEvent(Fermion::IEvent &event) override
    {
       
    }
    virtual void onImGuiRender() override
    {
        ImGui::Begin("Settings");
        ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));
        ImGui::End();
    }

private:
   
    std::shared_ptr<Fermion::VertexArray> m_squareVA;
    std::shared_ptr<Fermion::Shader> m_flatColorShader;
    glm::vec4 m_squareColor = {0.2, 0.3, 0.8, 1.0};

    Fermion::OrthographicCameraController m_cameraController;
};
