#include "Engine/Engine.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#ifdef USE_SFML_BACKEND
#include "SFMLWindow.hpp"
#include "SFMLRenderer.hpp"
#elif defined(USE_SDL_BACKEND)
#include "SDLWindow.h"
#include "SDLRenderer.h"
#elif defined(USE_OPENGL_BACKEND)
#include "OpenGLWindow.h"
#include "OpenGLRenderer.h"
#elif defined(USE_VULKAN_BACKEND)
#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#else
#error "No backend specified!"
#endif

#include <thread>
#include <chrono>

namespace Fermion
{
    Engine::Engine()
    {

        WindowProps windowProps;
#ifdef USE_SFML_BACKEND
        auto sfmlWindow = std::make_unique<SFMLWindow>(windowProps);
        SFMLWindow *sfmlWin = sfmlWindow.get();
        m_renderer = std::make_unique<SFMLRenderer>(sfmlWin->get());
        m_window = std::move(sfmlWindow);
        if (ImGui::SFML::Init(sfmlWin->get()))
        {
            Log::Info("ImGui initialized!");
        }
        else
        {
            Log::Debug("ImGui failed to initialize!");
        }
#endif
        m_window->setEventCallback([this](IEvent &event)
                                   { this->onEvent(event); });
    }

    void Engine::run()
    {
        Log::Info("Engine started!");
        init();
        while (m_running && m_window->isOpen())
        {
            // m_window->pollEvents();
            m_window->clear();

            for (auto &layer : m_layerStack)
                layer->OnUpdate();

            m_window->onUpdate();
        }
         ImGui::SFML::Shutdown();
    }

    void Engine::pushLayer(std::unique_ptr<Layer> layer)
    {
        layer->setRenderer(m_renderer.get());
        layer->OnAttach();
        m_layerStack.pushLayer(std::move(layer));
    }
    void Engine::pushOverlay(std::unique_ptr<Layer> overlay)
    {
        overlay->setRenderer(m_renderer.get());
        overlay->OnAttach();
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
            if (event.Handled)
                break;
            (*it)->OnEvent(event);
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
        Log::Info("SFML Window closed");
        return true;
    }
}