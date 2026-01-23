#include "RenderGraphResourcePool.hpp"

namespace Fermion
{
    std::shared_ptr<Framebuffer> RenderGraphResourcePool::acquireFramebuffer(const RenderGraphResourceDesc &desc)
    {
        for (auto &pooled : m_Pool)
        {
            if (!pooled.inUse && isCompatible(pooled.framebuffer->getSpecification(), desc))
            {
                pooled.inUse = true;
                return pooled.framebuffer;
            }
        }

        FramebufferSpecification spec;
        spec.width = desc.width;
        spec.height = desc.height;
        spec.attachments = {desc.format};
        spec.swapChainTarget = false;

        auto framebuffer = Framebuffer::create(spec);
        m_Pool.push_back({framebuffer, true});
        return framebuffer;
    }

    void RenderGraphResourcePool::releaseFramebuffer(std::shared_ptr<Framebuffer> framebuffer)
    {
        for (auto &pooled : m_Pool)
        {
            if (pooled.framebuffer == framebuffer)
            {
                pooled.inUse = false;
                return;
            }
        }
    }

    void RenderGraphResourcePool::reset()
    {
        for (auto &pooled : m_Pool)
        {
            pooled.inUse = false;
        }
    }

    bool RenderGraphResourcePool::isCompatible(const FramebufferSpecification &spec,
                                                const RenderGraphResourceDesc &desc) const
    {
        if (spec.width != desc.width || spec.height != desc.height)
            return false;

        if (spec.attachments.attachments.empty())
            return false;

        return spec.attachments.attachments[0].textureFormat == desc.format;
    }

} // namespace Fermion
