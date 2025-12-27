#include "fmpch.hpp"
#include "Renderer/Framebuffer.hpp"

#include "Renderer/Renderer.hpp"

#include "OpenGLFramebuffer.hpp"

namespace Fermion {

std::shared_ptr<Framebuffer> Framebuffer::create(const FramebufferSpecification &spec) {
    switch (Renderer::getAPI()) {
    case RendererAPI::API::None: return nullptr;
    case RendererAPI::API::OpenGL: return std::make_shared<OpenGLFramebuffer>(spec);
    }

    FERMION_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

} // namespace Fermion
