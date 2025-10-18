#pragma once
#include "Events/Event.hpp"
#include "Core/MouseCodes.h"

namespace Fermion
{
    class MouseMovedEvent : public IEvent
    {
    public:
        MouseMovedEvent(const float x, const float y)
            : m_mouseX(x), m_mouseY(y) {}

        float getX() const { return m_mouseX; }
        float getY() const { return m_mouseY; }

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "MouseMovedEvent: " << m_mouseX << ", " << m_mouseY;
            return ss.str();
        }
        static EventType getStaticType() { return EventType::MouseMoved; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "MouseMoved"; }

        virtual int getCategoryFlags() const override
        {
            return static_cast<int>(EventCategory::EventCategoryMouse) | static_cast<int>(EventCategory::EventCategoryInput);
        }

    private:
        float m_mouseX, m_mouseY;
    };

    class MouseScrolledEvent : public IEvent
    {
    public:
        MouseScrolledEvent(const float xOffset, const float yOffset)
            : m_xOffset(xOffset), m_yOffset(yOffset) {}

        float getXOffset() const { return m_xOffset; }
        float getYOffset() const { return m_yOffset; }

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "MouseScrolledEvent: " << getXOffset() << ", " << getYOffset();
            return ss.str();
        }
        static EventType getStaticType() { return EventType::MouseScrolled; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "MouseScrolled"; }

        virtual int getCategoryFlags() const override
        {
            return static_cast<int>(EventCategory::EventCategoryMouse) | static_cast<int>(EventCategory::EventCategoryInput);
        }

    private:
        float m_xOffset, m_yOffset;
    };

    class MouseButtonEvent : public IEvent
    {
    public:
        MouseCode GetMouseButton() const { return m_button; }
        virtual int getCategoryFlags() const override
        {
            return static_cast<int>(EventCategory::EventCategoryMouse) | static_cast<int>(EventCategory::EventCategoryInput) | static_cast<int>(EventCategory::EventCategoryMouseButton);
        }

    protected:
        MouseButtonEvent(const MouseCode button)
            : m_button(button) {}

        MouseCode m_button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(const MouseCode button)
            : MouseButtonEvent(button) {}

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "MouseButtonPressedEvent: " << static_cast<int>(m_button);
            return ss.str();
        }
        static EventType getStaticType() { return EventType::MouseButtonPressed; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "MouseButtonPressed"; }
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(const MouseCode button)
            : MouseButtonEvent(button) {}

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "MouseButtonReleasedEvent: " << static_cast<int>(m_button);
            return ss.str();
        }

        static EventType getStaticType() { return EventType::MouseButtonReleased; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "MouseButtonReleased"; }
    };
}
