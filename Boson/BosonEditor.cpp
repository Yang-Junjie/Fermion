#pragma once
#include "Core/Engine.hpp"
#include "BosonLayer.hpp"
namespace Fermion
{
    class Bonson : public Engine
    {
    public:
        Bonson() : Engine("Boson")
        {
            Log::Info("Boson Editor constructor called");
            pushLayer(std::make_unique<BosonLayer>());
        };
        ~Bonson() = default;
    };
    Engine *createEngine()
    {
        Log::Info("start preparing to create the engine");
        return new Fermion::Bonson();
    }
}