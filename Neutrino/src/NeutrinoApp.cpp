#pragma once

#include "Core/Engine.hpp"
#include "NeutrinoLayer.hpp"

namespace Fermion
{
    class Neutrino : public Engine
    {
    public:
        Neutrino()
            : Engine("Neutrino")
        {
            Log::Info("Neutrino runtime constructor called");

            std::filesystem::path projectPath = "../Boson/projects/test.fmproj";
            pushLayer(std::make_unique<::NeutrinoLayer>(projectPath));
        }

        ~Neutrino() = default;
    };

    Engine* createEngine()
    {
        Log::Info("start preparing to create the Neutrino engine");
        return new Fermion::Neutrino();
    }
}

