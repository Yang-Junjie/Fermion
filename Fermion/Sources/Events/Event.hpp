#pragma once
#include "fmpch.hpp"
namespace Fermion
{
    enum class EventType : int
    {
        None,

        // window events
        WindowClose,
        WindowResize,
        WindowFocus,
        WindowLostFocus,
        WindowMoved,

        // app events
        AppTick,
        AppUpdate,
        AppRender,

        // key events
        KeyPressed,
        KeyReleased,
        KeyTyped,

        // mouse events
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled

    };

    enum class EventCategory : int
    {
        None = 0,
        EventCategoryApplication = 1 << 0,
        EventCategoryInput = 1 << 1,
        EventCategoryKeyboard = 1 << 2,
        EventCategoryMouse = 1 << 3,
        EventCategoryMouseButton = 1 << 4
    };

    class IEvent
    {
    public:
        bool Handled = false;
        virtual ~IEvent() {}
        virtual EventType getEventType() const = 0;
        virtual const char *getName() const = 0;
        virtual int getCategoryFlags() const = 0;
        virtual std::string toString() const { return getName(); }

        bool isInCategory(EventCategory category) const
        {
            return getCategoryFlags() & static_cast<int>(category);
        }
    };

    class EventDispatcher
    {
    public:
        EventDispatcher(IEvent &event)
            : m_Event(event)
        {
        }

        template <typename T, typename F>
        bool dispatch(const F &func)
        {
            if (m_Event.getEventType() == T::getStaticType())
            {
                m_Event.Handled |= func(static_cast<T &>(m_Event));
                return true;
            }
            return false;
        }

    private:
        IEvent &m_Event;
    };

    inline std::ostream &operator<<(std::ostream &os, const IEvent &e)
    {
        return os << e.toString();
    }
}