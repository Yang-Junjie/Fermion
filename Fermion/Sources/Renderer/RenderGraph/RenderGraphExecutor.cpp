#include "RenderGraphExecutor.hpp"
#include "PassContext.hpp"

namespace Fermion
{
    void RenderGraphExecutor::execute(
        const std::vector<RenderGraphPass> &passes,
        const std::vector<size_t> &executionOrder,
        std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> &resources,
        RenderGraphResourcePool &resourcePool,
        RenderCommandQueue &commandQueue,
        RendererAPI &api)
    {
        allocateResources(resources, resourcePool);

        std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<Framebuffer>> framebufferMap;
        for (auto &[handle, resource] : resources)
        {
            if (resource->getFramebuffer())
                framebufferMap[handle] = resource->getFramebuffer();
        }

        // 执行所有 pass，录制命令到 commandQueue
        for (size_t passIndex : executionOrder)
        {
            const auto &pass = passes[passIndex];

            if (!pass.shouldExecute())
                continue;

            PassContext context(commandQueue, framebufferMap);

            if (pass.getExecuteFunc())
                pass.getExecuteFunc()(context);
        }

        // 统一执行所有录制的命令
        commandQueue.flush(api);
        releaseResources(resources, resourcePool);
    }

    void RenderGraphExecutor::allocateResources(
        std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> &resources,
        RenderGraphResourcePool &resourcePool)
    {
        for (auto &[handle, resource] : resources)
        {
            if (resource->isExternal() || resource->getFramebuffer())
                continue;

            auto framebuffer = resourcePool.acquireFramebuffer(resource->getDesc());
            resource->setFramebuffer(framebuffer);
        }
    }

    void RenderGraphExecutor::releaseResources(
        std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> &resources,
        RenderGraphResourcePool &resourcePool)
    {
        for (auto &[handle, resource] : resources)
        {
            if (resource->isExternal() || !resource->isTransient())
                continue;

            if (auto framebuffer = resource->getFramebuffer())
            {
                resourcePool.releaseFramebuffer(framebuffer);
                resource->setFramebuffer(nullptr);
            }
        }
    }

} // namespace Fermion
