#pragma once
#include <SFML/Window/Mouse.hpp>
#include "Core/MouseCodes.hpp"
namespace Fermion{
    MouseCode SFMLMouseCodeToOMouseCode(sf::Mouse::Button sfmlMouseCode){
        switch (sfmlMouseCode)
        {
        case sf::Mouse::Button::Left:      return MouseCode::Left;
        case sf::Mouse::Button::Right:     return MouseCode::Right;
        case sf::Mouse::Button::Middle:    return MouseCode::Middle;
        case sf::Mouse::Button::Extra1:  return MouseCode::XButton1;
        case sf::Mouse::Button::Extra2:  return MouseCode::XButton2;
        default: return MouseCode::None;
        }
    }
}