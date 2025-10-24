#pragma once 
#include "Core/KeyCodes.hpp"
#include "GLFW/glfw3.h"

namespace Fermion{
    inline KeyCode GLFWKeyCodeToFMKeyCode(int keyCode){
        switch(keyCode){
            case GLFW_KEY_A: return KeyCode::A;
            case GLFW_KEY_B: return KeyCode::B;
            case GLFW_KEY_C: return KeyCode::C;
            case GLFW_KEY_D: return KeyCode::D;
            case GLFW_KEY_E: return KeyCode::E;
            case GLFW_KEY_F: return KeyCode::F;
            case GLFW_KEY_G: return KeyCode::G;
            case GLFW_KEY_H: return KeyCode::H;
            case GLFW_KEY_I: return KeyCode::I;
            case GLFW_KEY_J: return KeyCode::J;
            case GLFW_KEY_K: return KeyCode::K;
            case GLFW_KEY_L: return KeyCode::L;
            case GLFW_KEY_M: return KeyCode::M;
            case GLFW_KEY_N: return KeyCode::N;
            case GLFW_KEY_O: return KeyCode::O;
            case GLFW_KEY_P: return KeyCode::P;
            case GLFW_KEY_Q: return KeyCode::Q;
            case GLFW_KEY_R: return KeyCode::R;
            case GLFW_KEY_S: return KeyCode::S;
            case GLFW_KEY_T: return KeyCode::T;
            case GLFW_KEY_U: return KeyCode::U;
            case GLFW_KEY_V: return KeyCode::V;
            case GLFW_KEY_W: return KeyCode::W;
            case GLFW_KEY_X: return KeyCode::X;
            case GLFW_KEY_Y: return KeyCode::Y;
            case GLFW_KEY_Z: return KeyCode::Z;

            case GLFW_KEY_SPACE:    return KeyCode::Space;
            case GLFW_KEY_ENTER:    return KeyCode::Enter;
            case GLFW_KEY_ESCAPE:   return KeyCode::Escape;
            case GLFW_KEY_DELETE:   return KeyCode::Delete;

            case GLFW_KEY_LEFT:     return KeyCode::Left;
            case GLFW_KEY_RIGHT:    return KeyCode::Right;
            case GLFW_KEY_UP:       return KeyCode::Up;
            case GLFW_KEY_DOWN:     return KeyCode::Down;

            case GLFW_KEY_LEFT_SHIFT:    return KeyCode::LeftShift;
            case GLFW_KEY_RIGHT_SHIFT:   return KeyCode::RightShift;
            case GLFW_KEY_LEFT_CONTROL:  return KeyCode::LeftControl;
            case GLFW_KEY_RIGHT_CONTROL: return KeyCode::RightControl;
            case GLFW_KEY_LEFT_ALT:      return KeyCode::LeftAlt;
            case GLFW_KEY_RIGHT_ALT:     return KeyCode::RightAlt;

            default: return KeyCode::None;

        }
    }
}