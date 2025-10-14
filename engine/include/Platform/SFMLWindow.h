#pragma once
#include "Engine/IWindow.h"
#include <SFML/Graphics.hpp>

class SFMLWindow : public IWindow {
    sf::RenderWindow m_window;
public:
    SFMLWindow();
    bool isOpen() const override;
    void pollEvents() override;
    void clear() override;
    void display() override;
    sf::RenderWindow& get();
};
