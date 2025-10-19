#include "Engine/Engine.hpp"
#include "imgui.h"

#ifdef USE_SFML_BACKEND
#include "SFMLWindow.hpp"
#include "SFMLRenderer.hpp"
#include "imgui-SFML.h"
#include "ImGuiBackendSFML.hpp"
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
        m_window = std::make_unique<SFMLWindow>(windowProps);
        auto &sfWindow = static_cast<SFMLWindow *>(m_window.get())->getWindow();

        if (!sfWindow.setActive(true))
        {
            Log::Error("Failed to activate SFML OpenGL context!");
            return;
        }
        Log::Info("SFML OpenGL context activated.");

        m_renderer = std::make_unique<SFMLRenderer>(sfWindow);

        if (ImGuiBackendSFML::Init(sfWindow))
            Log::Info("ImGui initialized successfully!");
        else
            Log::Error("ImGui initialization failed!");

#endif

        m_window->setEventCallback([this](IEvent &event)
                                   { this->onEvent(event); });

        m_imGuiLayer = std::make_unique<ImGuiLayer>();
        m_imGuiLayerRaw = m_imGuiLayer.get();

        m_layerStack.pushOverlay(std::move(m_imGuiLayer));
    }

    void Engine::run()
    {
        Log::Info("Engine started!");
        init();

        auto &sfWindow = static_cast<SFMLWindow *>(m_window.get())->getWindow();
        sf::Clock deltaClock;
        while (m_running && m_window->isOpen())
        {
            sf::Time dt = deltaClock.restart(); // 待实现Timer类
            ImGuiBackendSFML::BeginFrame(sfWindow, dt);

            sfWindow.clear();
            for (auto &layer : m_layerStack)
                layer->OnUpdate();

            ImGuiBackendSFML::EndFrame(sfWindow);
            m_window->onUpdate();
        }
        ImGuiBackendSFML::Shutdown(sfWindow);
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