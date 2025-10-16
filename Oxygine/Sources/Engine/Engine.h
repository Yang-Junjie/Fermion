#pragma once
#include "Core/Window.h"
#include "Renderer/Renderer.h"
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
    };
}