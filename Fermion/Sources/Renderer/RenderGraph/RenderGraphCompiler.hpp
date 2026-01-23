#pragma once
#include "RenderGraphPass.hpp"
#include "RenderGraphResource.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace Fermion
{
    struct RenderGraphCompileResult
    {
        bool success = false;
        std::vector<size_t> executionOrder;
        std::string errorMessage;
    };

    class RenderGraphCompiler
    {
    public:
        RenderGraphCompileResult compile(
            const std::vector<RenderGraphPass> &passes,
            const std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> &resources);

    private:
        bool validatePasses(
            const std::vector<RenderGraphPass> &passes,
            const std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<RenderGraphResource>> &resources,
            std::string &errorMessage);

        std::vector<size_t> topologicalSort(
            const std::vector<RenderGraphPass> &passes,
            bool &hasCycle,
            std::string &errorMessage);

        std::string getPassLabel(const std::vector<RenderGraphPass> &passes, size_t index) const;
    };

} // namespace Fermion
