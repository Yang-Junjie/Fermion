#pragma once
#include "Events/Event.hpp"
namespace Oxygine
{
    class WindowResizeEvent : public IEvent
    {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_width(width), m_height(height) {}

        unsigned int GetWidth() const { return m_width; }
        unsigned int GetHeight() const { return m_height; }

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_width << ", " << m_height;
            return ss.str();
        }
        static EventType getStaticType() { return EventType::WindowResize; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "WindowResize"; }

        virtual int getCategoryFlags() const override
        {
            return static_cast<int>(EventCategory::EventCategoryApplication);
        }

    private:
        unsigned int m_width, m_height;
    };

    class WindowCloseEvent : public IEvent
    {
    public:
        WindowCloseEvent() = default;

        static EventType getStaticType() { return EventType::WindowClose; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "WindowClose"; }

        virtual int getCategoryFlags() const override
        {
            return static_cast<int>(EventCategory::EventCategoryApplication);
        }
    };

    class AppTickEvent : public IEvent
    {
    public:
        AppTickEvent() = default;

        static EventType getStaticType() { return EventType::AppTick; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "AppTick"; }

        virtual int getCategoryFlags() const override
        {
            return static_cast<int>(EventCategory::EventCategoryApplication);
        }
    };

    class AppUpdateEvent : public IEvent
    {
    public:
        AppUpdateEvent() = default;

        static EventType getStaticType() { return EventType::AppUpdate; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "AppUpdate"; }

        virtual int getCategoryFlags() const override
        {
            return static_cast<int>(EventCategory::EventCategoryApplication);
        }
    };

    class AppRenderEvent : public IEvent
    {
    public:
        AppRenderEvent() = default;

        static EventType getStaticType() { return EventType::AppRender; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "AppRender"; }

        virtual int getCategoryFlags() const override
        {
            return static_cast<int>(EventCategory::EventCategoryApplication);
        }
    };
}