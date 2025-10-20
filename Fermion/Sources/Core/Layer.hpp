#pragma once
#include "Events/Event.hpp"
#include "Renderer/Renderer.hpp"
#include "Core/Timestep.hpp"
namespace Fermion
{
    class Layer
    {
    public:
        Layer(const std::string &name = "Layer") : m_name(name) {}
        virtual ~Layer() = default;
        void setRenderer(IRenderer *renderer) { m_renderer = renderer; }

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate(Timestep dt) = 0;
        virtual void OnEvent(IEvent &event) = 0;

        const std::string &getName() const { return m_name; }

    protected:
        IRenderer *getRenderer() const
        {
            return m_renderer;
        }
        std::string m_name;
        // Layer比Renderer的生命周期更长，所以Layer中可以持有Renderer的指针
    private:
        IRenderer *m_renderer = nullptr;
    };
}