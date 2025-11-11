#pragma once
#include "Renderer/Camera.hpp"
namespace Fermion
{
    class SceneCamera : public Camera
    {
    public:
        SceneCamera();
        SceneCamera(const glm::mat4 &mat);
        virtual ~SceneCamera() = default;
        void setOrthographic(float size, float nearClip, float farClip);
        void setViewportSize(float width, float height);

        float getOrthographicSize() const
        {
            return m_orthographicSize;
        }
        void setOrthographicSize(const float &size)
        {
            m_orthographicSize = size;
            recalculateProjection();
        }

    private:
        void recalculateProjection();

    private:
        float m_orthographicSize = 10.0f;
        float m_orthographicNear = -1.0f;
        float m_orthographicFar = 1.0f;

        float m_aspectRatio = 0.0f;
    };

}