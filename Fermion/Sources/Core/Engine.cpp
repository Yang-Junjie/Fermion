#include "Core/Engine.hpp"
#include "Core/Timestep.hpp"
#include <GLFW/glfw3.h>

namespace Fermion
{
    Engine *Engine::s_instance = nullptr;
    Engine::Engine()
    {
        FM_PROFILE_FUNCTION();
        s_instance = this;
        WindowProps windowProps;
        m_window = IWindow::create(windowProps);
        m_window->setEventCallback([this](IEvent &event)
                                   { this->onEvent(event); });
        m_window->setVSync(true);
        Renderer::init();
        m_imGuiLayer = std::make_unique<ImGuiLayer>(m_window->getNativeWindow());
        m_imGuiLayerRaw = m_imGuiLayer.get();
        pushOverlay(std::move(m_imGuiLayer));
    }

    void Engine::run()
    {
        FM_PROFILE_FUNCTION();
        Log::Info("Engine started!");

        while (m_running)
        {
            FM_PROFILE_SCOPE("RunLoop");
            float time = static_cast<float>(glfwGetTime()); // TODO :GLFE TIMER
            Timestep timestep = time - m_lastFrameTime;
            m_lastFrameTime = time;

            if (!m_minimized)
            {
                {
                    FM_PROFILE_SCOPE("LayerStack OnUpdate");
                    for (auto &layer : m_layerStack)
                        layer->onUpdate(timestep);
                }
                m_imGuiLayerRaw->begin();
                {
                    FM_PROFILE_SCOPE("LayerStack OnImGuiRender");
                    for (auto &layer : m_layerStack)
                        layer->onImGuiRender();
                }
                m_imGuiLayerRaw->end();
            }
            m_window->OnUpdate();
        }
    }

    void Engine::pushLayer(std::unique_ptr<Layer> layer)
    {
        FM_PROFILE_FUNCTION();
        layer->onAttach();
        m_layerStack.pushLayer(std::move(layer));
    }
    void Engine::pushOverlay(std::unique_ptr<Layer> overlay)
    {
        FM_PROFILE_FUNCTION();
        overlay->onAttach();
        m_layerStack.pushOverlay(std::move(overlay));
    }

    void Engine::onEvent(IEvent &event)
    {
        FM_PROFILE_FUNCTION();
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
        FM_PROFILE_FUNCTION();
        if (event.getWidth() == 0 || event.getHeight() == 0)
        {
            m_minimized = true;
            return false;
        }
        m_minimized = false;
        Renderer::onWindowResize(event.getWidth(), event.getHeight());
        // Log::Info("Window resized to " + std::to_string(event.getWidth()) + "x" + std::to_string(event.getHeight()));
        Log::Info(std::format("Window resized to {}x{}", event.getWidth(), event.getHeight()));
        return false;
    }

    bool Engine::onWindowClose(WindowCloseEvent &event)
    {
        m_running = false;
        Log::Info("Window closed");
        return true;
    }
}