#include "SceneCamera.hpp"
#include "fmpch.hpp"
#include <glm/gtc/matrix_transform.hpp>
namespace Fermion
{
    SceneCamera::SceneCamera() : Camera()
    {
        recalculateProjection();
    }

    SceneCamera::SceneCamera(const glm::mat4 &mat) : Camera(mat)
    {
        // Respect the provided projection matrix; don't override it here.
        // Viewport/aspect updates will call recalculateProjection() later if needed.
    }

    void SceneCamera::setOrthographic(float size, float nearClip, float farClip)
    {
        m_orthographicSize = size;
        m_orthographicNear = nearClip;
        m_orthographicFar = farClip;
        recalculateProjection();
    }
    void SceneCamera::setViewportSize(float width, float height)
    {
        m_aspectRatio = width / height;
        recalculateProjection();
    }
    void SceneCamera::recalculateProjection()
    {
        float orthoLeft = -m_orthographicSize * m_aspectRatio / 2.0f;
        float orthoRight = m_orthographicSize * m_aspectRatio / 2.0f;
        float orthoBottom = -m_orthographicSize / 2.0f;
        float orthoTop = m_orthographicSize / 2.0f;

        setProjection(glm::ortho(orthoLeft, orthoRight,
                                 orthoBottom, orthoTop,
                                 m_orthographicNear, m_orthographicFar));
    }
}
