#pragma once
#include "fmpch.hpp"
#include "Renderer/CommandBuffer.hpp"
namespace Fermion
{
    struct RenderPass
    {
        std::string Name;
        std::function<void(CommandBuffer &)> Execute;
    };
}
