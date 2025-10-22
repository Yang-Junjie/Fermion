#include "Engine/Engine.hpp"
#include "fmpch.hpp"
#include "imgui.h"
#include "Core/Timestep.hpp"

#ifdef USE_SFML_BACKEND
#include "SFMLWindow.hpp"
#include "SFMLRenderer.hpp"
#include "imgui-SFML.h"
#include "ImGuiBackendSFML.hpp"
#include "SFMLTimer.hpp"
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

        m_imguiBackend = std::make_unique<ImGuiBackendSFMLImpl>(sfWindow);
        if (m_imguiBackend->Init(&sfWindow))
            Log::Info("ImGui (SFML) initialized successfully!");
        else
            Log::Error("ImGui (SFML) initialization failed!");
        m_timer = std::make_unique<SFMLTimer>();

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

        while (m_running && m_window->isOpen())
        {
            float time = m_timer->elapsed();
            Timestep timestep = time - m_lastFrameTime;
            m_lastFrameTime = time;

            // if (timestep.GetSeconds() <= 0.0f)
            //     timestep = 1.0f / 60.0f;
            for (auto &layer : m_layerStack)
                layer->OnUpdate(timestep);

            m_imguiBackend->BeginFrame(timestep);
            for (auto &layer : m_layerStack)
                layer->OnImGuiRender();
            m_imguiBackend->EndFrame();

            m_window->OnUpdate();
        }

        m_imguiBackend->Shutdown();
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