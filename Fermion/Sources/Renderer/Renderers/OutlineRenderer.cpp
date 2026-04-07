#include "OutlineRenderer.hpp"
#include "GBufferRenderer.hpp"
#include "Renderer2D.hpp"
#include "SceneRenderer.hpp"

namespace Fermion
{
    OutlineRenderer::OutlineRenderer()
    {
    }

    void OutlineRenderer::addPass(RenderPassQueue& passQueue,
                                   const RenderContext& context,
                                   const GBufferRenderer* gBuffer,
                                   std::span<const MeshDrawCommand> drawList,
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

       
        RenderPass pass;
        pass.Name = "OutlinePass";
        pass.Inputs = {lightingResult};
        pass.Execute = [drawList, uniqueIDs = std::move(uniqueIDs), settings](RendererAPI&)
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
                    Renderer2D::Get().drawAABB(cmd.aabb, cmd.transform, settings.color, cmd.objectID);
                }
            }
        };
        passQueue.addPass(pass);
    }

} // namespace Fermion
