#pragma once
#include "Engine/Engine.hpp"
#include "GameLayer.hpp"

namespace Fermion
{
    class GameApp : public Engine
    {
    public:
        GameApp()
        {
            Log::Info("GameApp constructor called");
            pushLayer(std::make_unique<GameLayer>());
        };
        ~GameApp() = default;
    };
    Engine *createEngine()
    {
        return new Fermion::GameApp();
    }
}