﻿#pragma once
#include "Events/Event.hpp"
#include "Core/Timestep.hpp"
namespace Fermion
{
    class Layer
    {
    public:
        Layer(const std::string &name = "Layer") : m_name(name) {}
        virtual ~Layer() = default;
        
        virtual void onAttach() {};
        virtual void onDetach() {};
        virtual void onUpdate(Timestep dt) {};
        virtual void onEvent(IEvent &event){};
        virtual void onImGuiRender(){};

        const std::string &getName() const { return m_name; }

    protected:
        std::string m_name;
        // Layer比Renderer的生命周期更长，所以Layer中可以持有Renderer的指针
   
    };
}