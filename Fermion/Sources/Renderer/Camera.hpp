#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace Fermion
{
    class Camera
    {
    public:
        Camera() = default;
        Camera(const glm::mat4 &m_projection) : m_projection(m_projection) {}
        virtual ~Camera() = default;

        const glm::mat4 &getProjection() const { return m_projection; }

        void setProjection(const glm::mat4 &m_projection)
        {
            this->m_projection = m_projection;
        }
        void setPerspectiveProjection(const float radFov, const float width, const float height, const float nearP, const float farP)
        {
            m_projection = glm::perspectiveFov(radFov, width, height, farP, nearP);
        }

        void setOrthoProjection(const float width, const float height, const float nearP, const float farP)
        {

            m_projection = glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, farP, nearP);
        }

    protected:
        glm::mat4 m_projection;
    };

}