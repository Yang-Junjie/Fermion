#pragma once
#include "Platform/IWindow.h"
#include "Renderer/IRenderer.h"
#include <memory>

class Engine {
    std::unique_ptr<IWindow> m_window;
    std::unique_ptr<IRenderer> m_renderer;
public:
    Engine();
    void run();
};
