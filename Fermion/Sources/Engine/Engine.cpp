﻿#include "Engine/Engine.hpp"
#include "fmpch.hpp"
#include "Core/Timestep.hpp"
#include <GLFW/glfw3.h>


namespace Fermion
{
    Engine* Engine::s_instance = nullptr;
    Engine::Engine()
    {
        s_instance = this;
        WindowProps windowProps; 
        m_window = IWindow::create(windowProps);
        m_window->setEventCallback([this](IEvent &event)
                                   { this->onEvent(event); });
        m_imGuiLayer = std::make_unique<ImGuiLayer>(m_window->getNativeWindow());
        m_imGuiLayerRaw = m_imGuiLayer.get();
        pushOverlay(std::move(m_imGuiLayer));
        
    }

    void Engine::run()
    {
        Log::Info("Engine started!");

        while (m_running)
        {
            
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