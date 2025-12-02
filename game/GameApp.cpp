#pragma once
#include "Core/Application.hpp"
#include "GameLayer.hpp"
#include "SandBox2D.hpp"

namespace Fermion
{
    class GameApp : public Application
    {
    public:
        GameApp(): Application("SandBox")
        {
            Log::Info("GameApp constructor called");
            // pushLayer(std::make_unique<GameLayer>());
            pushLayer(std::make_unique<SandBox2D>());
        };
        ~GameApp() = default;
    };
    Application *createApplication()
    {
        Log::Info("start preparing to create the Application");
        return new Fermion::GameApp();
    }
}