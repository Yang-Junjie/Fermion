#pragma once
#include "Core/Window.hpp"

#include <GLFW/glfw3.h>
namespace Fermion
{
    class GLFWWindow : public IWindow
    {
    public:
        GLFWWindow(const WindowProps &props);
        virtual ~GLFWWindow();

        void onUpdate() override;

        unsigned int getWidth() const override
        {
            return m_data.width;
        }
        unsigned int getHeight() const override
        {
            return m_data.height;
        }

        void setEventCallback(const EventCallbackFn &callback) override
        {
            m_data.eventCallback = callback;
        }

        void getWindowPos(int *x, int *y) const override;
        void setWindowPos(int x, int y) override;

        virtual void setMaximized() override;
        virtual void setRestored() override;
        virtual void setMinimized() override;

        void setVSync(bool enabled) override;
        bool isVSync() const override;

        DeviceInfo getDeviceInfo() const override;

        virtual void *getNativeWindow() const override
        {
            return m_window;
        }

    private:
        virtual void init(const WindowProps &props);
        virtual void shutdown();

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
} // namespace Fermion