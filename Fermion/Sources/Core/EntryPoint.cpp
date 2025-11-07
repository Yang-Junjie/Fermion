#include "Core/Engine.hpp"

int main()
{

    Fermion::Log::Init("engine.log", Fermion::LogLevel::Debug);
    FM_PROFILE_BEGIN_SESSION("Startup", "FermionProfile-Startup.json");
    auto engine = Fermion::createEngine();
    FM_PROFILE_END_SESSION();

    FM_PROFILE_BEGIN_SESSION("Runtime", "FermionProfile-Runtime.json");
    engine->run();
    FM_PROFILE_END_SESSION();

    FM_PROFILE_BEGIN_SESSION("Shutdown", "FermionProfile-Shutdown.json");
    delete engine;
    FM_PROFILE_END_SESSION();
    return 0;
}
