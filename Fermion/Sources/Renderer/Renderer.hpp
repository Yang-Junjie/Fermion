#pragma once
#include <string>
#include "glm.hpp"
namespace Fermion
{
    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;
        virtual void drawRect(const glm::vec2 &pos, const glm::vec2 &size, const glm::vec4 &color) = 0;
        virtual void drawImage(const std::string &texturePath, const glm::vec2 &pos) = 0;
    };
}