#pragma once
#include "IWindow.h"
#include "IRenderer.h"
#include <memory>

class EngineCore {
    std::unique_ptr<IWindow> m_window;
    std::unique_ptr<IRenderer> m_renderer;
public:
    EngineCore();
    void run();
};
