/*
    本文件描述了键盘事件
*/
#pragma once
#include "Events/Event.hpp"
#include "Core/KeyCodes.hpp"
namespace Fermion
{

    class KeyEvent : public IEvent
    {
    public:
        KeyCode getKeyCode() const { return m_keyCode; }

        virtual int getCategoryFlags() const override { return static_cast<int>(EventCategory::EventCategoryKeyboard) |
                                                               static_cast<int>(EventCategory::EventCategoryInput); }

    protected:
        KeyEvent(const KeyCode keycode)
            : m_keyCode(keycode) {}

        KeyCode m_keyCode;
    };
    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(const KeyCode keycode, bool isRepeat = false)
            : KeyEvent(keycode), m_isRepeat(isRepeat) {}

        bool isRepeat() const { return m_isRepeat; }

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << static_cast<int>(m_keyCode) << " (repeat = " << m_isRepeat << ")";
            return ss.str();
        }
        static EventType getStaticType() { return EventType::KeyPressed; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "KeyPressed"; }

    private:
        bool m_isRepeat;
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(const KeyCode keycode)
            : KeyEvent(keycode) {}

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << static_cast<int>(m_keyCode);
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
            ss << "KeyTypedEvent: " << static_cast<int>(m_keyCode);
            return ss.str();
        }
        static EventType getStaticType() { return EventType::KeyTyped; }
        virtual EventType getEventType() const override { return getStaticType(); }
        virtual const char *getName() const override { return "KeyTyped"; }
    };
}
