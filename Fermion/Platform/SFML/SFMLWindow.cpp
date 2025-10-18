#include "SFMLWindow.hpp"
#include <SFML/Graphics.hpp>
#include "Core/Log.hpp"
#include "Core/KeyCodes.hpp"
#include "SFMLKeyCodes.hpp"
#include "SFMLMouseCodes.hpp"

namespace Fermion
{
    SFMLWindow::SFMLWindow() : m_window(sf::VideoMode(1600, 900), "Fermion")
    {
        Init(WindowProps());
        Log::Info("SFML Window created with default title and size 1600x900");
    }

    SFMLWindow::SFMLWindow(const WindowProps &props) : m_window(sf::VideoMode(props.width, props.height), props.title)
    {
        Init(props);
        Log::Info("SFML Window created with title: " + props.title + " and size: " + std::to_string(props.width) + "x" + std::to_string(props.height));
    }

    bool SFMLWindow::isOpen() const
    {
        return m_window.isOpen();
    }

    void SFMLWindow::Init(const WindowProps &props)
    {
        m_data.title = props.title;
        m_data.width = props.width;
        m_data.height = props.height;
    }

    void SFMLWindow::pollEvents()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            processEvent(event);
        }
    }

    void SFMLWindow::processEvent(const sf::Event &event)
    {
        if (!m_data.EventCallback)
            return;

        switch (event.type)
        {
        case sf::Event::Closed:
        {
            WindowCloseEvent e;
            m_data.EventCallback(e);
            m_window.close();
            break;
        }
        case sf::Event::Resized:
        {
            m_data.width = event.size.width;
            m_data.height = event.size.height;
            WindowResizeEvent e(event.size.width, event.size.height);
            m_data.EventCallback(e);
            break;
        }
        case sf::Event::KeyPressed:
        {
            KeyCode keyCode = SFMLKeyCodeToOKeyCode(event.key.code);
            bool isRepeat = m_heldKeys.contains(event.key.code);
            m_heldKeys.insert(event.key.code);

            KeyPressedEvent e(keyCode, isRepeat);
            m_data.EventCallback(e);
            Log::Info("Key Pressed: " + std::to_string(static_cast<int>(keyCode)) + (isRepeat ? " (repeat)" : ""));
            break;
        }
        case sf::Event::KeyReleased:
        {
            KeyCode keyCode = SFMLKeyCodeToOKeyCode(event.key.code);
            m_heldKeys.erase(event.key.code);

            KeyReleasedEvent e(keyCode);
            m_data.EventCallback(e);
            Log::Info("Key Released: " + std::to_string(static_cast<int>(keyCode)));
            break;
        }
        case sf::Event::MouseMoved:
        {
            MouseMovedEvent e(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
            m_data.EventCallback(e);
            Log::Trace("Mouse Moved: " + std::to_string(event.mouseMove.x) + ", " + std::to_string(event.mouseMove.y));
            break;
        }
        case sf::Event::MouseButtonPressed:
        {
            MouseCode mouseCode = SFMLMouseCodeToOMouseCode(event.mouseButton.button);
            MouseButtonPressedEvent e(mouseCode);
            m_data.EventCallback(e);
            Log::Info("Mouse Button Pressed: " + std::to_string(static_cast<int>(mouseCode)));
            break;
        }
        case sf::Event::MouseButtonReleased:
        {
            MouseCode mouseCode = SFMLMouseCodeToOMouseCode(event.mouseButton.button);
            MouseButtonReleasedEvent e(mouseCode);
            m_data.EventCallback(e);
            Log::Info("Mouse Button Released: " + std::to_string(static_cast<int>(mouseCode)));
            break;
        }
        case sf::Event::MouseWheelScrolled:
        {
            MouseScrolledEvent e(static_cast<float>(event.mouseWheelScroll.delta), 0.0f);
            m_data.EventCallback(e);
            Log::Info("Mouse Wheel Scrolled: " + std::to_string(event.mouseWheelScroll.delta));
            break;
        }
        default:
            break;
        }
    }

    void SFMLWindow::onUpdate()
    {
        pollEvents();
    }

    void SFMLWindow::clear()
    {
        m_window.clear();
    }

    void SFMLWindow::display()
    {
        m_window.display();
    }

    sf::RenderWindow &SFMLWindow::get()
    {
        return m_window;
    }

    void SFMLWindow::setVSync(bool enabled)
    {
        m_window.setVerticalSyncEnabled(enabled);
        m_data.VSync = enabled;
    }

    bool SFMLWindow::isVSync() const
    {
        return m_data.VSync;
    }
}