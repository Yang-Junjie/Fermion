#pragma once

#include "Camera.hpp"
#include "Core/Timestep.hpp"

#include "Events/MouseEvent.hpp"

#include <glm/glm.hpp>

namespace Fermion
{
    enum class ProjectionType
    {
        Perspective = 0,
        Orthographic = 1
    };

    class IEvent;
    class EditorCamera : public Camera
    {
    public:
        EditorCamera() = default;
        EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

        void onUpdate(Timestep ts);
        void onEvent(IEvent &e);

        inline float getDistance() const
        {
            return m_distance;
        }
        inline void setDistance(const float distance)
        {
            m_distance = distance;
        }

        inline void setPosition(const glm::vec3 &position)
        {
            m_position = position;
            updateView();
        }

        inline void setViewportSize(const float width, const float height)
        {
            m_viewportWidth = width;
            m_viewportHeight = height;
            updateProjection();
        }

        const glm::mat4 &getViewMatrix() const
        {
            return m_viewMatrix;
        }
        glm::mat4 getViewProjection() const
        {
            return m_projection * m_viewMatrix;
        }

        glm::vec3 getUpDirection() const;
        glm::vec3 getRightDirection() const;
        glm::vec3 getForwardDirection() const;
        const glm::vec3 &getPosition() const
        {
            return m_position;
        }
        glm::quat getOrientation() const;

        float getPitch() const
        {
            return m_pitch;
        }
        float getYaw() const
        {
            return m_yaw;
        }

        float getFov() const { return m_fov; }
        float getAspectRatio() const { return m_aspectRatio; }
        float getNearCilp() const { return m_nearClip; }
        float getFarCilp() const { return m_farClip; }

        float getFPSSpeed() const { return m_fpsMoveSpeed; }
        bool isFPSMode() const { return m_isFpsMode; }

        void setCanEnterFpsMode(bool canEnter) { m_canEnterFpsMode = canEnter; }

        ProjectionType getProjectionType() const { return m_projectionType; }
        void setProjectionType(ProjectionType type);

        float getOrthographicSize() const { return m_orthoSize; }
        void setOrthographicSize(float size) { m_orthoSize = size; updateProjection(); }

        glm::mat4 getPerspectiveProjection() const
        {
            return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearClip, m_farClip);
        }

        const glm::vec3 &getFocalPoint() const { return m_focalPoint; }

        void setYawPitch(float yaw, float pitch)
        {
            m_yaw = yaw;
            m_pitch = pitch;
            updateView();
        }

        void setFocalPoint(const glm::vec3 &focalPoint)
        {
            m_focalPoint = focalPoint;
            updateView();
        }

    private:
        void updateProjection();
        void updateView();
        void updateFpsCamera(Timestep ts);

        bool onMouseScroll(MouseScrolledEvent &e);

        void mousePan(const glm::vec2 &delta);
        void mouseRotate(const glm::vec2 &delta);
        void mouseZoom(float delta);

        glm::vec3 calculatePosition() const;

        std::pair<float, float> panSpeed() const;
        float rotationSpeed() const;
        float zoomSpeed() const;

    private:
        ProjectionType m_projectionType = ProjectionType::Perspective;
        float m_fov = 45.0f, m_aspectRatio = 1.778f, m_nearClip = 0.1f, m_farClip = 1000.0f;
        float m_orthoSize = 10.0f;
        float m_orthoNearClip = -1000.0f, m_orthoFarClip = 1000.0f;

        glm::mat4 m_viewMatrix;
        glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
        glm::vec3 m_focalPoint = {0.0f, 0.0f, 0.0f};

        glm::vec2 m_initialMousePosition = {0.0f, 0.0f};
        glm::vec2 m_savedCursorPosition = {0.0f, 0.0f};
        bool m_hasSavedCursorPosition = false;

        float m_distance = 17.3205f;  // sqrt(10^2 + 10^2 + 10^2) = sqrt(300)
        float m_pitch = 0.6155f, m_yaw = -0.7854f;  // 从(10,10,10)看向(0,0,0)

        float m_viewportWidth = 1280, m_viewportHeight = 720;

        bool m_isFpsMode = false;
        float m_fpsMoveSpeed = 5.0f;
        bool m_canEnterFpsMode = true;
    };

} // namespace Fermion
