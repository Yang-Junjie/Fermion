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
        std::unique_ptr<IWindow> m_window;
        std::unique_ptr<IRenderer> m_renderer;

    public:
        Engine();
        void run();
        
    private:
        void onEvent(IEvent& event);
        void onWindowResize(WindowResizeEvent& event);
    };
}