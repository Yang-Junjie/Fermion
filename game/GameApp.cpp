#pragma once
#include "Core/Application.hpp"
#include "GameLayer.hpp"
#include "SandBox2D.hpp"

namespace Fermion
{
    class GameApp : public Application
    {
    public:
        GameApp(const ApplicationSpecification& spec) : Application(spec)
        {
            Log::Info("GameApp constructor called");
            // pushLayer(std::make_unique<GameLayer>());
            pushLayer(std::make_unique<SandBox2D>());
        };
        ~GameApp() = default;
    };
    Application *createApplication(int argc, char** argv)
    {
        Fermion::ApplicationSpecification spec;
        spec.name = "SandBox";
        spec.windowWidth = 1920;
        spec.windowHeight = 1080;
        Log::Info("start preparing to create the Application");
        return new Fermion::GameApp(spec);
    }
}