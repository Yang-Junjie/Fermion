#include "GraphicsContext.hpp"

#include "Core/Log.hpp"
#include "OpenGLContext.hpp"
#include "Renderer/RendererAPI.hpp"

#ifdef FM_HAS_VULKAN
#include "VulkanContext.hpp"
#endif

namespace Fermion {

std::unique_ptr<GraphicsContext> GraphicsContext::create(void *window)
{
    switch (RendererAPI::getAPI()) {
    case RendererAPI::API::None:
        Log::Error("GraphicsContext creation requested with RendererAPI::None.");
        return nullptr;
    case RendererAPI::API::OpenGL:
        return std::make_unique<OpenGLContext>(window);
    case RendererAPI::API::Vulkan:
#ifdef FM_HAS_VULKAN
        return std::make_unique<VulkanContext>(window);
#else
        Log::Error("Vulkan was requested, but this build was not compiled with Vulkan support.");
        return nullptr;
#endif
    }

    return nullptr;
}

} // namespace Fermion
