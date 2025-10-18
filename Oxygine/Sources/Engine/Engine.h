#pragma once
#include "Core/Window.h"
#include "Renderer/Renderer.h"
#include "Events/Event.hpp"
#include "Events/ApplicationEvent.hpp"
#include <memory>

namespace Oxygine
{
    class Engine
    {
    public:
        Engine();
        void run();

    private:
        void onEvent(IEvent &event);
        bool onWindowResize(WindowResizeEvent &event);
        bool onWindowClose(WindowCloseEvent &event);

    private:
        bool m_running = true;
        bool m_minimized = false;
        std::unique_ptr<IWindow> m_window;
        std::unique_ptr<IRenderer> m_renderer;
    };
}