#pragma once
#include <glm/glm.hpp>
namespace Fermion
{
    class Camera
    {
    public:
        Camera() = default;
        Camera(const glm::mat4 &m_projection) : m_projection(m_projection) {}
        void setProjection(const glm::mat4 &m_projection) { this->m_projection = m_projection; }

        const glm::mat4 &getProjection() const { return m_projection; }
    private:
        glm::mat4 m_projection;
    };

}