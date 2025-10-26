#pragma once
#include "Core/Layer.hpp"
#include "Core/Log.hpp"
#include "imgui.h"
#include "Renderer/RenderCommand.hpp"

namespace Fermion
{
    class GameLayer : public Layer
    {
    public:
        GameLayer(const std::string &name = "GameLayer") : Layer(name)
        {
            
        }
        virtual ~GameLayer() = default;

        virtual void onAttach() override {}
        virtual void onDetach() override {}
        virtual void onUpdate(Timestep dt) override
        {
            Log::Trace("GameLayer OnUpdate called");
            
        }
        virtual void onEvent(IEvent &event) override
        {
            Log::Trace("GameLayer OnEvent called: " + event.toString());
        }
        virtual void onImGuiRender() override
        {
            ImGui::ShowDemoWindow();
        }

    private:
    };
}