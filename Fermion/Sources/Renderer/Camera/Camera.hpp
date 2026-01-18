#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace Fermion
{
    class Camera
    {
    public:
        Camera() = default;
        explicit Camera(const glm::mat4 &projection) : m_projection(projection)
        {
        }
        virtual ~Camera() = default;

        const glm::mat4 &getProjection() const
        {
            return m_projection;
        }

        void setProjection(const glm::mat4 &projection)
        {
            this->m_projection = projection;
        }
        void setPerspectiveProjection(const float radFov, const float width, const float height, const float nearP, const float farP)
        {
            m_projection = glm::perspectiveFov(radFov, width, height, nearP, farP);
        }

        void setOrthoProjection(const float width, const float height, const float nearP, const float farP)
        {
            m_projection = glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, nearP, farP);
        }

    protected:
        glm::mat4 m_projection;
    };

} // namespace Fermion