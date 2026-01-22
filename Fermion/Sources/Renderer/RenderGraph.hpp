#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
#include "Core/Log.hpp"
#include "Core/UUID.hpp"
#include "Renderer/RenderCommandQueue.hpp"
#include "Renderer/RendererBackend.hpp"

namespace Fermion
{
    using ResourceHandle = UUID;
    struct RenderGraphPass
    {
        std::string Name;
        std::vector<ResourceHandle> Inputs;
        std::vector<ResourceHandle> Outputs;
        std::function<void(CommandBuffer &)> Execute;
    };
    class RenderGraph
    {
    public:
        using PassHandle = size_t;

        static ResourceHandle createResource();
        PassHandle addPass(const RenderGraphPass &pass);
        bool compile();
        void execute(RenderCommandQueue &queue, RendererBackend &backend);
        void reset();
        bool lastCompileSucceeded() const;

    private:
        struct PassNode
        {
            RenderGraphPass Pass;
            std::shared_ptr<CommandBuffer> commandBuffer;
        };

        std::vector<PassNode> m_Passes;
        std::vector<PassHandle> m_ExecutionOrder;
        bool m_Dirty = true;
        bool m_Compiled = false;
        bool m_LastCompileSuccess = true;
    };

} // namespace Fermion
