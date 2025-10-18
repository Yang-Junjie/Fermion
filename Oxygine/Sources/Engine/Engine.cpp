#include "Engine/Engine.h"


#ifdef USE_SFML_BACKEND
#include "SFMLWindow.h"
#include "SFMLRenderer.h"
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

namespace Oxygine
{
    Engine::Engine()
    {
       
        WindowProps windowProps;
#ifdef USE_SFML_BACKEND
        auto sfmlWindow = std::make_unique<SFMLWindow>(windowProps);
        m_renderer = std::make_unique<SFMLRenderer>(sfmlWindow->get());
        m_window = std::move(sfmlWindow);
#endif

        m_window->setEventCallback([this](IEvent &event)
                                   { this->onEvent(event); });
    }

    void Engine::run()
    {
        Log::Info("Engine started!");

        while (m_running && m_window->isOpen())
        {
            m_window->pollEvents();
            m_window->clear();

            m_renderer->drawImage("assets/textures/test.jpg", {100, 100});
            m_renderer->drawRect({0, 0}, {100, 100}, {1.0f, 0.0f, 0.0f, 1.0f});
            m_window->display();

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    void Engine::onEvent(IEvent &event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent &e)
                                               { return this->onWindowResize(e); });
        dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent &e)
                                              { return this->onWindowClose(e); });
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