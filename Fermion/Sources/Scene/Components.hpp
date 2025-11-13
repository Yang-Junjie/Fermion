#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/SceneCamera.hpp"
#include "Scene/ScriptableEntity.hpp"
#include "fmpch.hpp"
#include "Core/Timestep.hpp"
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
        glm::vec3 translation{0.0f, 0.0f, 0.0f};
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};

        TransformComponent() = default;
        TransformComponent(const glm::vec3 &translation) : translation(translation) {}
        TransformComponent(const TransformComponent &transform) = default;

        glm::mat4 getTransform() const
        {
            glm::mat4 Rotation = glm::rotate(glm::mat4(1.0f), rotation.x, {1.0f, 0.0f, 0.0f});
            Rotation = glm::rotate(Rotation, rotation.y, {0.0f, 1.0f, 0.0f});
            Rotation = glm::rotate(Rotation, rotation.z, {0.0f, 0.0f, 1.0f});
            return glm::translate(glm::mat4(1.0f), translation) * Rotation * glm::scale(glm::mat4(1.0f), scale);
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
