#pragma once

#include "Core/Application.hpp"
#include "NeutrinoLayer.hpp"

namespace Fermion
{
    class Neutrino : public Application
    {
    public:
        Neutrino()
            : Application("Neutrino")
        {
            Log::Info("Neutrino runtime constructor called");

            std::filesystem::path projectPath = "../Boson/projects/test.fmproj";
            pushLayer(std::make_unique<NeutrinoLayer>(projectPath));
        }

        ~Neutrino() = default;
    };

    Application* createApplication()
    {
        Log::Info("start preparing to create the Neutrino Application");
        return new Fermion::Neutrino();
    }
}

