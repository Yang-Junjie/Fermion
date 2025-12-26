#pragma once
#include <memory>
#include <vector>

#include "Renderer/RenderPass.hpp"
#include "Renderer/RenderCommandQueue.hpp"
#include "Renderer/RendererBackend.hpp"

namespace Fermion
{
    class RenderGraph
    {
    public:
        void AddPass(const RenderPass &pass)
        {
            PassNode node;
            node.Pass = pass;
            node.CommandBuffer = std::make_shared<CommandBuffer>();
            m_Passes.emplace_back(std::move(node));
        }

        void Execute(RenderCommandQueue &queue, RendererBackend &backend)
        {
            for (auto &passNode : m_Passes)
            {
                passNode.CommandBuffer->Clear();
                if (passNode.Pass.Execute)
                    passNode.Pass.Execute(*passNode.CommandBuffer);
                queue.Submit(passNode.CommandBuffer);
            }
            queue.Flush(backend);
        }

        void Reset()
        {
            m_Passes.clear();
        }

    private:
        struct PassNode
        {
            RenderPass Pass;
            std::shared_ptr<CommandBuffer> CommandBuffer;
        };

        std::vector<PassNode> m_Passes;
    };

}
