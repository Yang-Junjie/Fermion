#pragma once
#include "Renderer/UniformBuffer.hpp"

namespace Fermion
{
    // OpenGL implementation of uniform buffer object
    class OpenGLUniformBuffer : public UniformBuffer
    {
    public:
        OpenGLUniformBuffer(uint32_t bindingPoint, uint32_t size);
        virtual ~OpenGLUniformBuffer();

        // Disable copy
        OpenGLUniformBuffer(const OpenGLUniformBuffer &) = delete;
        OpenGLUniformBuffer &operator=(const OpenGLUniformBuffer &) = delete;

        virtual void bind() const override;
        virtual void unbind() const override;
        virtual void setData(const void *data, uint32_t size, uint32_t offset = 0) override;
        virtual uint32_t getSize() const override { return m_size; }
        virtual uint32_t getBindingPoint() const override { return m_bindingPoint; }

    private:
        uint32_t m_rendererID = 0;
        uint32_t m_bindingPoint = 0;
        uint32_t m_size = 0;
    };
} // namespace Fermion
