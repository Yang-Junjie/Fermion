#pragma once

#include "Core/Application.hpp"
#include "NeutrinoLayer.hpp"

namespace Fermion
{
    class Neutrino : public Application
    {
    public:
        Neutrino(const ApplicationSpecification &spec, const std::string_view &projectPath)
            : Application(spec)
        {
            Log::Info("Neutrino runtime constructor called");

            pushLayer(std::make_unique<NeutrinoLayer>(projectPath));
        }

        ~Neutrino() = default;
    };

    Application *createApplication(int argc, char **argv)
    {
        std::string_view projectPath = "../Boson/projects/test.fmproj";
        if (argc > 1)
        {
            projectPath = argv[1];
        }
        ApplicationSpecification spec;
        spec.name = "Neutrino";
        spec.windowWidth = 1920;
        spec.windowHeight = 1080;
        Log::Info("start preparing to create the Neutrino Application");
        return new Fermion::Neutrino(spec, projectPath);
    }
}
