#pragma once
#include "fmpch.hpp"
namespace Fermion
{
    struct RenderPass
    {
        std::string Name;
        std::function<void()> Execute;
    };
}