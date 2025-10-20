#pragma once
#include "Events/Event.hpp"
#include <functional>
#include <cstdint>
namespace Fermion
{
    using EventCallbackFn = std::function<void(IEvent &)>;
    struct WindowProps
    {
        std::string title;
        uint32_t width;
        uint32_t height;

        WindowProps(const std::string &title = "Fermion",
                    uint32_t width = 1600,
                    uint32_t height = 900)
            : title(title), width(width), height(height)
        {
        }
    };
    class IWindow
    {
    public:
        virtual ~IWindow() = default;
        virtual bool isOpen() const = 0;
        virtual void clear() = 0;
        virtual void display() = 0;
        virtual void pollEvents() = 0;
        virtual void OnUpdate() = 0;

        virtual void setVSync(bool enabled) = 0;
        virtual bool isVSync() const = 0;

        virtual void setEventCallback(const EventCallbackFn &callback) = 0;

        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
    };
}