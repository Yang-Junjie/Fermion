#include "fmpch.hpp"
#include "Renderer/OrthographicCameraController.hpp"

#include "Core/Input.hpp"
#include "Core/KeyCodes.hpp"

namespace Fermion
{

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_aspectRatio(aspectRatio), 
		m_camera(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel), m_rotation(rotation)
	{
	}

	void OrthographicCameraController::onUpdate(Timestep ts)
	{
		FM_PROFILE_FUNCTION();

		if (Input::isKeyPressed(KeyCode::A))
		{
			m_cameraPosition.x -= cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
			m_cameraPosition.y -= sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
		}
		else if (Input::isKeyPressed(KeyCode::D))
		{
			m_cameraPosition.x += cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
			m_cameraPosition.y += sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
		}

		if (Input::isKeyPressed(KeyCode::W))
		{
			m_cameraPosition.x += -sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
			m_cameraPosition.y += cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
		}
		else if (Input::isKeyPressed(KeyCode::S))
		{
			m_cameraPosition.x -= -sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
			m_cameraPosition.y -= cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
		}

		if (m_rotation)
		{
			if (Input::isKeyPressed(KeyCode::Q))
				m_cameraRotation += m_cameraRotationSpeed * ts;
			if (Input::isKeyPressed(KeyCode::E))
				m_cameraRotation -= m_cameraRotationSpeed * ts;

			if (m_cameraRotation > 180.0f)
				m_cameraRotation -= 360.0f;
			else if (m_cameraRotation <= -180.0f)
				m_cameraRotation += 360.0f;

			m_camera.setRotation(m_cameraRotation);
		}

		m_camera.setPosition(m_cameraPosition);

		m_cameraTranslationSpeed = m_zoomLevel;
	}

	void OrthographicCameraController::onEvent(IEvent &e)
	{
		FM_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>([this](MouseScrolledEvent &e)
												{ return this->onMouseScrolled(e); });

		dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent &e)
											   { return this->onWindowResized(e); });
	}

void OrthographicCameraController::onResize(float width, float height)
{
    if (height <= 0.0f)
        return;
    m_aspectRatio = width / height;
    m_camera.setProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);
    
}

	bool OrthographicCameraController::onMouseScrolled(MouseScrolledEvent &e)
	{
        FM_PROFILE_FUNCTION();

		m_zoomLevel -= e.getYOffset() * 0.25f;
		m_zoomLevel = std::max(m_zoomLevel, 0.25f);
		
		m_camera.setProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);

		return false;
	}

	bool OrthographicCameraController::onWindowResized(WindowResizeEvent &e)
	{
        FM_PROFILE_FUNCTION();

		onResize((float)e.getWidth(), (float)e.getHeight());
		return false;
	}

}
