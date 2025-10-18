#pragma once
#include "Core/Window.hpp"
#include "Renderer/Renderer.hpp"
#include "Events/Event.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Core/Log.hpp"
#include "Core/LayerStack.hpp"
#include "Core/Layer.hpp"
#include <memory>

namespace Fermion
{
    class Engine
    {
    public:
        Engine();
        void run();
        void pushLayer(std::unique_ptr<Layer> layer);
        void pushOverlay(std::unique_ptr<Layer> overlay);
        virtual ~Engine() = default;
        virtual void init() = 0;

    private:
        void onEvent(IEvent &event);
        bool onWindowResize(WindowResizeEvent &event);
        bool onWindowClose(WindowCloseEvent &event);

    private:
        bool m_running = true;
        bool m_minimized = false;
        std::unique_ptr<IWindow> m_window;
        std::unique_ptr<IRenderer> m_renderer;

        LayerStack m_layerStack;
    };
    // clinet 实现
    Engine *createEngine();
}