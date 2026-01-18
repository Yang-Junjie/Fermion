#include "fmpch.hpp"
#include "Renderer/Camera/OrthographicCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Fermion
{

    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top) : m_projectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_viewMatrix(1.0f)
    {
        FM_PROFILE_FUNCTION();
        m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
    }

    void OrthographicCamera::setProjection(float left, float right, float bottom, float top)
    {
        FM_PROFILE_FUNCTION();
        m_projectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
        m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
    }

    void OrthographicCamera::recalculateViewMatrix()
    {
        FM_PROFILE_FUNCTION();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

        m_viewMatrix = glm::inverse(transform);
        m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
    }

} // namespace Fermion