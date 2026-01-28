#pragma once
#include "RenderContext.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/RenderGraphLegacy.hpp"
#include "Renderer/VertexArray.hpp"
#include <memory>

namespace Fermion
{
    class GBufferRenderer;
    class SSGIRenderer;
    class GTAORenderer;

    class PostProcessRenderer
    {
    public:
        enum class GBufferDebugMode : uint8_t
        {
            None = 0,
            Albedo = 1,
            Normal = 2,
            Material = 3,
            Roughness = 4,
            Metallic = 5,
            AO = 6,
            Emissive = 7,
            Depth = 8,
            ObjectID = 9,
            SSGI = 10,
            GTAO = 11
        };

        PostProcessRenderer();

        // Depth view pass
        void addDepthViewPass(RenderGraphLegacy& renderGraph,
                              const RenderContext& context,
                              const GBufferRenderer* gBuffer,
                              bool useDeferred,
                              float power,
                              ResourceHandle sceneDepth,
                              ResourceHandle lightingResult);

        // GBuffer debug pass
        void addGBufferDebugPass(RenderGraphLegacy& renderGraph,
                                 const RenderContext& context,
                                 const GBufferRenderer& gBuffer,
                                 const SSGIRenderer* ssgi,
                                 const GTAORenderer* gtao,
                                 GBufferDebugMode mode,
                                 float depthPower,
                                 ResourceHandle gBufferHandle,
                                 ResourceHandle sceneDepth,
                                 ResourceHandle ssgiHandle,
                                 ResourceHandle gtaoHandle);

    private:
        std::shared_ptr<Pipeline> m_depthViewPipeline;
        std::shared_ptr<Pipeline> m_debugPipeline;
        std::shared_ptr<VertexArray> m_quadVA;
    };

} // namespace Fermion
