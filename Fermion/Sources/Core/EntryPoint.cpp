#include "Engine/Engine.hpp"

int main()
{
    Fermion::Log::Init("engine.log", Fermion::LogLevel::Debug);
    auto engine = Fermion::createEngine();
    engine->init();
    engine->run();
    delete engine;
    return 0;
}
