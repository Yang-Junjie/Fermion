#pragma once
#include "RenderContext.hpp"
#include "Renderer/RenderDrawCommand.hpp"
#include "Renderer/RenderGraphLegacy.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Fermion
{
    class GBufferRenderer;
    class Pipeline;
    class VertexArray;

    class OutlineRenderer
    {
    public:
        struct Settings
        {
            glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            float lineWidth = 2.0f; 
        };

        OutlineRenderer();

        void addPass(RenderGraphLegacy& renderGraph,
                     const RenderContext& context,
                     const GBufferRenderer* gBuffer,
                     const std::vector<MeshDrawCommand>& drawList,
                     const std::vector<int>& outlineIDs,
                     const Settings& settings,
                     ResourceHandle gBufferHandle,
                     ResourceHandle sceneDepth,
                     ResourceHandle lightingResult);

    };

} // namespace Fermion
