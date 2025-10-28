/*
    GLFWKeyCodes.hpp
    本头文件定义了 GLFW 的按键代码转换成 Fermion 的按键代码的方法
*/

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
    inline int FMKeyCodeToGLFWKeyCode(KeyCode key){
        switch(key){
        case KeyCode::A: return GLFW_KEY_A;
        case KeyCode::B: return GLFW_KEY_B;
        case KeyCode::C: return GLFW_KEY_C;
        case KeyCode::D: return GLFW_KEY_D;
        case KeyCode::E: return GLFW_KEY_E;
        case KeyCode::F: return GLFW_KEY_F;
        case KeyCode::G: return GLFW_KEY_G;
        case KeyCode::H: return GLFW_KEY_H;
        case KeyCode::I: return GLFW_KEY_I;
        case KeyCode::J: return GLFW_KEY_J;
        case KeyCode::K: return GLFW_KEY_K;
        case KeyCode::L: return GLFW_KEY_L;
        case KeyCode::M: return GLFW_KEY_M;
        case KeyCode::N: return GLFW_KEY_N;
        case KeyCode::O: return GLFW_KEY_O;
        case KeyCode::P: return GLFW_KEY_P;
        case KeyCode::Q: return GLFW_KEY_Q;
        case KeyCode::R: return GLFW_KEY_R;
        case KeyCode::S: return GLFW_KEY_S;
        case KeyCode::T: return GLFW_KEY_T;
        case KeyCode::U: return GLFW_KEY_U;
        case KeyCode::V: return GLFW_KEY_V;
        case KeyCode::W: return GLFW_KEY_W;
        case KeyCode::X: return GLFW_KEY_X;
        case KeyCode::Y: return GLFW_KEY_Y;
        case KeyCode::Z: return GLFW_KEY_Z;

        case KeyCode::Space:     return GLFW_KEY_SPACE;
        case KeyCode::Enter:     return GLFW_KEY_ENTER;
        case KeyCode::Escape:    return GLFW_KEY_ESCAPE;
        case KeyCode::Delete:    return GLFW_KEY_DELETE;

        case KeyCode::Left:      return GLFW_KEY_LEFT;
        case KeyCode::Right:     return GLFW_KEY_RIGHT;
        case KeyCode::Up:        return GLFW_KEY_UP;
        case KeyCode::Down:      return GLFW_KEY_DOWN;

        case KeyCode::LeftShift:    return GLFW_KEY_LEFT_SHIFT;
        case KeyCode::RightShift:   return GLFW_KEY_RIGHT_SHIFT;
        case KeyCode::LeftControl:  return GLFW_KEY_LEFT_CONTROL;
        case KeyCode::RightControl: return GLFW_KEY_RIGHT_CONTROL;
        case KeyCode::LeftAlt:      return GLFW_KEY_LEFT_ALT;
        case KeyCode::RightAlt:     return GLFW_KEY_RIGHT_ALT;

        default: return GLFW_KEY_UNKNOWN;
    }
    }
}