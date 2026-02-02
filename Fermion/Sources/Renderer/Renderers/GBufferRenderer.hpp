#pragma once
#include "RenderContext.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/RenderDrawCommand.hpp"
#include "Renderer/RenderGraphLegacy.hpp"
#include <memory>
#include <vector>

namespace Fermion
{
    class EnvironmentRenderer;
    class Pipeline;

    class GBufferRenderer
    {
    public:
        enum class Attachment : uint32_t
        {
            Albedo = 0,
            Normal = 1,
            Material = 2,
            Emissive = 3,
            ObjectID = 4
        };

        GBufferRenderer();

        void addPass(RenderGraphLegacy& renderGraph,
                     const RenderContext& context,
                     const std::vector<MeshDrawCommand>& drawList,
                     const std::shared_ptr<Pipeline>& pbrPipeline,
                     EnvironmentRenderer* environmentRenderer,
                     ResourceHandle gBuffer,
                     ResourceHandle sceneDepth,
                     uint32_t* geometryDrawCalls,
                     uint32_t* iblDrawCalls);

        std::shared_ptr<Framebuffer> getFramebuffer() const { return m_framebuffer; }

        uint32_t getAttachmentRendererID(Attachment attachment) const
        {
            return m_framebuffer ? m_framebuffer->getColorAttachmentRendererID(static_cast<uint32_t>(attachment)) : 0;
        }

        void bindAttachment(Attachment attachment, uint32_t slot = 0) const
        {
            if (m_framebuffer)
                m_framebuffer->bindColorAttachment(static_cast<uint32_t>(attachment), slot);
        }

        void bindDepthAttachment(uint32_t slot = 0) const
        {
            if (m_framebuffer)
                m_framebuffer->bindDepthAttachment(slot);
        }

        void ensureFramebuffer(uint32_t width, uint32_t height);

    private:
        std::shared_ptr<Pipeline> m_phongPipeline;
        std::shared_ptr<Pipeline> m_pbrPipeline;
        std::shared_ptr<Framebuffer> m_framebuffer;
    };

} // namespace Fermion
