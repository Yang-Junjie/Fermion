#pragma once
#include "RenderGraphPass.hpp"
#include "RenderGraphResource.hpp"
#include "RenderGraphResourcePool.hpp"
#include "Renderer/RenderCommandQueue.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace Fermion
{
    class RenderGraphExecutor
    {
    public:
        void execute(
            const std::vector<RenderGraphPass> &passes,
            const std::vector<size_t> &executionOrder,
            std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> &resources,
            RenderGraphResourcePool &resourcePool,
            RenderCommandQueue &commandQueue,
            RendererAPI &api);

    private:
        void allocateResources(
            std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> &resources,
            RenderGraphResourcePool &resourcePool);

        void releaseResources(
            std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> &resources,
            RenderGraphResourcePool &resourcePool);
    };

} // namespace Fermion
