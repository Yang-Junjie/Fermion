#pragma once
#include "RenderContext.hpp"

#include "Renderer/RenderPassQueue.hpp"

#include <memory>

namespace Fermion
{
    class GBufferRenderer;
    class EnvironmentRenderer;
    class ShadowMapRenderer;
    class VertexArray;
    class Pipeline;

    class DeferredLightingRenderer
    {
    public:
        DeferredLightingRenderer();

        void addPass(RenderPassQueue& passQueue,
                     const RenderContext& context,
                     const GBufferRenderer& gBuffer,
                     const ShadowMapRenderer* shadowRenderer,
                     EnvironmentRenderer* envRenderer,
                     ResourceHandle lightingResult);

    private:
        std::shared_ptr<Pipeline> m_pipeline;
        std::shared_ptr<VertexArray> m_quadVA;
    };

} // namespace Fermion
