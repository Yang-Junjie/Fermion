#pragma once
#include "Core/Layer.hpp"
#include "Core/Log.hpp"
#include <imgui.h>
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/OrthographicCamera.hpp"

#include "Core/Input.hpp"

class GameLayer : public Fermion::Layer
{
public:
    GameLayer(const std::string &name = "GameLayer") : Layer(name), m_camera(-1.6f, 1.6f, -0.9f, 0.9f)
    {
        m_vertexArray = Fermion::VertexArray::create();
        float vertices[] = {
            // positions         // colors
            0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,   // 顶点0: 红色
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // 顶点1: 绿色
            0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f   // 顶点2: 蓝色
        };
        m_vertexBuffer = Fermion::VertexBuffer::create(vertices, sizeof(vertices));
        Fermion::BufferLayout layout = {
            {Fermion::ShaderDataType::Float3, "aPos"},
            {Fermion::ShaderDataType::Float4, "aColor"}};
        m_vertexBuffer->setLayout(layout);
        m_vertexArray->addVertexBuffer(m_vertexBuffer);

        // 索引数据
        uint32_t indices[] = {
            0, 1, 2};

        m_indexBuffer = Fermion::IndexBuffer::create(indices, sizeof(indices) / sizeof(uint32_t));
        m_vertexArray->setIndexBuffer(m_indexBuffer);

        std::string vertexShader = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec4 aColor;
    
    out vec4 vertexColor;
    uniform mat4 u_ViewProjection;
    
    void main() {
        gl_Position = u_ViewProjection * vec4(aPos, 1.0);
        vertexColor = aColor;
    }
)";

        std::string fragmentShader = R"(
    #version 330 core
    in vec4 vertexColor;
    out vec4 FragColor;
    
    void main() {
        FragColor = vertexColor;
    }
)";
        m_shader = std::make_shared<Fermion::OpenGLShader>(vertexShader, fragmentShader);
        m_camera.setRotation(45.0f);
    }
    virtual ~GameLayer() = default;

    virtual void onAttach() override {}
    virtual void onDetach() override {}
    virtual void onUpdate(Fermion::Timestep dt) override
    {

        Fermion::Log::Trace("GameLayer OnUpdate called");

        if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::Up))
        {
            m_camera.setPosition(m_camera.getPosition() + glm::vec3(0.0f, 0.01f, 0.0f));
        }
        if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::Down))
        {
            m_camera.setPosition(m_camera.getPosition() + glm::vec3(0.0f, -0.01f, 0.0f));
        }
        if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::Left))
        {
            m_camera.setPosition(m_camera.getPosition() + glm::vec3(-0.01f, 0.0f, 0.0f));
        }
        if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::Right))
        {
            m_camera.setPosition(m_camera.getPosition() + glm::vec3(0.01f, 0.0f, 0.0f));
        }
        if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::Q))
        {
            m_camera.setRotation(m_camera.getRotation() + 1.0f);
        }
        if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::R))
        {
            m_camera.setRotation(m_camera.getRotation() - 1.0f);
        }

        Fermion::RenderCommand::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});
        Fermion::RenderCommand::clear();

        Fermion::Renderer::beginScene(m_camera);
        Fermion::Renderer::submit(m_shader, m_vertexArray);
        Fermion::Renderer::endScene();
    }
    virtual void onEvent(Fermion::IEvent &event) override
    {
        Fermion::Log::Trace("GameLayer OnEvent called: " + event.toString());
    }
    virtual void onImGuiRender() override
    {
        ImGui::ShowDemoWindow();
    }

private:
    std::shared_ptr<Fermion::OpenGLShader> m_shader;
    std::shared_ptr<Fermion::VertexArray> m_vertexArray;
    std::shared_ptr<Fermion::VertexBuffer> m_vertexBuffer;
    std::shared_ptr<Fermion::IndexBuffer> m_indexBuffer;

    Fermion::OrthographicCamera m_camera;
};
