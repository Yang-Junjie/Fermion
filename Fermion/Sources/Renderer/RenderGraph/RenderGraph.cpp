#include "RenderGraph.hpp"
#include "Core/Log.hpp"

namespace Fermion
{
    RenderGraph::RenderGraph() = default;

    RenderGraphResourceHandle RenderGraph::declareResource(const std::string &name, const RenderGraphResourceDesc &desc)
    {
        auto resource = std::make_shared<RenderGraphResource>(name, desc);
        auto handle = resource->getHandle();
        m_Resources[handle] = resource;
        m_Dirty = true;
        return handle;
    }

    RenderGraphResourceHandle RenderGraph::importResource(const std::string &name, std::shared_ptr<Framebuffer> external)
    {
        auto resource = std::make_shared<RenderGraphResource>(name, external);
        auto handle = resource->getHandle();
        m_Resources[handle] = resource;
        m_Dirty = true;
        return handle;
    }

    void RenderGraph::addPass(const std::string &name, std::function<void(PassBuilder &)> setupFunc)
    {
        PassBuilder builder(name);
        setupFunc(builder);
        m_Passes.push_back(builder.build());
        m_Dirty = true;
    }

    bool RenderGraph::compile()
    {
        m_ExecutionOrder.clear();
        m_Compiled = false;

        if (m_Passes.empty())
        {
            m_Compiled = true;
            m_Dirty = false;
            m_LastCompileSuccess = true;
            m_LastError.clear();
            return true;
        }

        auto result = m_Compiler.compile(m_Passes, m_Resources);

        m_LastCompileSuccess = result.success;
        m_LastError = result.errorMessage;

        if (!result.success)
        {
            Log::Error("RenderGraph compile error: " + result.errorMessage);
            FERMION_ASSERT(false, "RenderGraph compilation failed");

            m_ExecutionOrder.clear();
            for (size_t i = 0; i < m_Passes.size(); ++i)
                m_ExecutionOrder.push_back(i);
        }
        else
        {
            m_ExecutionOrder = std::move(result.executionOrder);
        }

        m_Compiled = true;
        m_Dirty = false;
        return result.success;
    }

    void RenderGraph::execute(RenderCommandQueue &commandQueue, RendererBackend &backend)
    {
        if (m_Dirty || !m_Compiled)
            compile();

        m_Executor.execute(m_Passes, m_ExecutionOrder, m_Resources, m_ResourcePool, commandQueue, backend);
    }

    void RenderGraph::reset()
    {
        m_Passes.clear();
        m_Resources.clear();
        m_ExecutionOrder.clear();
        m_ResourcePool.reset();
        m_Dirty = true;
        m_Compiled = false;
        m_LastCompileSuccess = true;
        m_LastError.clear();
    }

    std::shared_ptr<Framebuffer> RenderGraph::getFramebuffer(RenderGraphResourceHandle handle) const
    {
        auto it = m_Resources.find(handle);
        if (it != m_Resources.end())
            return it->second->getFramebuffer();
        return nullptr;
    }

} // namespace Fermion
