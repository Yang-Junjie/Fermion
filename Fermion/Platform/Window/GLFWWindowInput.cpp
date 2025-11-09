#include "fmpch.hpp"
#include "Core/Input.hpp"

#include "Core/Engine.hpp"
#include <GLFW/glfw3.h>
#include "GLFWKeyCodes.hpp"
#include "GLFWMouseCodes.hpp"
namespace Fermion {

	bool Input::isKeyPressed(const KeyCode key)
	{
		auto* window = static_cast<GLFWwindow*>(Engine::get().getWindow().getNativeWindow());
		auto state = glfwGetKey(window, FMKeyCodeToGLFWKeyCode(key));
		return state == GLFW_PRESS;
	}

	bool Input::isMouseButtonPressed(const MouseCode button)
	{
		auto* window = static_cast<GLFWwindow*>(Engine::get().getWindow().getNativeWindow());
		auto state = glfwGetMouseButton(window, FMMouseCodeToGLFWMouseCode(button));
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::getMousePosition()
	{
		auto* window = static_cast<GLFWwindow*>(Engine::get().getWindow().getNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
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