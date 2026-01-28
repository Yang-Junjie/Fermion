#pragma once
#include "RenderContext.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/RenderDrawCommand.hpp"
#include "Renderer/RenderGraphLegacy.hpp"
#include <memory>
#include <vector>

namespace Fermion
{
    class EnvironmentRenderer;
    class ShadowMapRenderer;

    class ForwardRenderer
    {
    public:
        ForwardRenderer();

        void addPass(RenderGraphLegacy& renderGraph,
                     const RenderContext& context,
                     const std::vector<MeshDrawCommand>& drawList,
                     const ShadowMapRenderer* shadowRenderer,
                     EnvironmentRenderer* envRenderer,
                     ResourceHandle shadowMap,
                     ResourceHandle sceneDepth,
                     ResourceHandle lightingResult,
                     bool transparentOnly,
                     uint32_t* geometryDrawCalls,
                     uint32_t* iblDrawCalls);

        std::shared_ptr<Pipeline> getPhongPipeline() const { return m_phongPipeline; }
        std::shared_ptr<Pipeline> getPBRPipeline() const { return m_pbrPipeline; }

    private:
        std::shared_ptr<Pipeline> m_phongPipeline;
        std::shared_ptr<Pipeline> m_pbrPipeline;
    };

} // namespace Fermion
