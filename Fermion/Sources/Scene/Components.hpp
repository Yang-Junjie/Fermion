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
    struct IDComponent
    {
        UUID ID;

        IDComponent() = default;

        IDComponent(const IDComponent &) = default;

        IDComponent(const UUID &id) : ID(id)
        {
        }
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

        MeshComponent() = default;

        MeshComponent(const MeshComponent &) = default;
    };

    // 材质槽位组件 - 存储Mesh使用的所有材质
    struct MaterialSlotsComponent
    {
        // 材质槽位数组，每个SubMesh通过MaterialSlotIndex索引到对应材质
        std::vector<std::shared_ptr<Material>> materials;
        
        MaterialSlotsComponent() = default;
        MaterialSlotsComponent(const MaterialSlotsComponent &) = default;
        
        // 设置槽位材质
        void setMaterial(uint32_t slotIndex, std::shared_ptr<Material> material)
        {
            if (slotIndex >= materials.size())
            {
                materials.resize(slotIndex + 1, nullptr);
            }
            materials[slotIndex] = material;
        }
        
        // 获取槽位材质
        std::shared_ptr<Material> getMaterial(uint32_t slotIndex) const
        {
            if (slotIndex < materials.size())
                return materials[slotIndex];
            return nullptr;
        }
        
        // 获取槽位数量
        size_t getSlotCount() const
        {
            return materials.size();
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

    struct PhongMaterialComponent
    {
        // Phong材质参数
        glm::vec4 diffuseColor{1.0f, 1.0f, 1.0f, 1.0f}; // 漫反射颜色
        glm::vec4 ambientColor{0.1f, 0.1f, 0.1f, 1.0f}; // 环境光颜色

        // 纹理资源句柄
        AssetHandle diffuseTextureHandle = AssetHandle(0);

        bool useTexture = false;
        bool flipUV = false;

        PhongMaterialComponent() = default;

        PhongMaterialComponent(const PhongMaterialComponent &) = default;
    };

    struct PBRMaterialComponent
    {
        // PBR材质参数
        glm::vec3 albedo{1.0f, 1.0f, 1.0f};
        float metallic = 0.0f;
        float roughness = 0.5f;
        float ao = 1.0f;

        // 纹理资源句柄
        AssetHandle albedoMapHandle = AssetHandle(0);
        AssetHandle normalMapHandle = AssetHandle(0);
        AssetHandle metallicMapHandle = AssetHandle(0);
        AssetHandle roughnessMapHandle = AssetHandle(0);
        AssetHandle aoMapHandle = AssetHandle(0);

        bool flipUV = false;

        PBRMaterialComponent() = default;

        PBRMaterialComponent(const PBRMaterialComponent &) = default;
    };

    template <typename... Component>
    struct ComponentGroup
    {
    };

    using AllComponents =
        ComponentGroup<TransformComponent, SpriteRendererComponent, MeshComponent,
                       CircleRendererComponent,
                       CameraComponent,
                       ScriptComponent,
                       ScriptContainerComponent,
                       NativeScriptComponent,
                       Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent, BoxSensor2DComponent,
                       Rigidbody3DComponent, BoxCollider3DComponent,
                       TextComponent,

                       /* Lighting */
                       PointLightComponent,
                       SpotLightComponent,
                       DirectionalLightComponent,
                       /************/

                       /* Materials */
                       PhongMaterialComponent,
                       PBRMaterialComponent,
                       MaterialSlotsComponent
                       /************/>;
} // namespace Fermion
