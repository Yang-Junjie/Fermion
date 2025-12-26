#include "fmpch.hpp"
#include "Core/Input.hpp"

#include "Core/Application.hpp"
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
		case CursorMode::Disabled:
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

	float Input::getMouseX()
	{
		return getMousePosition().x;
	}

	float Input::getMouseY()
	{
		return getMousePosition().y;
	}

}