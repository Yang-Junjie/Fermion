#pragma once
#include "Renderer/GraphicsContext.hpp"
#include <GLFW/glfw3.h>
#include "Core/Window.hpp"
namespace Fermion
{
    class GLFWWindow : public IWindow
    {
    public:
        GLFWWindow(const WindowProps &props);
        virtual ~GLFWWindow();

        void OnUpdate() override;

        unsigned int getWidth() const override { return m_data.width; }
        unsigned int getHeight() const override { return m_data.height; }

        void setEventCallback(const EventCallbackFn &callback) override { m_data.eventCallback = callback; }
        void setVSync(bool enabled) override;
        bool isVSync() const override;

        virtual void *getNativeWindow() const override { return m_window; }

    private:
        virtual void Init(const WindowProps &props);
        virtual void Shutdown();

    private:
        GLFWwindow *m_window;
        std::unique_ptr<GraphicsContext> m_context;
        struct WindowData
        {
            std::string title;
            unsigned int width, height;
            bool VSync;

            EventCallbackFn eventCallback;
        };

        WindowData m_data;
    };
}