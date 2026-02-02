#include "Core/Input.hpp"
#include "Core/KeyCodes.hpp"

#include "Core/Application.hpp"
#include "Core/Window.hpp"
#include <GLFW/glfw3.h>
#include "GLFWKeyCodes.hpp"
#include "GLFWMouseCodes.hpp"
namespace Fermion
{

    static int ConvertFMCursorModeToGLFW(const CursorMode mode)
    {
        switch (mode)
        {
        case CursorMode::Normal:
            return GLFW_CURSOR_NORMAL;
        case CursorMode::Hidden:
            return GLFW_CURSOR_HIDDEN;
        case CursorMode::Locked:
            return GLFW_CURSOR_DISABLED;
        default:
            return GLFW_CURSOR_NORMAL;
        }
    }

    bool Input::isKeyPressed(const KeyCode key)
    {
        auto *window = static_cast<GLFWwindow *>(Application::get().getWindow().getNativeWindow());
        auto state = glfwGetKey(window, FMKeyCodeToGLFWKeyCode(key));
        return state == GLFW_PRESS;
    }

    bool Input::isMouseButtonPressed(const MouseCode button)
    {
        auto *window = static_cast<GLFWwindow *>(Application::get().getWindow().getNativeWindow());
        auto state = glfwGetMouseButton(window, FMMouseCodeToGLFWMouseCode(button));
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::getMousePosition()
    {
        auto *window = static_cast<GLFWwindow *>(Application::get().getWindow().getNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        return {(float)xpos, (float)ypos};
    }

    void Input::setCursorMode(const CursorMode mode)
    {
        auto *window = static_cast<GLFWwindow *>(Application::get().getWindow().getNativeWindow());
        glfwSetInputMode(window, GLFW_CURSOR, ConvertFMCursorModeToGLFW(mode));
    }

    void Input::setMousePosition(float x, float y)
    {
        auto *window = static_cast<GLFWwindow *>(Application::get().getWindow().getNativeWindow());
        glfwSetCursorPos(window, static_cast<double>(x), static_cast<double>(y));
    }

    void Input::setRawMouseMotion(bool enabled)
    {
        auto *window = static_cast<GLFWwindow *>(Application::get().getWindow().getNativeWindow());
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, enabled ? GLFW_TRUE : GLFW_FALSE);
        }
    }

    float Input::getMouseX()
    {
        return getMousePosition().x;
    }

    float Input::getMouseY()
    {
        return getMousePosition().y;
    }

} // namespace Fermion
