#include "fmpch.hpp"
#include "EditorCamera.hpp"

#include "Core/Input.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/MouseCodes.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Fermion
{

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_fov(fov), m_aspectRatio(aspectRatio), m_nearClip(nearClip), m_farClip(farClip),
		  Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		updateView();
	}

	void EditorCamera::updateProjection()
	{
		m_aspectRatio = m_viewportWidth / m_viewportHeight;
		m_projection = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearClip, m_farClip);
	}

	void EditorCamera::updateView()
	{
		glm::quat orientation = getOrientation();

		if (!m_isFpsMode)
			m_position = calculatePosition();

		m_viewMatrix =
			glm::translate(glm::mat4(1.0f), m_position) *
			glm::toMat4(orientation);

		m_viewMatrix = glm::inverse(m_viewMatrix);
	}

	void EditorCamera::updateFpsCamera(Timestep ts)
	{
		float speed = m_fpsMoveSpeed;
		float dt = ts.getSeconds();

		if (Input::isKeyPressed(KeyCode::W))
			m_position += getForwardDirection() * speed * dt;
		if (Input::isKeyPressed(KeyCode::S))
			m_position -= getForwardDirection() * speed * dt;
		if (Input::isKeyPressed(KeyCode::A))
			m_position -= getRightDirection() * speed * dt;
		if (Input::isKeyPressed(KeyCode::D))
			m_position += getRightDirection() * speed * dt;

		if (Input::isKeyPressed(KeyCode::Space))
			m_position += getUpDirection() * speed * dt;
		if (Input::isKeyPressed(KeyCode::LeftShift))
			m_position -= getUpDirection() * speed * dt;
	}

	void EditorCamera::onUpdate(Timestep ts)
	{
		glm::vec2 mouse{Input::getMouseX(), Input::getMouseY()};

		// FPS MODE
		if (Input::isMouseButtonPressed(MouseCode::Right))
		{
			if (!m_isFpsMode)
			{
				m_isFpsMode = true;
				m_initialMousePosition = mouse;
				Input::setCursorMode(CursorMode::Disabled);
			}

			glm::vec2 delta = (mouse - m_initialMousePosition) * 0.002f;
			m_initialMousePosition = mouse;

			m_yaw += delta.x;
			m_pitch += delta.y;
			m_pitch = glm::clamp(m_pitch, -1.5f, 1.5f);

			updateFpsCamera(ts);
			updateView();
			return;
		}
		else if (m_isFpsMode)
		{
			m_isFpsMode = false;
			m_focalPoint = m_position + getForwardDirection() * m_distance;
			Input::setCursorMode(CursorMode::Normal);
		}

		if (Input::isKeyPressed(KeyCode::LeftAlt))
		{
			glm::vec2 delta = (mouse - m_initialMousePosition) * 0.003f;
			m_initialMousePosition = mouse;

			if (Input::isMouseButtonPressed(MouseCode::Middle))
				mousePan(delta);
			else if (Input::isMouseButtonPressed(MouseCode::Left))
				mouseRotate(delta);
			else if (Input::isMouseButtonPressed(MouseCode::Right))
				mouseZoom(delta.y);
		}

		updateView();
	}

	void EditorCamera::onEvent(IEvent &e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(
			[this](MouseScrolledEvent &e)
			{ return onMouseScroll(e); });
	}

	bool EditorCamera::onMouseScroll(MouseScrolledEvent &e)
	{
		float delta = e.getYOffset();

		if (m_isFpsMode)
		{
			m_fov -= delta * 2.0f;
			m_fov = glm::clamp(m_fov, 20.0f, 90.0f);
			updateProjection();
		}
		else
		{
			mouseZoom(delta * 0.1f);
		}
		updateView();
		return false;
	}

	void EditorCamera::mousePan(const glm::vec2 &delta)
	{
		auto [xSpeed, ySpeed] = panSpeed();
		m_focalPoint += -getRightDirection() * delta.x * xSpeed * m_distance;
		m_focalPoint += getUpDirection() * delta.y * ySpeed * m_distance;
	}

	void EditorCamera::mouseRotate(const glm::vec2 &delta)
	{
		float yawSign = getUpDirection().y < 0 ? -1.0f : 1.0f;
		m_yaw += yawSign * delta.x * rotationSpeed();
		m_pitch += delta.y * rotationSpeed();
	}

	void EditorCamera::mouseZoom(float delta)
	{
		m_distance -= delta * zoomSpeed();
		if (m_distance < 1.0f)
		{
			m_focalPoint += getForwardDirection();
			m_distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::getUpDirection() const
	{
		return glm::rotate(getOrientation(), glm::vec3(0, 1, 0));
	}

	glm::vec3 EditorCamera::getRightDirection() const
	{
		return glm::rotate(getOrientation(), glm::vec3(1, 0, 0));
	}

	glm::vec3 EditorCamera::getForwardDirection() const
	{
		return glm::rotate(getOrientation(), glm::vec3(0, 0, -1));
	}

	glm::vec3 EditorCamera::calculatePosition() const
	{
		return m_focalPoint - getForwardDirection() * m_distance;
	}

	glm::quat EditorCamera::getOrientation() const
	{
		return glm::quat(glm::vec3(-m_pitch, -m_yaw, 0.0f));
	}

	std::pair<float, float> EditorCamera::panSpeed() const
	{
		float x = std::min(m_viewportWidth / 1000.0f, 2.4f);
		float y = std::min(m_viewportHeight / 1000.0f, 2.4f);

		return {
			0.0366f * (x * x) - 0.1778f * x + 0.3021f,
			0.0366f * (y * y) - 0.1778f * y + 0.3021f};
	}

	float EditorCamera::rotationSpeed() const { return 0.8f; }

	float EditorCamera::zoomSpeed() const
	{
		float distance = std::max(m_distance * 0.2f, 0.0f);
		return std::min(distance * distance, 100.0f);
	}

}
