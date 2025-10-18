#pragma once
#include "Core/Layer.hpp"
#include "Core/LayerStack.hpp"
#include "Renderer/Renderer.hpp"
#include "Core/Log.hpp"

namespace Fermion
{
    class GameLayer : public Layer
    {
    public:
        GameLayer(const std::string &name = "GameLayer") : Layer(name)
        {
        }
        virtual ~GameLayer() = default;

        virtual void OnAttach() override {}
        virtual void OnDetach() override {}
        virtual void OnUpdate() override
        {
            Log::Trace("GameLayer OnUpdate called");
            getRenderer()->drawRect({50, 50}, {200, 200}, {0.0f, 1.0f, 0.0f, 1.0f});
        }
        virtual void OnEvent(IEvent &event) override
        {
            Log::Trace("GameLayer OnEvent called: " + event.toString());
        }

    private:
    };
}