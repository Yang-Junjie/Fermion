
#include "Core/Application.hpp"
#include "NeutrinoLayer.hpp"

namespace Fermion
{
class Neutrino : public Application {
public:
    Neutrino(const ApplicationSpecification &spec) : Application(spec) {
        Log::Info("Neutrino runtime constructor called");

        pushLayer(std::make_unique<NeutrinoLayer>());
    }

    ~Neutrino() override = default;
};

Application *createApplication(int argc, char **argv) {
    ApplicationSpecification spec;
    spec.name = "Neutrino";
    spec.windowWidth = 1920;
    spec.windowHeight = 1080;
    spec.rendererConfig.ShaderPath = "../Resources/shaders/";
    Log::Info("start preparing to create the Neutrino Application");
    return new Fermion::Neutrino(spec);
}
} // namespace Fermion
