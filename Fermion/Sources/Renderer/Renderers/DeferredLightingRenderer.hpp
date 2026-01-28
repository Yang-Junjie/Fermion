#pragma once
#include "RenderContext.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/RenderGraphLegacy.hpp"
#include "Renderer/VertexArray.hpp"
#include <memory>

namespace Fermion
{
    class GBufferRenderer;
    class SSGIRenderer;
    class GTAORenderer;
    class EnvironmentRenderer;
    class ShadowMapRenderer;

    class DeferredLightingRenderer
    {
    public:
        DeferredLightingRenderer();

        void addPass(RenderGraphLegacy& renderGraph,
                     const RenderContext& context,
                     const GBufferRenderer& gBuffer,
                     const ShadowMapRenderer* shadowRenderer,
                     const SSGIRenderer* ssgiRenderer,
                     const GTAORenderer* gtaoRenderer,
                     EnvironmentRenderer* envRenderer,
                     ResourceHandle lightingResult,
                     bool enableSSGI,
                     bool enableGTAO);

    private:
        std::shared_ptr<Pipeline> m_pipeline;
        std::shared_ptr<VertexArray> m_quadVA;
    };

} // namespace Fermion
