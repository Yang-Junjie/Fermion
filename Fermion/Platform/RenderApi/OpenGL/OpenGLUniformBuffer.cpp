#include "fmpch.hpp"
#include "OpenGLUniformBuffer.hpp"
#include <glad/glad.h>

namespace Fermion
{
    OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t bindingPoint, uint32_t size)
        : m_bindingPoint(bindingPoint), m_size(size)
    {
        FM_PROFILE_FUNCTION();

        // Create buffer using DSA (Direct State Access)
        glCreateBuffers(1, &m_rendererID);

        // Allocate buffer storage with dynamic usage hint
        glNamedBufferData(m_rendererID, size, nullptr, GL_DYNAMIC_DRAW);

        // Bind buffer to the specified binding point
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_rendererID);
    }

    OpenGLUniformBuffer::~OpenGLUniformBuffer()
    {
        FM_PROFILE_FUNCTION();

        if (m_rendererID != 0)
        {
            glDeleteBuffers(1, &m_rendererID);
        }
    }

    void OpenGLUniformBuffer::bind() const
    {
        FM_PROFILE_FUNCTION();

        // Bind to the uniform buffer binding point
        glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_rendererID);
    }

    void OpenGLUniformBuffer::unbind() const
    {
        FM_PROFILE_FUNCTION();

        // Unbind from the binding point
        glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, 0);
    }

    void OpenGLUniformBuffer::setData(const void *data, uint32_t size, uint32_t offset)
    {
        FM_PROFILE_FUNCTION();

        // Validate parameters
        FERMION_ASSERT(offset + size <= m_size, "Buffer overflow: trying to write beyond buffer size");

        // Update buffer data using DSA
        glNamedBufferSubData(m_rendererID, offset, size, data);
    }

} // namespace Fermion
