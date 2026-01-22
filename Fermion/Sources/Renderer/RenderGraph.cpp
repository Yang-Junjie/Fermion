#include "Renderer/RenderGraph.hpp"

namespace Fermion
{
    ResourceHandle RenderGraph::createResource()
    {
        ResourceHandle handle;
        while (!handle.isValid())
            handle = ResourceHandle{};
        return handle;
    }

    RenderGraph::PassHandle RenderGraph::addPass(const RenderGraphPass &pass)
    {
        PassNode node;
        node.Pass = pass;
        node.CommandBuffer = std::make_shared<CommandBuffer>();
        m_Passes.emplace_back(std::move(node));
        m_Dirty = true;
        return m_Passes.size() - 1;
    }

    bool RenderGraph::compile()
    {
        m_ExecutionOrder.clear();
        m_Compiled = false;

        const PassHandle passCount = m_Passes.size();
        if (passCount == 0)
        {
            m_Compiled = true;
            m_Dirty = false;
            m_LastCompileSuccess = true;
            return true;
        }

        bool success = true;

        std::unordered_map<ResourceHandle, PassHandle> producerByResource;
        producerByResource.reserve(passCount * 2);

        auto passLabel = [this](PassHandle handle) -> std::string
        {
            if (handle >= m_Passes.size())
                return std::format("#{}", handle);
            const auto &name = m_Passes[handle].Pass.Name;
            return name.empty() ? std::format("#{}", handle) : name;
        };

        for (PassHandle pass = 0; pass < passCount; ++pass)
        {
            for (const auto &output : m_Passes[pass].Pass.Outputs)
            {
                if (!output.isValid())
                    continue;

                auto [it, inserted] = producerByResource.emplace(output, pass);
                if (!inserted)
                {
                    success = false;
                    Log::Error(std::format("RenderGraph compile error: resource {} has multiple producers ('{}' and '{}')",
                                           output.toString(), passLabel(it->second), passLabel(pass)));
                }
            }
        }

        std::vector<std::vector<PassHandle>> edges(passCount);
        std::vector<uint32_t> indegree(passCount, 0);
        for (PassHandle consumer = 0; consumer < passCount; ++consumer)
        {
            for (const auto &input : m_Passes[consumer].Pass.Inputs)
            {
                if (!input.isValid())
                    continue;

                auto it = producerByResource.find(input);
                if (it == producerByResource.end())
                    continue;

                const PassHandle producer = it->second;
                if (producer == consumer)
                    continue;

                edges[producer].push_back(consumer);
                indegree[consumer]++;
            }
        }

        using ReadyQueue = std::priority_queue<PassHandle, std::vector<PassHandle>, std::greater<PassHandle>>;
        ReadyQueue ready;
        for (PassHandle pass = 0; pass < passCount; ++pass)
        {
            if (indegree[pass] == 0)
                ready.push(pass);
        }

        m_ExecutionOrder.reserve(passCount);
        while (!ready.empty())
        {
            const PassHandle pass = ready.top();
            ready.pop();
            m_ExecutionOrder.push_back(pass);

            for (const PassHandle dependent : edges[pass])
            {
                if (--indegree[dependent] == 0)
                    ready.push(dependent);
            }
        }

        if (m_ExecutionOrder.size() != passCount)
        {
            success = false;

            std::string cyclePasses;
            for (PassHandle pass = 0; pass < passCount; ++pass)
            {
                if (indegree[pass] == 0)
                    continue;

                if (!cyclePasses.empty())
                    cyclePasses += ", ";
                cyclePasses += passLabel(pass);
            }

            Log::Error(std::format("RenderGraph compile error: cycle detected involving: {}", cyclePasses));
        }

        m_LastCompileSuccess = success;
        if (!success)
        {
            FERMION_ASSERT(false, "RenderGraph compilation failed (see log)");

            // Fallback to insertion order in release builds.
            m_ExecutionOrder.clear();
            m_ExecutionOrder.reserve(passCount);
            for (PassHandle pass = 0; pass < passCount; ++pass)
                m_ExecutionOrder.push_back(pass);
        }

        m_Compiled = true;
        m_Dirty = false;
        return success;
    }

    void RenderGraph::execute(RenderCommandQueue &queue, RendererBackend &backend)
    {
        if (m_Dirty || !m_Compiled)
            compile();

        for (const PassHandle pass : m_ExecutionOrder)
        {
            auto &passNode = m_Passes[pass];
            passNode.CommandBuffer->clear();
            if (passNode.Pass.Execute)
                passNode.Pass.Execute(*passNode.CommandBuffer);
            queue.submit(passNode.CommandBuffer);
        }
        queue.flush(backend);
    }

    void RenderGraph::reset()
    {
        m_Passes.clear();
        m_ExecutionOrder.clear();
        m_Dirty = true;
        m_Compiled = false;
        m_LastCompileSuccess = true;
    }

    bool RenderGraph::lastCompileSucceeded() const
    {
        return m_LastCompileSuccess;
    }

} // namespace Fermion
