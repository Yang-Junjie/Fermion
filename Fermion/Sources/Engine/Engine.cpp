#include "Engine/Engine.hpp"
#include "fmpch.hpp"
#include "Core/Timestep.hpp"
#include "GLFW/glfw3.h"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Renderer.hpp"

namespace Fermion
{
    Engine::Engine()
    {
        WindowProps windowProps;
        m_window = IWindow::create(windowProps);
        m_window->setEventCallback([this](IEvent &event)
                                   { this->onEvent(event); });
        m_imGuiLayer = std::make_unique<ImGuiLayer>(m_window->getNativeWindow());
        m_imGuiLayerRaw = m_imGuiLayer.get();
        pushOverlay(std::move(m_imGuiLayer));
        m_vertexArray = VertexArray::create();
        float vertices[] = {
            // positions         // colors
            0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,   // 顶点0: 红色
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // 顶点1: 绿色
            0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f   // 顶点2: 蓝色
        };
        m_vertexBuffer = VertexBuffer::create(vertices, sizeof(vertices));
        BufferLayout layout = {
            {ShaderDataType::Float3, "aPos"},
            {ShaderDataType::Float4, "aColor"}};
        m_vertexBuffer->setLayout(layout);
        m_vertexArray->addVertexBuffer(m_vertexBuffer);

        // 索引数据
        uint32_t indices[] = {
            0, 1, 2};

        m_indexBuffer = IndexBuffer::create(indices, sizeof(indices) / sizeof(uint32_t));
        m_vertexArray->setIndexBuffer(m_indexBuffer);

        std::string vertexShader = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec4 aColor;
    
    out vec4 vertexColor;
    
    void main() {
        gl_Position = vec4(aPos, 1.0);
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
        m_shader = std::make_shared<OpenGLShader>(vertexShader, fragmentShader);
    }

    void Engine::run()
    {
        Log::Info("Engine started!");

        while (m_running)
        {
            RenderCommand::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});
            RenderCommand::clear();
            m_shader->bind();
            Renderer::submit(m_vertexArray);
            Renderer::beginScene();

            Renderer::endScene();
            float time = static_cast<float>(glfwGetTime()); // TODO :GLFE TIMER
            Timestep timestep = time - m_lastFrameTime;
            m_lastFrameTime = time;

            // if (timestep.GetSeconds() <= 0.0f)
            //     timestep = 1.0f / 60.0f;
            for (auto &layer : m_layerStack)
                layer->onUpdate(timestep);

            m_imGuiLayerRaw->begin();
            for (auto &layer : m_layerStack)
                layer->onImGuiRender();
            m_imGuiLayerRaw->end();
            m_window->OnUpdate();
        }
    }

    void Engine::pushLayer(std::unique_ptr<Layer> layer)
    {

        layer->onAttach();
        m_layerStack.pushLayer(std::move(layer));
    }
    void Engine::pushOverlay(std::unique_ptr<Layer> overlay)
    {
        overlay->onAttach();
        m_layerStack.pushOverlay(std::move(overlay));
    }

    void Engine::onEvent(IEvent &event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent &e)
                                               { return this->onWindowResize(e); });
        dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent &e)
                                              { return this->onWindowClose(e); });
        // 从后往前遍历 LayerStack，优先分发给最上层的 Layer
        for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it)
        {
            if (event.handled)
                break;
            (*it)->onEvent(event);
        }
    }

    bool Engine::onWindowResize(WindowResizeEvent &event)
    {
        if (event.getWidth() == 0 || event.getHeight() == 0)
        {
            m_minimized = true;
            return false;
        }
        m_minimized = false;
        Log::Info("Window resized to " + std::to_string(event.getWidth()) + "x" + std::to_string(event.getHeight()));
        return false;
    }

    bool Engine::onWindowClose(WindowCloseEvent &event)
    {
        m_running = false;
        Log::Info("Window closed");
        return true;
    }
}