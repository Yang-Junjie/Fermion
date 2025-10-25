
/*
    本文件是ImGui层
*/
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

        virtual void onAttach() override;
        virtual void onDetach() override;

        virtual void onEvent(IEvent &e) override;
        virtual void onImGuiRender() override;
        
        void begin();
        void end();

        void blockEvents(bool block) { m_blockEvents = block; }

    private:
        bool m_blockEvents = true;
        GLFWwindow *m_window;
    };
}