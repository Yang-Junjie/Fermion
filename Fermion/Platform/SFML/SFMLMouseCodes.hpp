#pragma once
#include <SFML/Window/Mouse.hpp>
#include "Core/MouseCodes.hpp"
namespace Fermion{
    MouseCode SFMLMouseCodeToOMouseCode(int sfmlMouseCode){
        switch (sfmlMouseCode)
        {
        case sf::Mouse::Left:      return MouseCode::Left;
        case sf::Mouse::Right:     return MouseCode::Right;
        case sf::Mouse::Middle:    return MouseCode::Middle;
        case sf::Mouse::XButton1:  return MouseCode::XButton1;
        case sf::Mouse::XButton2:  return MouseCode::XButton2;
        default: return MouseCode::None;
        }
    }
}