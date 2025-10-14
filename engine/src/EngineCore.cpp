#include "Engine/EngineCore.h"
#include "spdlog/spdlog.h"

#ifdef USE_SFML_BACKEND
#include "Platform/SFMLWindow.h"
#include "Platform/SFMLRenderer.h"
#elif defined(USE_SDL_BACKEND)
#include "Platform/SDLWindow.h"
#include "Platform/SDLRenderer.h"
#endif

#include <thread>
#include <chrono>

EngineCore::EngineCore()
{
#ifdef USE_SFML_BACKEND
    auto sfmlWindow = std::make_unique<SFMLWindow>();
    m_renderer = std::make_unique<SFMLRenderer>(sfmlWindow->get());
    m_window = std::move(sfmlWindow);
#elif defined(USE_SDL_BACKEND)
    auto sdlWindow = std::make_unique<SDLWindow>();
    renderer = std::make_unique<SDLRenderer>(sdlWindow->getRenderer());
    window = std::move(sdlWindow);
#endif
}

void EngineCore::run()
{
    spdlog::info("Engine starting...");
    while (m_window->isOpen())
    {
        m_window->pollEvents();
        m_window->clear();

        m_renderer->drawImage("assets/textures/test.jpg", 0, 0);
        m_window->display();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
