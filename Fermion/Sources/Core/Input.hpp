#pragma once

#include "Core/KeyCodes.hpp"
#include "Core/MouseCodes.hpp"
#include <glm/glm.hpp>

namespace Fermion
{
    enum class CursorMode
    {
        Normal = 0,
        Hidden = 1,
        Disabled = 2 // 锁定并隐藏光标
    };

    class Input
    {
    public:
        static bool isKeyPressed(KeyCode key);

        static bool isMouseButtonPressed(MouseCode button);

        static glm::vec2 getMousePosition();

        static void setCursorMode(CursorMode mode);

        static void setMousePosition(float x, float y);
        static void setRawMouseMotion(bool enabled);

        static float getMouseX();

        static float getMouseY();
    };
} // namespace Fermion
