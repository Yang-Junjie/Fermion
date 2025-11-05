#pragma once
#include "Fermion.hpp"

#include <imgui.h>
#include "OpenGLShader.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class GameLayer : public Fermion::Layer
{
public:
    GameLayer(const std::string &name = "GameLayer") : Layer(name), m_cameraController(1600.0f / 900.0f)
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
            uniform mat4 u_Transform;
            
            void main() {
                gl_Position = u_ViewProjection* u_Transform * vec4(aPos, 1.0);
                vertexColor = aColor;
            })";

        std::string fragmentShader = R"(
            #version 330 core
            in vec4 vertexColor;
            out vec4 FragColor;
            
            void main() {
                FragColor = vertexColor;
            })";
        m_shader = Fermion::Shader::create("VertexPosColor", vertexShader, fragmentShader);

        m_squareVA = Fermion::VertexArray::create();

        float squareVertices[4 * 5] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f};

        std::shared_ptr<Fermion::VertexBuffer> squareVB = Fermion::VertexBuffer::create(squareVertices, sizeof(squareVertices));
        squareVB->setLayout({{Fermion::ShaderDataType::Float3, "a_Position"},
                             {Fermion::ShaderDataType::Float2, "a_TexCoord"}});
        m_squareVA->addVertexBuffer(squareVB);

        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        std::shared_ptr<Fermion::IndexBuffer> squareIB = Fermion::IndexBuffer::create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
        m_squareVA->setIndexBuffer(squareIB);
        std::string flatColorShaderVertexSrc = R"(
    #version 330 core
    layout (location = 0) in vec3 a_Position;
    
    uniform mat4 u_ViewProjection;
    uniform mat4 u_Transform;
    
    void main() {
        gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
    }
)";

        std::string flatColorShaderFragmentSrc = R"(
			#version 330 core
            out vec4 FragColor;
            uniform vec3 vertexColor;
            void main() {
                FragColor = vec4(vertexColor,1);
            }
		)";

        m_squareShader = Fermion::Shader::create("FlatShader", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

        auto textureShader = m_shaderLibrary.load("game/assets/shaders/Texture.glsl");

        m_Texture = Fermion::Texture2D::create("assets/textures/Checkerboard.png");
        m_logoTexture = Fermion::Texture2D::create("assets/textures/pslogo.png");

        std::dynamic_pointer_cast<Fermion::OpenGLShader>(textureShader)->bind();
        std::dynamic_pointer_cast<Fermion::OpenGLShader>(textureShader)->setInt("u_Texture", 0);
    }
    virtual ~GameLayer() = default;

    virtual void onAttach() override {}
    virtual void onDetach() override {}
    virtual void onUpdate(Fermion::Timestep dt) override
    {

        Fermion::Log::Trace("GameLayer OnUpdate called");
        m_cameraController.onUpdate(dt);

        // if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::W))
        //     m_cameraPosition.y += m_cameraMoveSpeed * dt;
        // if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::S))
        //     m_cameraPosition.y -= m_cameraMoveSpeed * dt;
        // if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::A))
        //     m_cameraPosition.x -= m_cameraMoveSpeed * dt;
        // if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::D))
        //     m_cameraPosition.x += m_cameraMoveSpeed * dt;
        // if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::Q))
        //     m_cameraRotation += m_cameraRotationSpeed * dt;
        // if (Fermion::Input::IsKeyPressed(Fermion::KeyCode::E))
        //     m_cameraRotation -= m_cameraRotationSpeed * dt;

        // m_camera.setPosition(m_cameraPosition);
        // m_camera.setRotation(m_cameraRotation);

        Fermion::RenderCommand::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});
        Fermion::RenderCommand::clear();

        Fermion::Renderer::beginScene(m_cameraController.getCamera());

        // glm::vec4 redColor = glm::vec4(0.8f, 0.3f, 0.2f, 1.0f);
        // glm::vec4 blueColor = glm::vec4(0.2f, 0.3f, 8.0f, 1.0f);
        std::dynamic_pointer_cast<Fermion::OpenGLShader>(m_squareShader)->bind();
        std::dynamic_pointer_cast<Fermion::OpenGLShader>(m_squareShader)->setFloat3("vertexColor", m_squareColor);

        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
        for (int y = 0; y < 20; y++)
        {
            for (int x = 0; x < 20; x++)
            {
                glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
                Fermion::Renderer::submit(m_squareShader, m_squareVA, transform);
            }
        }
        auto textureShader = m_shaderLibrary.get("Texture");
        m_Texture->bind();
        Fermion::Renderer::submit(textureShader, m_squareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
        m_logoTexture->bind();
        Fermion::Renderer::submit(textureShader, m_squareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

        // triangle
        // Fermion::Renderer::submit(m_shader, m_vertexArray);
        Fermion::Renderer::endScene();
    }
    virtual void onEvent(Fermion::IEvent &event) override
    {
        Fermion::Log::Trace("GameLayer OnEvent called: " + event.toString());
        m_cameraController.onEvent(event);
    }
    virtual void onImGuiRender() override
    {
        // ImGui::ShowDemoWindow();
        ImGui::Begin("Settings");
        ImGui::ColorEdit3("Square Color", glm::value_ptr(m_squareColor));
        ImGui::End();
    }

private:
    Fermion::ShaderLibrary m_shaderLibrary;

    std::shared_ptr<Fermion::Shader> m_shader;
    std::shared_ptr<Fermion::VertexArray> m_vertexArray;
    std::shared_ptr<Fermion::VertexBuffer> m_vertexBuffer;
    std::shared_ptr<Fermion::IndexBuffer> m_indexBuffer;

    std::shared_ptr<Fermion::VertexArray> m_squareVA;
    std::shared_ptr<Fermion::Shader> m_squareShader;
    glm::vec3 m_squareColor = {0.2, 0.3, 0.8};

    std::shared_ptr<Fermion::Texture2D> m_Texture, m_logoTexture;

    // Fermion::OrthographicCamera m_camera;
    Fermion::OrthographicCameraController m_cameraController;
    float m_cameraRotation = 0.0f;
    float m_cameraRotationSpeed = 180.0f;
    float m_cameraMoveSpeed = 2.5f;
    glm::vec3 m_cameraPosition = {0, 0, 0};
};
