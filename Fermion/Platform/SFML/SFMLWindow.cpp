#include "SFMLWindow.hpp"
#include <SFML/Graphics.hpp>
#include "Core/Log.hpp"
#include "Core/KeyCodes.hpp"
#include "SFMLKeyCodes.hpp"
#include "SFMLMouseCodes.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

namespace Fermion
{
    SFMLWindow::SFMLWindow() : m_window(sf::VideoMode({1600, 900}), "Fermion")
    {
        Init(WindowProps());
        Log::Info("SFML Window created with default title and size 1600x900");
    }

    SFMLWindow::SFMLWindow(const WindowProps &props) : m_window(sf::VideoMode({props.width, props.height}), props.title)
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
        std::optional<sf::Event> event;
        while ((event = m_window.pollEvent()))
        {
            ImGui::SFML::ProcessEvent(m_window, *event);
            processEvent(*event);
        }
    }

    void SFMLWindow::processEvent(const sf::Event &event)
    {
        if (!m_data.EventCallback)
            return;

        if (event.is<sf::Event::Closed>())
        {
            WindowCloseEvent e;
            m_data.EventCallback(e);
            m_window.close();
        }
        else if (const auto *keyPress = event.getIf<sf::Event::KeyPressed>())
        {
            KeyCode keyCode = SFMLKeyCodeToOKeyCode(keyPress->code);
            bool isRepeat = m_heldKeys.contains(keyPress->code);
            m_heldKeys.insert(keyPress->code);

            KeyPressedEvent e(keyCode, isRepeat);
            m_data.EventCallback(e);
            Log::Info("Key Pressed: " + std::to_string(static_cast<int>(keyCode)) + (isRepeat ? " (repeat)" : ""));
        }
        else if (const auto *keyRelease = event.getIf<sf::Event::KeyReleased>())
        {
            KeyCode keyCode = SFMLKeyCodeToOKeyCode(keyRelease->code);
            m_heldKeys.erase(keyRelease->code);

            KeyReleasedEvent e(keyCode);
            m_data.EventCallback(e);
            Log::Info("Key Released: " + std::to_string(static_cast<int>(keyCode)));
        }
        else if (const auto *resized = event.getIf<sf::Event::Resized>())
        {
            m_data.width = resized->size.x;
            m_data.height = resized->size.y;
            WindowResizeEvent e(resized->size.x, resized->size.y);
            m_data.EventCallback(e);
        }
        else if (const auto *mouseMove = event.getIf<sf::Event::MouseMoved>())
        {
            MouseMovedEvent e(static_cast<float>(mouseMove->position.x), static_cast<float>(mouseMove->position.y));
            m_data.EventCallback(e);
            Log::Trace("Mouse Moved: " + std::to_string(mouseMove->position.x) + ", " + std::to_string(mouseMove->position.y));
        }
        else if (const auto *mousePress = event.getIf<sf::Event::MouseButtonPressed>())
        {
            MouseCode mouseCode = SFMLMouseCodeToOMouseCode(mousePress->button);
            MouseButtonPressedEvent e(mouseCode);
            m_data.EventCallback(e);
            Log::Info("Mouse Button Pressed: " + std::to_string(static_cast<int>(mouseCode)));
        }
        else if (const auto *mouseRelease = event.getIf<sf::Event::MouseButtonReleased>())
        {
            MouseCode mouseCode = SFMLMouseCodeToOMouseCode(mouseRelease->button);
            MouseButtonReleasedEvent e(mouseCode);
            m_data.EventCallback(e);
            Log::Info("Mouse Button Released: " + std::to_string(static_cast<int>(mouseCode)));
        }
        else if (const auto *mouseScroll = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            MouseScrolledEvent e(static_cast<float>(mouseScroll->delta), 0.0f);
            m_data.EventCallback(e);
            Log::Info("Mouse Wheel Scrolled: " + std::to_string(mouseScroll->delta));
        }
    }
    void SFMLWindow::clear()
    {
        m_window.clear();
    }

    void SFMLWindow::OnUpdate()
    {
        pollEvents();
        display();
        clear();
    }
    void SFMLWindow::display()
    {
        m_window.display();
    }

    sf::RenderWindow &SFMLWindow::getWindow()
    {
        return m_window;
    }

    void SFMLWindow::setVSync(bool enabled)
    {
        if (enabled)
        {
            m_window.setFramerateLimit(60);
        }
        else
        {
            m_window.setFramerateLimit(0);
        }
        m_data.VSync = enabled;
        Log::Info("VSync " + std::string(enabled ? "enabled" : "disabled"));
    }

    bool SFMLWindow::isVSync() const
    {
        return m_data.VSync;
    }
    SFMLWindow::~SFMLWindow()
    {
    }
}