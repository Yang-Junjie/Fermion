#pragma once
#include "Events/Event.hpp"
#include "Core/KeyCodes.hpp"
namespace Fermion
{

    class KeyEvent : public IEvent
    {
    public:
        KeyCode GetKeyCode() const { return m_KeyCode; }

        virtual int getCategoryFlags() const override { return static_cast<int>(EventCategory::EventCategoryKeyboard) |
                                                               static_cast<int>(EventCategory::EventCategoryInput); }

    protected:
        KeyEvent(const KeyCode keycode)
            : m_KeyCode(keycode) {}

        KeyCode m_KeyCode;
    };
    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(const KeyCode keycode, bool isRepeat = false)
            : KeyEvent(keycode), m_IsRepeat(isRepeat) {}

        bool IsRepeat() const { return m_IsRepeat; }

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << static_cast<int>(m_KeyCode) << " (repeat = " << m_IsRepeat << ")";
            return ss.str();
        }
        static EventType getStaticType() { return EventType::KeyPressed; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "KeyPressed"; }

    private:
        bool m_IsRepeat;
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(const KeyCode keycode)
            : KeyEvent(keycode) {}

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << static_cast<int>(m_KeyCode);
            return ss.str();
        }
        static EventType getStaticType() { return EventType::KeyReleased; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "KeyReleased"; }
    };

    class KeyTypedEvent : public KeyEvent
    {
    public:
        KeyTypedEvent(const KeyCode keycode)
            : KeyEvent(keycode) {}

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "KeyTypedEvent: " << static_cast<int>(m_KeyCode);
            return ss.str();
        }
        static EventType getStaticType() { return EventType::KeyTyped; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "KeyTyped"; }
    };
}
