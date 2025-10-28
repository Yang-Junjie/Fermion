/*
    GLFWMouseCodes.hpp
    本文件定义并实现了将 GLFW 鼠标按键代码转换成 FMouseCode 的方法
*/
#pragma once
#include "Core/MouseCodes.hpp"
#include "GLFW/glfw3.h"

namespace Fermion
{
    inline MouseCode GLFWMouseCodeToFMouseCode(int mouseCode)
    {
        switch (mouseCode)
        {
        case GLFW_MOUSE_BUTTON_1:
            return MouseCode::Left;
        case GLFW_MOUSE_BUTTON_2:
            return MouseCode::Right;
        case GLFW_MOUSE_BUTTON_3:
            return MouseCode::Middle;
        case GLFW_MOUSE_BUTTON_4:
            return MouseCode::XButton1;
        case GLFW_MOUSE_BUTTON_5:
            return MouseCode::XButton2;
        default:
            return MouseCode::None;
        }
    }
    inline int FMMouseCodeToGLFWMouseCode(MouseCode mousecode)
    {
        switch (mousecode)
        {
        case MouseCode::Left:
            return GLFW_MOUSE_BUTTON_1;
        case MouseCode::Right:
            return GLFW_MOUSE_BUTTON_2;
        case MouseCode::Middle:
            return GLFW_MOUSE_BUTTON_3;
        case MouseCode::XButton1:
            return GLFW_MOUSE_BUTTON_4;
        }
    }
}