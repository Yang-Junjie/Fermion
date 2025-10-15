#include "Engine/Engine.h"
#include "Core/Log.hpp"


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
#ifdef USE_SFML_BACKEND
        auto sfmlWindow = std::make_unique<SFMLWindow>();
        m_renderer = std::make_unique<SFMLRenderer>(sfmlWindow->get());
        m_window = std::move(sfmlWindow);
#elif defined(USE_SDL_BACKEND)
        auto sdlWindow = std::make_unique<SDLWindow>();
        m_renderer = std::make_unique<SDLRenderer>(sdlWindow->getRenderer());
        m_window = std::move(sdlWindow);
#endif
    }

    void Engine::run()
    {
        Log::Init("engine.log", LogLevel::Debug);
        Log::Info("Engine started!");

        while (m_window->isOpen())
        {
            m_window->pollEvents();
            m_window->clear();

            // m_renderer->drawImage("assets/textures/test.jpg", 0, 0);
            m_renderer->drawRect({0,0},{100,100},{1.0f,0.0f,0.0f,1.0f});
            m_window->display();

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

}