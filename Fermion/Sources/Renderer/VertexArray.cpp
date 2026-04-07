#include "fmpch.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Renderers/Renderer.hpp"
#include "OpenGLVertexArray.hpp"

namespace Fermion {

std::shared_ptr<VertexArray> VertexArray::create() {
    switch (Renderer::getAPI()) {
    case RendererAPI::API::None: return nullptr;
    case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexArray>();
    case RendererAPI::API::Vulkan:
        FERMION_ASSERT(false, "Vulkan vertex array creation is not implemented yet.");
        return nullptr;
    }
    return nullptr;
}

} // namespace Fermion
