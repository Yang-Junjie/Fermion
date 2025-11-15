#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "Renderer/Texture.hpp"
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
        // Euler angles in radians (pitch = x, yaw = y, roll = z)
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};

        TransformComponent() = default;
        TransformComponent(const glm::vec3 &translation) : translation(translation) {}
        TransformComponent(const TransformComponent &transform) = default;

        glm::vec3 getRotationEuler() const
        {
            return rotation;
        }

        void setRotationEuler(const glm::vec3 &eulerRadians)
        {
            rotation = eulerRadians;
        }

        glm::mat4 getTransform() const
        {
            glm::mat4 rotationMatrix = glm::toMat4(glm::quat(rotation));
            return glm::translate(glm::mat4(1.0f), translation) * rotationMatrix * glm::scale(glm::mat4(1.0f), scale);
        }
    };

    struct SpriteRendererComponent
    {
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        std::shared_ptr<Texture2D> texture = nullptr;
        float tilingFactor = 1.0f;

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
