#include "Core/Window.hpp"
#include "GLFWWindow.hpp"

namespace Fermion
{
    // 若存在多个窗口后端使用工厂模式
    std::unique_ptr<IWindow> IWindow::create(const WindowProps &props)
    {
        return std::make_unique<GLFWWindow>(props);
    }
}