#include "Core/Window.hpp"

#include "GLFWWindow.hpp"

namespace Fermion
{
    std::unique_ptr<IWindow> IWindow::create(const WindowProps &props)
    {
        return std::make_unique<GLFWWindow>(props);
    }
}