#include "Core/Application.hpp"
#include "BosonLayer.hpp"
namespace Fermion
{
    class Bonson : public Application
    {
    public:
        Bonson(const ApplicationSpecification& spec) : Application(spec)
        {
            Log::Info("Boson Editor constructor called");
            pushLayer(std::make_unique<BosonLayer>());
        };
        ~Bonson() = default;
    };
    Application *createApplication(int argc, char** argv)
    {
        ApplicationSpecification spec;
        spec.name = "Fermion - Boson";
        spec.windowWidth = 1600;
        spec.windowHeight = 900;
        spec.rendererConfig.ShaderPath = "../Boson/Resources/Shaders/";
        Log::Info("start preparing to create the Application");
        return new Fermion::Bonson(spec);
    }
}