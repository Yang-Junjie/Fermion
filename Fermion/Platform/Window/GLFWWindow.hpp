#pragma once
#define FERMION_PLATFORM_GLFW
#include "glad/glad.h"
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

        unsigned int getWidth() const override { return m_Data.Width; }
        unsigned int getHeight() const override { return m_Data.Height; }

        void setEventCallback(const EventCallbackFn &callback) override { m_Data.EventCallback = callback; }
        void setVSync(bool enabled) override;
        bool isVSync() const override;

        virtual void *GetNativeWindow() const { return m_Window; }

    private:
        virtual void Init(const WindowProps &props);
        virtual void Shutdown();

    private:
        GLFWwindow *m_Window;
       

        struct WindowData
        {
            std::string Title;
            unsigned int Width, Height;
            bool VSync;

            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };
}