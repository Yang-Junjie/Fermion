#pragma once
#include "Renderer/Camera.hpp"
namespace Fermion
{
    class SceneCamera : public Camera
    {
    public:
        enum class ProjectionType
        {
            Orthographic = 0,
            Perspective = 1
        };

    public:
        SceneCamera();
        SceneCamera(const glm::mat4 &mat);
        virtual ~SceneCamera() = default;
        
        void setOrthographic(float size, float nearClip, float farClip);
        void setPerspective(float fov, float near, float far); 
        void setViewportSize(float width, float height);
        

        void setProjectionType(ProjectionType type){ m_projectionType = type; recalculateProjection(); }
        ProjectionType getProjectionType() const{return m_projectionType;}

        float getOrthographicSize() const{return m_orthographicSize;}
        void setOrthographicSize(const float &size){m_orthographicSize = size; recalculateProjection();}
        float getOrthographicNearClip() const{return m_orthographicNear;}
        void setOrthographicNearClip(float nearclip){m_orthographicNear = nearclip; recalculateProjection();}
        float getOrthographicFarClip() const{return m_orthographicFar;}
        void setOrthographicFarClip(float farclip){m_orthographicFar = farclip; recalculateProjection();}


        float getPerspectiveFOV() const{return m_perspectiveFOV;}
        void  setPerspectiveFOV(float fov){m_perspectiveFOV = fov; recalculateProjection();}
        float getPerspectiveNearClip() const{return m_perspectiveNear;}
        void  setPerspectiveNearClip(float near){m_perspectiveNear = near; recalculateProjection();}
        float getPerspectiveFarClip() const {return m_perspectiveFar;}
        void  setPerspectiveFarClip(float farclip) {m_perspectiveFar = farclip;recalculateProjection();}


        
        

    private:
        void recalculateProjection();

    private:
        ProjectionType m_projectionType = ProjectionType::Orthographic;
        
        float m_perspectiveFOV = glm::radians(45.0f);
        float m_perspectiveNear = 0.01f;
        float m_perspectiveFar = 100.0f;
        
        float m_orthographicSize = 10.0f;
        // Widen near/far to avoid early clipping when editing Z in 2D
        float m_orthographicNear = -100.0f;
        float m_orthographicFar = 100.0f;

        float m_aspectRatio = 0.0f;
    };

}
