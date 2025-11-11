#pragma once
#include <glm/glm.hpp>
#include "Renderer/SceneCamera.hpp"
#include "Scene/ScriptableEntity.hpp"
#include "fmpch.hpp"
#include "Core/TimeStep.hpp"
namespace Fermion
{
    struct TagComponent
    {
        std::string tag;
        TagComponent() = default;
        TagComponent(const std::string &tag) : tag(tag) {}
        TagComponent(const TagComponent &tag) = default;
    };

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
        SpriteRendererComponent(const SpriteRendererComponent &) = default;
    };

    struct CameraComponent
    {
        SceneCamera camera;
        bool primary = true;
        bool fixedAspectRatio = false;
        CameraComponent() = default;
        CameraComponent(const glm::mat4 &projection) : camera(projection) {}
        CameraComponent(const CameraComponent &camera) = default;
    };

    struct NativeScriptComponent
    {
        ScriptableEntity *instance = nullptr;

        ScriptableEntity *(*instantiateScript)();
        void (*destroyScript)(NativeScriptComponent *);

        template <typename T>
        void bind()
        {
            instantiateScript = []()
            {
                return static_cast<ScriptableEntity *>(new T());
            };
            destroyScript = [](NativeScriptComponent *nsc)
            {
                delete nsc->instance;
                nsc->instance = nullptr;
            };
        }
    };

}