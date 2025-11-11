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

        std::function<void()> instantiateFunction;
        std::function<void()> destroyInstanceFunction;

        std::function<void(ScriptableEntity *instance)> onCreateFunction;
        std::function<void(ScriptableEntity *instance)> onDestroyFunction;
        std::function<void(ScriptableEntity *instance, Timestep ts)> onUpdateFunction;

        template <typename T>
        void bind()
        {
            instantiateFunction = [this]()
            { instance = new T(); };
            destroyInstanceFunction = [this]()
            { delete static_cast<T*>(instance);instance = nullptr; };

            onCreateFunction = [this](ScriptableEntity *instance)
            { static_cast<T *>(instance)->onCreate(); };
            onDestroyFunction = [this](ScriptableEntity *instance)
            { static_cast<T *>(instance)->onDestroy(); };
            onUpdateFunction = [this](ScriptableEntity *instance, Timestep ts)
            { static_cast<T *>(instance)->onUpdate(ts); };
        }
    };

}