//
// Created by oxygen on 2025/12/30.
//

#include "Core/Application.hpp"
#include "LauncherLayer.hpp"

namespace Fermion {
    class LauncherApp : public Application {
    public:
        explicit LauncherApp(const ApplicationSpecification &spec) : Application(spec) {
            Log::Info("Singularity constructor called");

            pushLayer(std::make_unique<LauncherLayer>());
        }

        ~LauncherApp() override = default;
    };

    Application *createApplication(int argc, char **argv) {
        ApplicationSpecification spec;
        spec.name = "Fermion - Launcher";
        spec.windowWidth = 1280;
        spec.windowHeight = 720;
        Log::Info("start preparing to create the LauncherApp Application");
        return new Fermion::LauncherApp(spec);
    }
} // namespace Fermion
