#pragma once
#include "Engine/Engine.h"
namespace Oxygine
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
        return new Oxygine::GameApp();
    }
}