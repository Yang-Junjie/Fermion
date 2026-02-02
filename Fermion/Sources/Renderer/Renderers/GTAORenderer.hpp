#pragma once
#include "RenderContext.hpp"
#include "Renderer/RenderGraphLegacy.hpp"
#include <memory>

namespace Fermion
{
    class GBufferRenderer;
    class Framebuffer;
    class Pipeline;
    class VertexArray;

    class GTAORenderer
    {
    public:
        struct Settings
        {
            float intensity = 1.0f;
            float radius = 1.0f;
            float bias = 0.03f;
            float power = 1.25f;
            int sliceCount = 6;
            int stepCount = 6;
        };

        GTAORenderer();

        void addPass(RenderGraphLegacy& renderGraph,
                     const RenderContext& context,
                     const GBufferRenderer& gBuffer,
                     ResourceHandle gtaoOutput,
                     const Settings& settings);

        std::shared_ptr<Framebuffer> getResultFramebuffer() const { return m_framebuffer; }

        void ensureFramebuffer(uint32_t width, uint32_t height);

    private:
        std::shared_ptr<Pipeline> m_pipeline;
        std::shared_ptr<VertexArray> m_quadVA;
        std::shared_ptr<Framebuffer> m_framebuffer;
    };

} // namespace Fermion
