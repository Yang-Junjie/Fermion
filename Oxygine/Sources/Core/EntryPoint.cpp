#include "Engine/Engine.h"

int main()
{
    Oxygine::Log::Init("engine.log", Oxygine::LogLevel::Debug);
    auto engine = Oxygine::createEngine();
    engine->run();
    delete engine;
    return 0;
}
