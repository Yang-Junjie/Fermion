#include "Platform/SFMLWindow.h"
#include <SFML/Graphics.hpp>
namespace Oxygine
{
    SFMLWindow::SFMLWindow() : m_window(sf::VideoMode(800, 600), "SFML") {}

    bool SFMLWindow::isOpen() const { return m_window.isOpen(); }

    void SFMLWindow::pollEvents()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                m_window.close();
        }
    }

    void SFMLWindow::clear() { m_window.clear(); }
    void SFMLWindow::display() { m_window.display(); }

    sf::RenderWindow &SFMLWindow::get() { return m_window; }
}