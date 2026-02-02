#pragma once
#include "fmpch.hpp"

#include "Core/Timestep.hpp"
#include "Renderer/RendererConfig.hpp"

namespace Fermion
{
    class IWindow;
    class LayerStack;
    class Layer;
    class ImGuiLayer;
    class IEvent;
    class WindowResizeEvent;
    class WindowCloseEvent;

    struct ApplicationSpecification
    {
        std::string name = "Fermion";
        uint32_t windowWidth = 1600, windowHeight = 900;
        RendererConfig rendererConfig = {""};
        bool maximized = false;
    };

    class Application
    {
    public:
        explicit Application(const ApplicationSpecification &spec);
        virtual ~Application();

        void pushLayer(std::unique_ptr<Layer> layer);
        void pushOverlay(std::unique_ptr<Layer> overlay);

        IWindow &getWindow() const
        {
            return *m_window;
        }
        ImGuiLayer *getImGuiLayer() const
        {
            return m_imGuiLayerRaw;
        }

        void close()
        {
            m_running = false;
        }

        void run();
        static Application &get()
        {
            return *s_instance;
        }

        Timestep getTimestep() const
        {
            return m_timestep;
        }

    private:
        void onEvent(IEvent &event);
        bool onWindowResize(const WindowResizeEvent &event);
        bool onWindowClose(const WindowCloseEvent &event);

    private:
        bool m_running = true;
        bool m_minimized = false;

        std::unique_ptr<IWindow> m_window;

        std::unique_ptr<ImGuiLayer> m_imGuiLayer; // 管理生命周期
        ImGuiLayer *m_imGuiLayerRaw = nullptr;    // 供开发者访问
        std::unique_ptr<LayerStack> m_layerStack;

        Timestep m_timestep;
        float m_lastFrameTime = 0.0f;
        static Application *s_instance;
    };
    // client 实现
    Application *createApplication(int argc, char **argv);
} // namespace Fermion
