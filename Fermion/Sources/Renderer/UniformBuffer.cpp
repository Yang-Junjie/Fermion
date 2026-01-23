#include "fmpch.hpp"
#include "UniformBuffer.hpp"
#include "OpenGLUniformBuffer.hpp"
#include "Renderer/Renderers/Renderer.hpp"

namespace Fermion
{
    std::shared_ptr<UniformBuffer> UniformBuffer::create(uint32_t bindingPoint, uint32_t size)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            FERMION_ASSERT(false, "RendererAPI::None is not supported!");
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLUniformBuffer>(bindingPoint, size);
        }

        FERMION_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
} // namespace Fermion
