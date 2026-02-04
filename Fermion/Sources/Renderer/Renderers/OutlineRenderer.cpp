#include "OutlineRenderer.hpp"
#include "GBufferRenderer.hpp"
#include "Renderer2DCompat.hpp"

namespace Fermion
{
    OutlineRenderer::OutlineRenderer()
    {
    }

    void OutlineRenderer::addPass(RenderGraphLegacy& renderGraph,
                                   const RenderContext& context,
                                   const GBufferRenderer* gBuffer,
                                   const std::vector<MeshDrawCommand>& drawList,
                                   const std::vector<int>& outlineIDs,
                                   const Settings& settings,
                                   ResourceHandle gBufferHandle,
                                   ResourceHandle sceneDepth,
                                   ResourceHandle lightingResult)
    {
        std::vector<int> allOutlineIDs = outlineIDs;
        for (const auto& cmd : drawList)
        {
            if (cmd.drawOutline && cmd.visible)
            {
                allOutlineIDs.push_back(cmd.objectID);
            }
        }

        // 去重
        std::vector<int> uniqueIDs;
        uniqueIDs.reserve(allOutlineIDs.size());
        for (int id : allOutlineIDs)
        {
            if (id < 0)
                continue;

            bool exists = false;
            for (int existing : uniqueIDs)
            {
                if (existing == id)
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
                uniqueIDs.push_back(id);
        }

        if (uniqueIDs.empty())
            return;

       
        LegacyRenderGraphPass pass;
        pass.Name = "OutlinePass";
        pass.Inputs = {lightingResult};
        pass.Execute = [&drawList, uniqueIDs = std::move(uniqueIDs), settings](RenderCommandQueue& queue)
        {
            for (const auto& cmd : drawList)
            {
                if (!cmd.visible)
                    continue;

                bool shouldOutline = false;
                for (int id : uniqueIDs)
                {
                    if (id == cmd.objectID)
                    {
                        shouldOutline = true;
                        break;
                    }
                }

                if (shouldOutline)
                {
                    Renderer2DCompat::drawAABB(cmd.aabb, cmd.transform, settings.color, cmd.objectID);
                }
            }
        };
        renderGraph.addPass(pass);
    }

} // namespace Fermion
