#pragma once
#include <SFML/Window/Keyboard.hpp>
#include "Core/KeyCodes.hpp"
namespace Fermion
{
    KeyCode SFMLKeyCodeToOKeyCode(int sfmlKeyCode){
        switch (sfmlKeyCode)
        {
        case sf::Keyboard::A:  return KeyCode::A;
        case sf::Keyboard::B:  return KeyCode::B;
        case sf::Keyboard::C:  return KeyCode::C;
        case sf::Keyboard::D:  return KeyCode::D;
        case sf::Keyboard::E:  return KeyCode::E;
        case sf::Keyboard::F:  return KeyCode::F;
        case sf::Keyboard::G:  return KeyCode::G;
        case sf::Keyboard::H:  return KeyCode::H;
        case sf::Keyboard::I:  return KeyCode::I;
        case sf::Keyboard::J:  return KeyCode::J;
        case sf::Keyboard::K:  return KeyCode::K;
        case sf::Keyboard::L:  return KeyCode::L;
        case sf::Keyboard::M:  return KeyCode::M;
        case sf::Keyboard::N:  return KeyCode::N;
        case sf::Keyboard::O:  return KeyCode::O;
        case sf::Keyboard::P:  return KeyCode::P;
        case sf::Keyboard::Q:  return KeyCode::Q;
        case sf::Keyboard::R:  return KeyCode::R;
        case sf::Keyboard::S:  return KeyCode::S;
        case sf::Keyboard::T:  return KeyCode::T;
        case sf::Keyboard::U:  return KeyCode::U;
        case sf::Keyboard::V:  return KeyCode::V;
        case sf::Keyboard::W:  return KeyCode::W;
        case sf::Keyboard::X:  return KeyCode::X;
        case sf::Keyboard::Y:  return KeyCode::Y;
        case sf::Keyboard::Z:  return KeyCode::Z;

        case sf::Keyboard::Down:    return KeyCode::Down;
        case sf::Keyboard::Up:      return KeyCode::Up;
        case sf::Keyboard::Left:    return KeyCode::Left;
        case sf::Keyboard::Right:   return KeyCode::Right;

        case sf::Keyboard::Space:   return KeyCode::Space;
        case sf::Keyboard::Enter:   return KeyCode::Enter;
        case sf::Keyboard::Delete:  return KeyCode::Delete;
        case sf::Keyboard::Escape:  return KeyCode::Escape;

        case sf::Keyboard::LControl:    return KeyCode::LeftControl;
        case sf::Keyboard::RControl:    return KeyCode::RightControl;
        case sf::Keyboard::LAlt:        return KeyCode::LeftAlt;
        case sf::Keyboard::RAlt:        return KeyCode::RightAlt;
        case sf::Keyboard::LShift:      return KeyCode::LeftShift;
        case sf::Keyboard::RShift:      return KeyCode::RightShift;
        
        default: return KeyCode::None;
        }
    }
}