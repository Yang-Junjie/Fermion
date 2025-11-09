/*
    Window.hpp
    本文件定义了窗口接口类
*/
#pragma once
#include "Events/Event.hpp"
#include "fmpch.hpp"
namespace Fermion
{
    struct WindowProps
    {
        std::string title;
        uint32_t width;
        uint32_t height;

        WindowProps(const std::string &title = "Fermion Engine",
                    uint32_t width = 1280,
                    uint32_t height = 720)
            : title(title), width(width), height(height)
        {
        }
    };

    class IWindow
    {
    public:
        using EventCallbackFn = std::function<void(IEvent &)>;

        virtual ~IWindow() = default;

        virtual void OnUpdate() = 0;

        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;

        virtual void setEventCallback(const EventCallbackFn &callback) = 0;
        virtual void setVSync(bool enabled) = 0;
        virtual bool isVSync() const = 0;

        virtual void *getNativeWindow() const = 0;

        static std::unique_ptr<IWindow> create(const WindowProps &props = WindowProps());
    };
}