#pragma once
#include "RenderGraph/RenderGraph.hpp"
#include <functional>
#include <string>
#include <vector>

namespace Fermion
{
    using ResourceHandle = RenderGraphResourceHandle;

    struct LegacyRenderGraphPass
    {
        std::string Name;
        std::vector<ResourceHandle> Inputs;
        std::vector<ResourceHandle> Outputs;
        std::function<void(RenderCommandQueue &)> Execute;
    };

    class RenderGraphLegacy
    {
    public:
        using PassHandle = size_t;

        ResourceHandle createResource();
        PassHandle addPass(const LegacyRenderGraphPass &pass);
        bool compile();
        void execute(RenderCommandQueue &queue, RendererAPI &api);
        void reset();
        bool lastCompileSucceeded() const;

    private:
        RenderGraph m_Graph;
        std::vector<LegacyRenderGraphPass> m_LegacyPasses;
    };

} // namespace Fermion
