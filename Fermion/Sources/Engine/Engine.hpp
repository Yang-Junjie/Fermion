/*
    Engine.hpp
    本头文件定义了引擎类
*/
#pragma once
#include "fmpch.hpp"

#include "Core/Window.hpp"
#include "Core/Log.hpp"
#include "Core/LayerStack.hpp"
#include "Core/Layer.hpp"

#include "Events/Event.hpp"
#include "Events/ApplicationEvent.hpp"

#include "ImGui/ImGuiLayer.hpp"

#include "Time/Timer.hpp"

#include "Renderer/Buffer.hpp"
#include "Renderer/VertexArray.hpp"
#include "OpenGLShader.hpp"
namespace Fermion
{
    class Engine
    {
    public:
        Engine();
        virtual ~Engine() = default;

        void pushLayer(std::unique_ptr<Layer> layer);
        void pushOverlay(std::unique_ptr<Layer> overlay);

        IWindow &getWindow() { return *m_window; }
        ImGuiLayer *getImGuiLayer() { return m_imGuiLayerRaw; }

        void run();

    private:
        void onEvent(IEvent &event);
        bool onWindowResize(WindowResizeEvent &event);
        bool onWindowClose(WindowCloseEvent &event);

    private:
        bool m_running = true;
        bool m_minimized = false;

        std::unique_ptr<IWindow> m_window;

        std::unique_ptr<ImGuiLayer> m_imGuiLayer; // 管理生命周期
        ImGuiLayer *m_imGuiLayerRaw = nullptr;    // 供开发者访问
        LayerStack m_layerStack;

        std::unique_ptr<ITimer> m_timer;
        float m_lastFrameTime = 0.0f;

        
    };
    // clinet 实现
    Engine *createEngine();
}