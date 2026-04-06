#pragma once
#include "Core/UUID.hpp"
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace Fermion
{
    class RendererAPI;

    using ResourceHandle = UUID;

    struct RenderPass
    {
        std::string Name;
        std::vector<ResourceHandle> Inputs;
        std::vector<ResourceHandle> Outputs;
        std::function<void(RendererAPI &)> Execute;
    };

    class RenderPassQueue
    {
    public:
        using PassHandle = size_t;

        ResourceHandle createResource();
        PassHandle addPass(const RenderPass &pass);
        void execute(RendererAPI &api);
        void reset();

    private:
        std::vector<ResourceHandle> m_Resources;
        std::vector<RenderPass> m_Passes;
    };

} // namespace Fermion
