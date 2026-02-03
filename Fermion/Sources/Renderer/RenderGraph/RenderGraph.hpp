#pragma once
#include "RenderGraphCompiler.hpp"
#include "RenderGraphExecutor.hpp"
#include "RenderGraphPass.hpp"
#include "RenderGraphResource.hpp"
#include "RenderGraphResourcePool.hpp"
#include "Renderer/RenderCommandQueue.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace Fermion
{
    class RenderGraph
    {
    public:
        RenderGraph();

        RenderGraphResourceHandle declareResource(const std::string &name, const RenderGraphResourceDesc &desc);
        RenderGraphResourceHandle importResource(const std::string &name, std::shared_ptr<Framebuffer> external);

        void addPass(const std::string &name, std::function<void(PassBuilder &)> setupFunc);

        bool compile();
        void execute(RenderCommandQueue &commandQueue, RendererAPI &api);
        void reset();

        bool lastCompileSucceeded() const { return m_LastCompileSuccess; }
        const std::string &getLastError() const { return m_LastError; }

        std::shared_ptr<Framebuffer> getFramebuffer(RenderGraphResourceHandle handle) const;

    private:
        std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> m_Resources;
        std::vector<RenderGraphPass> m_Passes;
        std::vector<size_t> m_ExecutionOrder;

        RenderGraphCompiler m_Compiler;
        RenderGraphExecutor m_Executor;
        RenderGraphResourcePool m_ResourcePool;

        bool m_Dirty = true;
        bool m_Compiled = false;
        bool m_LastCompileSuccess = true;
        std::string m_LastError;
    };

} // namespace Fermion
