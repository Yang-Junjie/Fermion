/*
    GLFWWindow.hpp
    本头文件定义了 GLFWWindow 类，该类继承自 IWindow 类。
    该类封装了 GLFW 库，并实现了 IWindow 类中的所有方法。
    用于创建 GLFW 窗口、设置窗口属性，和处理窗口事件。
*/
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

        void onUpdate() override;

        unsigned int getWidth() const override { return m_data.width; }
        unsigned int getHeight() const override { return m_data.height; }

        void setEventCallback(const EventCallbackFn &callback) override { m_data.eventCallback = callback; }

        void getWindowPos(int *x, int *y) const override;
        void setWindowPos(int x, int y) override;

        virtual void setMaximized() override;
        virtual void setRestored() override;
        virtual void setMinimized() override;

        void setVSync(bool enabled) override;
        bool isVSync() const override;

        virtual void *getNativeWindow() const override { return m_window; }

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
}