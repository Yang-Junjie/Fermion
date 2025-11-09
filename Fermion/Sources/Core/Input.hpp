#pragma once

#include "Core/KeyCodes.hpp"
#include "Core/MouseCodes.hpp"
#include <glm/glm.hpp>

namespace Fermion
{

    class Input
    {
    public:
        static bool isKeyPressed(KeyCode key);

        static bool isMouseButtonPressed(MouseCode button);
        static glm::vec2 getMousePosition();
        static float getMouseX();
        static float getMouseY();
    };
}
