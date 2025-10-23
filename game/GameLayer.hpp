#pragma once
#include "Core/Layer.hpp"
#include "Core/LayerStack.hpp"
#include "Renderer/Renderer.hpp"
#include "Core/Log.hpp"
#include "imgui.h"

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
        virtual void OnUpdate(Timestep dt) override
        {
            Log::Trace("GameLayer OnUpdate called");

        }
        virtual void OnEvent(IEvent &event) override
        {
            Log::Trace("GameLayer OnEvent called: " + event.toString());
        }
        virtual void OnImGuiRender() override
        {
            ImGui::Begin("Hello, world!");
            ImGui::Button("button");
            ImGui::ShowDemoWindow();
            ImGui::End();
        }

    private:
    };
}