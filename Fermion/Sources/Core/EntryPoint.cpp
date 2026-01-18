#include "Core/Application.hpp"

int main(int argc, char **argv)
{
    Fermion::Log::Init("engine.log", Fermion::LogLevel::Debug);
    FM_PROFILE_BEGIN_SESSION("Startup", "FermionProfile-Startup.json");
    const auto app = Fermion::createApplication(argc, argv);
    FM_PROFILE_END_SESSION();

    FM_PROFILE_BEGIN_SESSION("Runtime", "FermionProfile-Runtime.json");
    app->run();
    FM_PROFILE_END_SESSION();

    FM_PROFILE_BEGIN_SESSION("Shutdown", "FermionProfile-Shutdown.json");
    delete app;
    FM_PROFILE_END_SESSION();
    return 0;
}
