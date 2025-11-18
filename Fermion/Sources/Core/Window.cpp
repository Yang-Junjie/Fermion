#include "Core/Window.hpp"
#ifdef FM_PLATFORM_DESKTOP
#include "GLFWWindow.hpp"
#else
#endif

namespace Fermion
{
    std::unique_ptr<IWindow> IWindow::create(const WindowProps &props)
    {
        return std::make_unique<GLFWWindow>(props);
    }
}