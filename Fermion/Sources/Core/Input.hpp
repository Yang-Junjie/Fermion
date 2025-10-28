#pragma once

#include "Core/KeyCodes.hpp"
#include "Core/MouseCodes.hpp"

#include "glm.hpp"

namespace Fermion
{

    class Input
    {
    public:
        static bool IsKeyPressed(KeyCode key);

        static bool IsMouseButtonPressed(MouseCode button);
        static glm::vec2 GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();
    };
}
