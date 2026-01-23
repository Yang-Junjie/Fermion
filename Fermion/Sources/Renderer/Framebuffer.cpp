#include "Renderer/Framebuffer.hpp"
#include "Renderer/Renderers/Renderer.hpp"
#include "OpenGLFramebuffer.hpp"

namespace Fermion
{

    std::shared_ptr<Framebuffer> Framebuffer::create(const FramebufferSpecification &spec)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLFramebuffer>(spec);
        }

        FERMION_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    void Framebuffer::blit(const std::shared_ptr<Framebuffer> &source, const std::shared_ptr<Framebuffer> &target,
                           const FramebufferBlitSpecification &spec)
    {
        if (!source || !target)
            return;
        source->blitTo(target, spec);
    }

} // namespace Fermion
