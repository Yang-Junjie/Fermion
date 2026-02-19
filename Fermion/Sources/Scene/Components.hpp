#pragma once
#include "Core/UUID.hpp"
#include "Renderer/Model/Mesh.hpp"
#include "Renderer/Texture/Texture.hpp"
#include "Renderer/Camera/SceneCamera.hpp"
#include "Renderer/Font/Font.hpp"
#include "Asset/Asset.hpp"
#include "Math/Math.hpp"
#include "Animation/Animator.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Fermion
{
    struct IDComponent
    {
        UUID ID;

        IDComponent() = default;

        IDComponent(const IDComponent &) = default;

        IDComponent(const UUID &id) : ID(id)
        {
        }
    };
    struct RelationshipComponent
    {
        UUID parentHandle = 0;
        std::vector<UUID> children;

        RelationshipComponent() = default;
        RelationshipComponent(const RelationshipComponent &other) = default;
        RelationshipComponent(UUID parent)
            : parentHandle(parent) {}
    };
    struct TagComponent
    {
        std::string tag;

        TagComponent() = default;

        TagComponent(const std::string &tag) : tag(tag)
        {
        }

        TagComponent(const TagComponent &tag) = default;
    };

    struct TransformComponent
    {
        glm::vec3 translation{0.0f, 0.0f, 0.0f};
        // Euler angles in radians (pitch = x, yaw = y, roll = z)
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};

        TransformComponent() = default;

        TransformComponent(const glm::vec3 &translation) : translation(translation)
        {
        }

        TransformComponent(const TransformComponent &transform) = default;

        void setTransform(const glm::mat4 &transform)
        {
            Math::decomposeTransform(transform, translation, rotation, scale);
        }

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

        glm::vec3 getForward() const
        {
            return glm::normalize(glm::rotate(glm::quat(rotation), glm::vec3(0.0f, 0.0f, -1.0f)));
        }
    };

    struct SpriteRendererComponent
    {
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        std::shared_ptr<Texture2D> texture = nullptr;
        float tilingFactor = 1.0f;
        AssetHandle textureHandle = AssetHandle(0);

        SpriteRendererComponent() = default;

        SpriteRendererComponent(const glm::vec4 &color) : color(color)
        {
        }

        SpriteRendererComponent(const SpriteRendererComponent &) = default;
    };

    struct MeshComponent
    {
        AssetHandle meshHandle = AssetHandle(0);

        bool memoryOnly = false;
        MemoryMeshType memoryMeshType = MemoryMeshType::None;

        std::vector<AssetHandle> submeshMaterials;

        MeshComponent() = default;

        MeshComponent(const MeshComponent &) = default;

        void setSubmeshMaterial(uint32_t submeshIndex, AssetHandle materialHandle)
        {
            if (submeshIndex >= submeshMaterials.size())
            {
                submeshMaterials.resize(submeshIndex + 1, AssetHandle(0));
            }
            submeshMaterials[submeshIndex] = materialHandle;
        }

        AssetHandle getSubmeshMaterial(uint32_t submeshIndex) const
        {
            if (submeshIndex < submeshMaterials.size())
                return submeshMaterials[submeshIndex];
            return AssetHandle(0);
        }

        void clearSubmeshMaterial(uint32_t submeshIndex)
        {
            if (submeshIndex < submeshMaterials.size())
            {
                submeshMaterials[submeshIndex] = AssetHandle(0);
            }
        }

        void clearAllSubmeshMaterials()
        {
            submeshMaterials.clear();
        }

        size_t getSubmeshMaterialCount() const
        {
            return submeshMaterials.size();
        }

        void resizeSubmeshMaterials(size_t count)
        {
            submeshMaterials.resize(count, AssetHandle(0));
        }
    };

    struct TextComponent
    {
        std::string textString;
        std::shared_ptr<Font> fontAsset = Font::getDefault();
        glm::vec4 color{1.0f};
        float kerning = 0.0f;
        float lineSpacing = 0.0f;
        AssetHandle fontHandle = AssetHandle(0);
    };

    struct CircleRendererComponent
    {
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        float thickness = 1.0f;
        float fade = 0.005f;

        CircleRendererComponent() = default;

        CircleRendererComponent(const CircleRendererComponent &) = default;
    };

    struct CameraComponent
    {
        SceneCamera camera;
        bool primary = true;
        bool fixedAspectRatio = false;

        CameraComponent() = default;

        CameraComponent(const glm::mat4 &projection) : camera(projection)
        {
        }

        CameraComponent(const CameraComponent &camera) = default;
    };

    struct ScriptComponent
    {
        std::string className;

        ScriptComponent() = default;

        ScriptComponent(const ScriptComponent &script) = default;
    };

    struct ScriptContainerComponent
    {
        std::vector<std::string> scriptClassNames;
    };

    class ScriptableEntity;

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

    // Physics

    struct Rigidbody2DComponent
    {
        enum class BodyType
        {
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

    struct BoxCollider2DComponent
    {
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

    struct BoxSensor2DComponent
    {
        glm::vec2 offset = {0.0f, 0.0f};
        glm::vec2 size = {0.5f, 0.5f};
        bool sensorBegin = false;
        bool sensorEnd = false;

        // Storage for runtime
        void *runtimeFixture = nullptr;

        BoxSensor2DComponent() = default;

        BoxSensor2DComponent(const BoxSensor2DComponent &) = default;
    };

    struct CircleSensor2DComponent
    {
        glm::vec2 offset = {0.0f, 0.0f};
        float radius = 0.5f;
        bool sensorBegin = false;
        bool sensorEnd = false;

        // Storage for runtime
        void *runtimeFixture = nullptr;

        CircleSensor2DComponent() = default;

        CircleSensor2DComponent(const CircleSensor2DComponent &) = default;
    };

    struct CircleCollider2DComponent
    {
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

    struct CapsuleCollider2DComponent
    {
        glm::vec2 offset = {0.0f, 0.0f};
        float radius = 0.5f;
        float height = 1.0f;

        float density = 1.0f;
        float friction = 0.5f;
        float restitution = 0.0f;
        float restitutionThreshold = 0.5f;

        void *runtimeFixture = nullptr;

        CapsuleCollider2DComponent() = default;
        CapsuleCollider2DComponent(const CapsuleCollider2DComponent &) = default;
    };

    struct RevoluteJoint2DComponent
    {
        UUID connectedBodyID = 0;
        glm::vec2 localAnchorA = {0.0f, 0.0f}; // in own body space
        glm::vec2 localAnchorB = {0.0f, 0.0f}; // in connected body space

        bool enableLimit = false;
        float lowerAngle = 0.0f;
        float upperAngle = 0.0f;

        bool enableMotor = false;
        float motorSpeed = 0.0f;
        float maxMotorTorque = 0.0f;

        // Storage for runtime
        void *runtimeJoint = nullptr;

        RevoluteJoint2DComponent() = default;

        RevoluteJoint2DComponent(const RevoluteJoint2DComponent &) = default;
    };

    struct DistanceJoint2DComponent
    { 
        UUID connectedBodyID = 0;
        glm::vec2 localAnchorA = {0.0f, 0.0f}; // in own body space
        glm::vec2 localAnchorB = {0.0f, 0.0f}; // in connected body space

        float length = 1.0f;
       
        bool enableSpring = false;
        float damping = 0.0f;
        float hertz = 0.0f;

        bool enableLimit = false;
        float minLength = 0.0f;
        float maxLength = 0.0f;

        // Storage for runtime
        void *runtimeJoint = nullptr;

        DistanceJoint2DComponent() = default;

        DistanceJoint2DComponent(const DistanceJoint2DComponent &) = default;
    };

    struct Rigidbody3DComponent
    {
        enum class BodyType
        {
            Static = 0,
            Dynamic,
            Kinematic
        };

        BodyType type = BodyType::Static;
        float mass = 1.0f;
        float linearDamping = 0.01f;
        float angularDamping = 0.05f;
        bool useGravity = true;
        bool fixedRotation = false;

        void *runtimeBody = nullptr;

        Rigidbody3DComponent() = default;

        Rigidbody3DComponent(const Rigidbody3DComponent &) = default;
    };

    struct BoxCollider3DComponent
    {
        glm::vec3 offset = {0.0f, 0.0f, 0.0f};
        glm::vec3 size = {0.5f, 0.5f, 0.5f};

        float density = 1.0f;
        float friction = 0.5f;
        float restitution = 0.0f;
        bool isTrigger = false;

        void *runtimeShape = nullptr;

        BoxCollider3DComponent() = default;

        BoxCollider3DComponent(const BoxCollider3DComponent &) = default;
    };

    struct CircleCollider3DComponent
    {
        glm::vec3 offset = {0.0f, 0.0f, 0.0f};
        float radius = 0.5f;

        float density = 1.0f;
        float friction = 0.5f;
        float restitution = 0.0f;
        bool isTrigger = false;

        void *runtimeShape = nullptr;

        CircleCollider3DComponent() = default;

        CircleCollider3DComponent(const CircleCollider3DComponent &) = default;
    };

    struct CapsuleCollider3DComponent
    {
        glm::vec3 offset = {0.0f, 0.0f, 0.0f};
        float radius = 0.5f;
        float height = 1.5f;

        float density = 1.0f;
        float friction = 0.5f;
        float restitution = 0.0f;
        bool isTrigger = false;

        void *runtimeShape = nullptr;

        CapsuleCollider3DComponent() = default;

        CapsuleCollider3DComponent(const CapsuleCollider3DComponent &) = default;
    };

    struct MeshCollider3DComponent
    {
        glm::vec3 offset = {0.0f, 0.0f, 0.0f};
        AssetHandle meshHandle = AssetHandle(0);

        float density = 1.0f;
        float friction = 0.5f;
        float restitution = 0.0f;
        bool isTrigger = false;
        bool convex = false;

        void *runtimeShape = nullptr;

        MeshCollider3DComponent() = default;

        MeshCollider3DComponent(const MeshCollider3DComponent &) = default;
    };

    struct HingeConstraint3DComponent
    {
        UUID connectedBodyID = 0;
        glm::vec3 localAnchorA = {0.0f, 0.0f, 0.0f};
        glm::vec3 localAnchorB = {0.0f, 0.0f, 0.0f};
        glm::vec3 hingeAxisA = {0.0f, 1.0f, 0.0f};
        glm::vec3 hingeAxisB = {0.0f, 1.0f, 0.0f};

        bool enableLimit = false;
        float lowerAngle = 0.0f;
        float upperAngle = 0.0f;

        bool enableMotor = false;
        float motorSpeed = 0.0f;
        float maxMotorTorque = 0.0f;

        // Storage for runtime
        void *runtimeConstraint = nullptr;

        HingeConstraint3DComponent() = default;

        HingeConstraint3DComponent(const HingeConstraint3DComponent &) = default;
    };

    struct DirectionalLightComponent
    {
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;

        bool mainLight = false;

        DirectionalLightComponent() = default;

        DirectionalLightComponent(const DirectionalLightComponent &) = default;
    };

    struct PointLightComponent
    {
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;
        float range = 10.0f;

        PointLightComponent() = default;

        PointLightComponent(const PointLightComponent &) = default;
    };

    struct SpotLightComponent
    {
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;

        float range = 10.0f;

        float angle = 30.0f;
        float softness = 0.2f;

        SpotLightComponent() = default;

        SpotLightComponent(const SpotLightComponent &) = default;
    };

    struct AnimatorComponent
    {
        // Asset references (serialized)
        AssetHandle skeletonHandle = AssetHandle(0);
        std::vector<AssetHandle> animationClipHandles;
        uint32_t activeClipIndex = 0;
        float speed = 1.0f;
        bool playing = true;
        bool looping = true;

        // Runtime data (not serialized)
        std::shared_ptr<Skeleton> runtimeSkeleton = nullptr;
        std::vector<std::shared_ptr<AnimationClip>> runtimeClips;
        std::shared_ptr<Animator> runtimeAnimator = nullptr;

        AnimatorComponent() = default;
        AnimatorComponent(const AnimatorComponent &) = default;

        void addAnimationClip(AssetHandle clipHandle)
        {
            if (std::find(animationClipHandles.begin(), animationClipHandles.end(), clipHandle) == animationClipHandles.end())
            {
                animationClipHandles.push_back(clipHandle);
            }
        }

        void removeAnimationClip(size_t index)
        {
            if (index < animationClipHandles.size())
            {
                animationClipHandles.erase(animationClipHandles.begin() + index);
                if (activeClipIndex >= animationClipHandles.size() && !animationClipHandles.empty())
                    activeClipIndex = static_cast<uint32_t>(animationClipHandles.size() - 1);
                else if (animationClipHandles.empty())
                    activeClipIndex = 0;
            }
        }

        void clearAnimationClips()
        {
            animationClipHandles.clear();
            runtimeClips.clear();
            activeClipIndex = 0;
        }
    };

    template <typename... Component>
    struct ComponentGroup
    {
    };

    using AllComponents =
        ComponentGroup<
            TransformComponent, SpriteRendererComponent, MeshComponent, RelationshipComponent,
            CircleRendererComponent,
            CameraComponent,
            ScriptComponent,
            ScriptContainerComponent,
            NativeScriptComponent,

            Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent, CapsuleCollider2DComponent, BoxSensor2DComponent, CircleSensor2DComponent, RevoluteJoint2DComponent, DistanceJoint2DComponent,
            Rigidbody3DComponent, BoxCollider3DComponent, CircleCollider3DComponent, CapsuleCollider3DComponent, MeshCollider3DComponent, HingeConstraint3DComponent,

            TextComponent,

            /* Lighting */
            PointLightComponent,
            SpotLightComponent,
            DirectionalLightComponent,
            /************/

            /* Animation */
            AnimatorComponent
            /************/>;
} // namespace Fermion
