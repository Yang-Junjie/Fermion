#pragma once
#include <string>
namespace Oxygine
{
    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;
        virtual void drawImage(const std::string &texturePath, float x, float y) = 0;
    };
}