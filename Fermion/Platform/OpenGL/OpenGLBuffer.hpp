#pragma once

#include "Renderer/Buffer.hpp"

namespace Fermion {
    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        explicit OpenGLVertexBuffer(uint32_t size);

        OpenGLVertexBuffer(float *vertices, uint32_t size);

        ~OpenGLVertexBuffer() override;

        void bind() const override;

        void unbind() const override;

        void setData(const void *data, uint32_t size) override;

        const BufferLayout &getLayout() const override {
            return m_layout;
        }

        void setLayout(const BufferLayout &layout) override {
            m_layout = layout;
        }

    private:
        uint32_t m_rendererID;
        BufferLayout m_layout;
    };

    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(uint32_t *indices, uint32_t count);

        ~OpenGLIndexBuffer() override;

        virtual void bind() const;

        virtual void unbind() const;

        virtual uint32_t getCount() const {
            return m_count;
        }

    private:
        uint32_t m_rendererID;
        uint32_t m_count;
    };
} // namespace Fermion
