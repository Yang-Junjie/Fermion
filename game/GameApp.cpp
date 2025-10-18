#pragma once
#include "Engine/Engine.h"
namespace Fermion
{
    class GameApp : public Engine
    {
    public:
        GameApp(){
            Log::Info("GameApp constructor called");
        };
        ~GameApp() = default;
    };
    Engine *createEngine()
    {
        return new Fermion::GameApp();
    }
}