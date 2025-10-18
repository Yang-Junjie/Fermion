#include "SFMLWindow.h"
#include <SFML/Graphics.hpp>
#include "Core/Log.hpp"

namespace Oxygine
{
    SFMLWindow::SFMLWindow() : m_window(sf::VideoMode(1600, 900), "Oxygine")
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
        default:
            break;
        }
    }

    void SFMLWindow::OnUpdate()
    {
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
}