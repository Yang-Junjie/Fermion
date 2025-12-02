#include "Core/Application.hpp"

int main()
{

    Fermion::Log::Init("engine.log", Fermion::LogLevel::Debug);
    FM_PROFILE_BEGIN_SESSION("Startup", "FermionProfile-Startup.json");
    auto app = Fermion::createApplication();
    FM_PROFILE_END_SESSION();

    FM_PROFILE_BEGIN_SESSION("Runtime", "FermionProfile-Runtime.json");
    app->run();
    FM_PROFILE_END_SESSION();

    FM_PROFILE_BEGIN_SESSION("Shutdown", "FermionProfile-Shutdown.json");
    delete app;
    FM_PROFILE_END_SESSION();
    return 0;
}
