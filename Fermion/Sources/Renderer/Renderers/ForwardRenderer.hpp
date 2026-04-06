#pragma once
#include "RenderContext.hpp"

#include "Renderer/RenderPassQueue.hpp"
#include <memory>
#include <span>
#include <vector>

namespace Fermion
{
    class EnvironmentRenderer;
    struct MeshDrawCommand;
    class ShadowMapRenderer;
    class Pipeline;

    class ForwardRenderer
    {
    public:
        ForwardRenderer();

        void addPass(RenderPassQueue& passQueue,
                     const RenderContext& context,
                     std::span<const MeshDrawCommand> drawList,
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
        std::shared_ptr<Pipeline> getSkinnedPBRPipeline() const { return m_skinnedPBRPipeline; }

    private:
        std::shared_ptr<Pipeline> m_phongPipeline;
        std::shared_ptr<Pipeline> m_pbrPipeline;
        std::shared_ptr<Pipeline> m_skinnedPBRPipeline;
    };

} // namespace Fermion
