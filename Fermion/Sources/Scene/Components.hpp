#pragma once
#include "fmpch.hpp"
#include "Core/UUID.hpp"
// #include "Scene/ScriptableEntity.hpp"
#include "Renderer/Model/Mesh.hpp"
#include "Renderer/Model/Material.hpp"
#include "Renderer/Texture/Texture.hpp"
#include "Renderer/Camera/SceneCamera.hpp"
#include "Renderer/Font/Font.hpp"
#include "Asset/Asset.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
namespace Fermion
{
struct IDComponent {
    UUID ID;

    IDComponent() = default;
    IDComponent(const IDComponent &) = default;
    IDComponent(const UUID &id) : ID(id) {
    }
};
struct TagComponent {
    std::string tag;
    TagComponent() = default;
    TagComponent(const std::string &tag) : tag(tag) {
    }
    TagComponent(const TagComponent &tag) = default;
};

struct TransformComponent {
    glm::vec3 translation{0.0f, 0.0f, 0.0f};
    // Euler angles in radians (pitch = x, yaw = y, roll = z)
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    TransformComponent() = default;
    TransformComponent(const glm::vec3 &translation) : translation(translation) {
    }
    TransformComponent(const TransformComponent &transform) = default;

    glm::vec3 getRotationEuler() const {
        return rotation;
    }

    void setRotationEuler(const glm::vec3 &eulerRadians) {
        rotation = eulerRadians;
    }

    glm::mat4 getTransform() const {
        glm::mat4 rotationMatrix = glm::toMat4(glm::quat(rotation));
        return glm::translate(glm::mat4(1.0f), translation) * rotationMatrix * glm::scale(glm::mat4(1.0f), scale);
    }
    glm::vec3 getForward() const {
        return glm::normalize(glm::rotate(glm::quat(rotation), glm::vec3(0.0f, 0.0f, -1.0f)));
    }
};

struct SpriteRendererComponent {
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    std::shared_ptr<Texture2D> texture = nullptr;
    float tilingFactor = 1.0f;
    AssetHandle textureHandle = AssetHandle(0);

    SpriteRendererComponent() = default;
    SpriteRendererComponent(const glm::vec4 &color) : color(color) {
    }
    SpriteRendererComponent(const SpriteRendererComponent &) = default;
};

struct MeshComponent {
    AssetHandle meshHandle = AssetHandle(0);
    
    bool memoryOnly = false;
    MemoryMeshType memoryMeshType = MemoryMeshType::None;

    MeshComponent() = default;
    MeshComponent(const MeshComponent &) = default;
};

struct MaterialComponent {
    std::shared_ptr<Material> MaterialInstance;
    bool overrideMaterial = false;
    MaterialComponent() {
        MaterialInstance = std::make_shared<Material>();
    }
    MaterialComponent(const MaterialComponent &) = default;
};

struct TextComponent {
    std::string textString;
    std::shared_ptr<Font> fontAsset = Font::getDefault();
    glm::vec4 color{1.0f};
    float kerning = 0.0f;
    float lineSpacing = 0.0f;
    AssetHandle fontHandle = AssetHandle(0);
};

struct CircleRendererComponent {
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    float thickness = 1.0f;
    float fade = 0.005f;

    CircleRendererComponent() = default;
    CircleRendererComponent(const CircleRendererComponent &) = default;
};
struct CameraComponent {
    SceneCamera camera;
    bool primary = true;
    bool fixedAspectRatio = false;
    CameraComponent() = default;
    CameraComponent(const glm::mat4 &projection) : camera(projection) {
    }
    CameraComponent(const CameraComponent &camera) = default;
};

struct ScriptComponent {
    std::string className;
    ScriptComponent() = default;
    ScriptComponent(const ScriptComponent &script) = default;
};

struct ScriptContainerComponent {
    std::vector<std::string> scriptClassNames;
};

class ScriptableEntity;
struct NativeScriptComponent {
    ScriptableEntity *instance = nullptr;

    ScriptableEntity *(*instantiateScript)();
    void (*destroyScript)(NativeScriptComponent *);

    template <typename T>
    void bind() {
        instantiateScript = []() {
            return static_cast<ScriptableEntity *>(new T());
        };
        destroyScript = [](NativeScriptComponent *nsc) {
            delete nsc->instance;
            nsc->instance = nullptr;
        };
    }
};

// Physics

struct Rigidbody2DComponent {
    enum class BodyType {
        Static = 0,
        Dynamic,
        Kinematic
    };
    BodyType type = BodyType::Static;
    bool fixedRotation = false;

    // Storage for runtime
    void *runtimeBody = nullptr;

    Rigidbody2DComponent() = default;
    Rigidbody2DComponent(const Rigidbody2DComponent &) = default;
};

struct BoxCollider2DComponent {
    glm::vec2 offset = {0.0f, 0.0f};
    glm::vec2 size = {0.5f, 0.5f};

    // TODO: move into physics material component in the future
    float density = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;
    float restitutionThreshold = 0.5f;

    // Storage for runtime
    void *runtimeFixture = nullptr;

    BoxCollider2DComponent() = default;
    BoxCollider2DComponent(const BoxCollider2DComponent &) = default;
};

struct BoxSensor2DComponent {
    glm::vec2 offset = {0.0f, 0.0f};
    glm::vec2 size = {0.5f, 0.5f};
    bool sensorBegin = false;
    bool sensorEnd = false;

    // Storage for runtime
    void *runtimeFixture = nullptr;

    BoxSensor2DComponent() = default;
    BoxSensor2DComponent(const BoxSensor2DComponent &) = default;
};

struct CircleCollider2DComponent {
    glm::vec2 offset = {0.0f, 0.0f};
    float radius = 0.5f;

    // TODO: move into physics material in the future maybe
    float density = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;
    float restitutionThreshold = 0.5f;

    // Storage for runtime
    void *runtimeFixture = nullptr;

    CircleCollider2DComponent() = default;
    CircleCollider2DComponent(const CircleCollider2DComponent &) = default;
};

struct DirectionalLightComponent {
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    bool mainLight = false;

    DirectionalLightComponent() = default;
    DirectionalLightComponent(const DirectionalLightComponent &) = default;
};

struct PointLightComponent {
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float range = 10.0f;

    PointLightComponent() = default;
    PointLightComponent(const PointLightComponent &) = default;
};

struct SpotLightComponent {
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    float range = 10.0f;

    float angle = 30.0f;
    float softness = 0.2f;

    SpotLightComponent() = default;
    SpotLightComponent(const SpotLightComponent &) = default;
};

template <typename... Component>
struct ComponentGroup {
};
using AllComponents =
    ComponentGroup<TransformComponent, SpriteRendererComponent, MeshComponent, MaterialComponent,
                   CircleRendererComponent,
                   CameraComponent,
                   ScriptComponent,
                   ScriptContainerComponent,
                   NativeScriptComponent,
                   Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent, BoxSensor2DComponent,
                   TextComponent,

                   /* Lighting */
                   PointLightComponent,
                   SpotLightComponent,
                   DirectionalLightComponent
                   /************/>;
} // namespace Fermion
