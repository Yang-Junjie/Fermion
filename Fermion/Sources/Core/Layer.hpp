#pragma once
#include "Events/Event.hpp"

namespace Fermion
{
    class Layer
    {
    public:
        Layer(const std::string &name = "Layer") : m_name(name) {}
        virtual ~Layer() = default;
        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnEvent(IEvent &event) = 0;

        const std::string &getName() const { return m_name; }

    protected:
        std::string m_name;
    };
}