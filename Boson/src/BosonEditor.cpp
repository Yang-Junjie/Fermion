#include "Core/Application.hpp"
#include "BosonLayer.hpp"
namespace Fermion
{
    class Bonson : public Application
    {
    public:
        Bonson() : Application("Fermion - Boson")
        {
            Log::Info("Boson Editor constructor called");
            pushLayer(std::make_unique<BosonLayer>());
        };
        ~Bonson() = default;
    };
    Application *createApplication()
    {
        Log::Info("start preparing to create the Application");
        return new Fermion::Bonson();
    }
}