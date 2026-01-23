#pragma once
#include "RenderGraphResource.hpp"
#include <unordered_map>
#include <vector>

namespace Fermion
{
    class RenderGraphResourcePool
    {
    public:
        std::shared_ptr<Framebuffer> acquireFramebuffer(const RenderGraphResourceDesc &desc);
        void releaseFramebuffer(std::shared_ptr<Framebuffer> framebuffer);
        void reset();

    private:
        struct PooledFramebuffer
        {
            std::shared_ptr<Framebuffer> framebuffer;
            bool inUse = false;
        };

        std::vector<PooledFramebuffer> m_Pool;

        bool isCompatible(const FramebufferSpecification &spec, const RenderGraphResourceDesc &desc) const;
    };

} // namespace Fermion
