#include "SFMLWindow.h"
#include <SFML/Graphics.hpp>
#include "Core/Log.hpp"
namespace Oxygine
{
    SFMLWindow::SFMLWindow() : m_window(sf::VideoMode(800, 600), "SFML")
    {
        Log::Info("SFML Window created with size 800x600");
    }

    bool SFMLWindow::isOpen() const { return m_window.isOpen(); }

    void SFMLWindow::pollEvents()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                m_window.close();
                Log::Info("SFML Window closed");
            }
        }
    }

    void SFMLWindow::clear() { m_window.clear(); }
    void SFMLWindow::display() { m_window.display(); }

    sf::RenderWindow &SFMLWindow::get() { return m_window; }
}