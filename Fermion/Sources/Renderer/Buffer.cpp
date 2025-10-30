﻿#include "Renderer/Buffer.hpp"
#include "Renderer/Renderer.hpp"
#include "OpenGLBuffer.hpp"

namespace Fermion
{

    std::shared_ptr<VertexBuffer> VertexBuffer::create(uint32_t size)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLVertexBuffer>(size);
        }

        return nullptr;
    }

    std::shared_ptr<VertexBuffer> VertexBuffer::create(float *vertices, uint32_t size)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLVertexBuffer>(vertices, size);
        }

        return nullptr;
    }

    std::shared_ptr<IndexBuffer> IndexBuffer::create(uint32_t *indices, uint32_t size)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLIndexBuffer>(indices, size);
        }

        return nullptr;
    }

}