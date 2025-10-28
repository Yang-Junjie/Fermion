#include "fmpch.hpp"
#include "Core/Input.hpp"

#include "Engine/Engine.hpp"
#include <GLFW/glfw3.h>
#include "GLFWKeyCodes.hpp"
#include "GLFWMouseCodes.hpp"
namespace Fermion {

	bool Input::IsKeyPressed(const KeyCode key)
	{
		auto* window = static_cast<GLFWwindow*>(Engine::get().getWindow().getNativeWindow());
		auto state = glfwGetKey(window, FMKeyCodeToGLFWKeyCode(key));
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonPressed(const MouseCode button)
	{
		auto* window = static_cast<GLFWwindow*>(Engine::get().getWindow().getNativeWindow());
		auto state = glfwGetMouseButton(window, FMMouseCodeToGLFWMouseCode(button));
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePosition()
	{
		auto* window = static_cast<GLFWwindow*>(Engine::get().getWindow().getNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}

}