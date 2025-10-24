#pragma once
#include "Events/Event.hpp"
#include "fmpch.hpp"
namespace Fermion
{
    struct WindowProps
    {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowProps(const std::string &title = "Fermion Engine",
                    uint32_t width = 1600,
                    uint32_t height = 900)
            : Title(title), Width(width), Height(height)
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

        // virtual void *getNativeWindow() const = 0;

        static std::unique_ptr<IWindow> create(const WindowProps &props = WindowProps());
    };
}