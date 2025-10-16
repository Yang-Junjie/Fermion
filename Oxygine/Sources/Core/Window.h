#pragma once
namespace Oxygine
{
    class IWindow
    {
    public:
        virtual ~IWindow() = default;
        virtual bool isOpen() const = 0;
        virtual void pollEvents() = 0;
        virtual void clear() = 0;
        virtual void display() = 0;
    };
}