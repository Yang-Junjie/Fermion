/*
    本文件是ImGui层
*/
#pragma once

#include "Core/Layer.hpp"
#include "Events/Event.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"
#include <GLFW/glfw3.h>

namespace Fermion {
    class ImGuiLayer : public Layer {
    public:
        explicit ImGuiLayer(void *nativeWindow);

        ~ImGuiLayer() override = default;

        void onAttach() override;

        void onDetach() override;

        void onEvent(IEvent &e) override;

        void onImGuiRender() override;

        void begin();

        void end();

        void blockEvents(const bool block) {
            m_blockEvents = block;
        }

        void setDarkThemeColors();

    private:
        bool m_blockEvents = true;
        GLFWwindow *m_window;
    };
} // namespace Fermion
