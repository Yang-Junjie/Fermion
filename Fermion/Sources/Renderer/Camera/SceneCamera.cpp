#include "fmpch.hpp"
#include "SceneCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>
namespace Fermion {
SceneCamera::SceneCamera() : Camera() {
    recalculateProjection();
}

SceneCamera::SceneCamera(const glm::mat4 &mat) : Camera(mat) {
    // Respect the provided projection matrix; don't override it here.
    // Viewport/aspect updates will call recalculateProjection() later if needed.
}

void SceneCamera::setOrthographic(float size, float nearClip, float farClip) {
    m_projectionType = ProjectionType::Orthographic;
    m_orthographicSize = size;
    m_orthographicNear = nearClip;
    m_orthographicFar = farClip;
    recalculateProjection();
}
void SceneCamera::setPerspective(float fov, float near, float far) {
    m_projectionType = ProjectionType::Perspective;
    m_perspectiveFOV = fov;
    m_perspectiveNear = near;
    m_perspectiveFar = far;
    recalculateProjection();
}
void SceneCamera::setViewportSize(float width, float height) {
    m_aspectRatio = width / height;
    recalculateProjection();
}
void SceneCamera::recalculateProjection() {
    if (m_projectionType == ProjectionType::Orthographic) {
        float orthoLeft = -m_orthographicSize * m_aspectRatio / 2.0f;
        float orthoRight = m_orthographicSize * m_aspectRatio / 2.0f;
        float orthoBottom = -m_orthographicSize / 2.0f;
        float orthoTop = m_orthographicSize / 2.0f;

        m_projection = glm::ortho(orthoLeft, orthoRight,
                                  orthoBottom, orthoTop,
                                  m_orthographicNear, m_orthographicFar);
    } else {
        m_projection = glm::perspective(m_perspectiveFOV, m_aspectRatio,
                                        m_perspectiveNear, m_perspectiveFar);
    }
}
} // namespace Fermion
