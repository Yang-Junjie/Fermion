#include "RenderGraphCompiler.hpp"
#include "Core/Log.hpp"
#include <format>
#include <queue>
#include <unordered_map>

namespace Fermion
{
    RenderGraphCompileResult RenderGraphCompiler::compile(
        const std::vector<RenderGraphPass> &passes,
        const std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> &resources)
    {
        RenderGraphCompileResult result;

        if (passes.empty())
        {
            result.success = true;
            return result;
        }

        if (!validatePasses(passes, resources, result.errorMessage))
        {
            result.success = false;
            return result;
        }

        bool hasCycle = false;
        result.executionOrder = topologicalSort(passes, hasCycle, result.errorMessage);

        if (hasCycle || result.executionOrder.size() != passes.size())
        {
            result.success = false;
            if (result.errorMessage.empty())
                result.errorMessage = "Topological sort failed";
            return result;
        }

        result.success = true;
        return result;
    }

    bool RenderGraphCompiler::validatePasses(
        const std::vector<RenderGraphPass> &passes,
        const std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> &resources,
        std::string &errorMessage)
    {
        std::unordered_map<RenderGraphResourceHandle, size_t> producerByResource;

        for (size_t passIndex = 0; passIndex < passes.size(); ++passIndex)
        {
            const auto &pass = passes[passIndex];

            for (const auto &output : pass.getOutputs())
            {
                if (!output.isValid())
                    continue;

                auto [it, inserted] = producerByResource.emplace(output, passIndex);
                if (!inserted)
                {
                    errorMessage = std::format("Resource has multiple producers: '{}' and '{}'",
                                               getPassLabel(passes, it->second),
                                               getPassLabel(passes, passIndex));
                    return false;
                }
            }
        }

        return true;
    }

    std::vector<size_t> RenderGraphCompiler::topologicalSort(
        const std::vector<RenderGraphPass> &passes,
        bool &hasCycle,
        std::string &errorMessage)
    {
        const size_t passCount = passes.size();
        std::unordered_map<RenderGraphResourceHandle, size_t> producerByResource;

        for (size_t pass = 0; pass < passCount; ++pass)
        {
            for (const auto &output : passes[pass].getOutputs())
            {
                if (output.isValid())
                    producerByResource[output] = pass;
            }
        }

        std::vector<std::vector<size_t>> edges(passCount);
        std::vector<uint32_t> indegree(passCount, 0);

        for (size_t consumer = 0; consumer < passCount; ++consumer)
        {
            for (const auto &input : passes[consumer].getInputs())
            {
                if (!input.isValid())
                    continue;

                auto it = producerByResource.find(input);
                if (it == producerByResource.end())
                    continue;

                const size_t producer = it->second;
                if (producer == consumer)
                    continue;

                edges[producer].push_back(consumer);
                indegree[consumer]++;
            }
        }

        using ReadyQueue = std::priority_queue<size_t, std::vector<size_t>, std::greater<size_t>>;
        ReadyQueue ready;

        for (size_t pass = 0; pass < passCount; ++pass)
        {
            if (indegree[pass] == 0)
                ready.push(pass);
        }

        std::vector<size_t> executionOrder;
        executionOrder.reserve(passCount);

        while (!ready.empty())
        {
            const size_t pass = ready.top();
            ready.pop();
            executionOrder.push_back(pass);

            for (const size_t dependent : edges[pass])
            {
                if (--indegree[dependent] == 0)
                    ready.push(dependent);
            }
        }

        if (executionOrder.size() != passCount)
        {
            hasCycle = true;
            std::string cyclePasses;
            for (size_t pass = 0; pass < passCount; ++pass)
            {
                if (indegree[pass] == 0)
                    continue;

                if (!cyclePasses.empty())
                    cyclePasses += ", ";
                cyclePasses += getPassLabel(passes, pass);
            }
            errorMessage = std::format("Cycle detected involving: {}", cyclePasses);
        }
        else
        {
            hasCycle = false;
        }

        return executionOrder;
    }

    std::string RenderGraphCompiler::getPassLabel(const std::vector<RenderGraphPass> &passes, size_t index) const
    {
        if (index >= passes.size())
            return std::format("#{}", index);

        const auto &name = passes[index].getName();
        return name.empty() ? std::format("#{}", index) : name;
    }

} // namespace Fermion
