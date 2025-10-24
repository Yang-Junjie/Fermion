#pragma once

#include "Core/Layer.hpp"
#include "Events/Event.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"
#include "GLFW/glfw3.h"
namespace Fermion
{
    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer(void *nativeWindow);
        ~ImGuiLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnEvent(IEvent &e) override;
        virtual void OnImGuiRender() override;
        
        void Begin();
        void End();

        void BlockEvents(bool block) { m_BlockEvents = block; }

    private:
        bool m_BlockEvents = true;
        GLFWwindow *m_Window;
    };
}