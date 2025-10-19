#pragma once
#include <SFML/Window/Keyboard.hpp>
#include "Core/KeyCodes.hpp"
namespace Fermion
{
    KeyCode SFMLKeyCodeToOKeyCode(sf::Keyboard::Key sfmlKeyCode){
        switch (sfmlKeyCode)
        {
        case sf::Keyboard::Key::A:  return KeyCode::A;
        case sf::Keyboard::Key::B:  return KeyCode::B;
        case sf::Keyboard::Key::C:  return KeyCode::C;
        case sf::Keyboard::Key::D:  return KeyCode::D;
        case sf::Keyboard::Key::E:  return KeyCode::E;
        case sf::Keyboard::Key::F:  return KeyCode::F;
        case sf::Keyboard::Key::G:  return KeyCode::G;
        case sf::Keyboard::Key::H:  return KeyCode::H;
        case sf::Keyboard::Key::I:  return KeyCode::I;
        case sf::Keyboard::Key::J:  return KeyCode::J;
        case sf::Keyboard::Key::K:  return KeyCode::K;
        case sf::Keyboard::Key::L:  return KeyCode::L;
        case sf::Keyboard::Key::M:  return KeyCode::M;
        case sf::Keyboard::Key::N:  return KeyCode::N;
        case sf::Keyboard::Key::O:  return KeyCode::O;
        case sf::Keyboard::Key::P:  return KeyCode::P;
        case sf::Keyboard::Key::Q:  return KeyCode::Q;
        case sf::Keyboard::Key::R:  return KeyCode::R;
        case sf::Keyboard::Key::S:  return KeyCode::S;
        case sf::Keyboard::Key::T:  return KeyCode::T;
        case sf::Keyboard::Key::U:  return KeyCode::U;
        case sf::Keyboard::Key::V:  return KeyCode::V;
        case sf::Keyboard::Key::W:  return KeyCode::W;
        case sf::Keyboard::Key::X:  return KeyCode::X;
        case sf::Keyboard::Key::Y:  return KeyCode::Y;
        case sf::Keyboard::Key::Z:  return KeyCode::Z;

        case sf::Keyboard::Key::Down:    return KeyCode::Down;
        case sf::Keyboard::Key::Up:      return KeyCode::Up;
        case sf::Keyboard::Key::Left:    return KeyCode::Left;
        case sf::Keyboard::Key::Right:   return KeyCode::Right;

        case sf::Keyboard::Key::Space:   return KeyCode::Space;
        case sf::Keyboard::Key::Enter:   return KeyCode::Enter;
        case sf::Keyboard::Key::Delete:  return KeyCode::Delete;
        case sf::Keyboard::Key::Escape:  return KeyCode::Escape;

        case sf::Keyboard::Key::LControl:    return KeyCode::LeftControl;
        case sf::Keyboard::Key::RControl:    return KeyCode::RightControl;
        case sf::Keyboard::Key::LAlt:        return KeyCode::LeftAlt;
        case sf::Keyboard::Key::RAlt:        return KeyCode::RightAlt;
        case sf::Keyboard::Key::LShift:      return KeyCode::LeftShift;
        case sf::Keyboard::Key::RShift:      return KeyCode::RightShift;
        
        default: return KeyCode::None;
        }
    }
}