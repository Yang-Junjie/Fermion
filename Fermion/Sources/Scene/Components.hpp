#pragma once
#include <glm/glm.hpp>
namespace Fermion
{

    struct TransformComponent
    {
        glm::mat4 transform{1.0f};
        TransformComponent() = default;
        TransformComponent(const glm::mat4 &transform) : transform(transform) {}
        TransformComponent(const TransformComponent &transform) = default;

        operator glm::mat4 &()
        {
            return transform;
        }
        operator const glm::mat4 &() const
        {
            return transform;
        }
    };

    struct SpriteRendererComponent
    {
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        SpriteRendererComponent() = default;
        SpriteRendererComponent(const glm::vec4 &color) : color(color) {}
        SpriteRendererComponent(const SpriteRendererComponent& ) = default;
    };
}