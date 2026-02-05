#include "SceneSerializer.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include "Project/Project.hpp"
#include "Asset/AssetManager/RuntimeAssetManager.hpp"
#include "Renderer/Model/MeshFactory.hpp"
#include "yaml-cpp/emittermanip.h"

#include <fstream>
#include <yaml-cpp/yaml.h>
#include "Core/Yaml.hpp"

namespace Fermion
{
    SceneSerializer::SceneSerializer(const std::shared_ptr<Scene> &scene) : m_scene(scene)
    {
    }

    static void serializeEntity(YAML::Emitter &out, Entity entity)
    {
        FERMION_ASSERT(entity.hasComponent<IDComponent>(), "Entity must have an IDComponent");
        out << YAML::BeginMap;
        out << YAML::Key << "Entity" << YAML::Value << entity.getUUID();
        if (entity.hasComponent<TagComponent>())
        {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap;
            out << YAML::Key << "Tag" << YAML::Value << entity.getComponent<TagComponent>().tag;
            out << YAML::EndMap;
        }

        if (entity.hasComponent<TransformComponent>())
        {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;
            auto &tc = entity.getComponent<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << tc.translation;
            out << YAML::Key << "Rotation" << YAML::Value << tc.getRotationEuler();
            out << YAML::Key << "Scale" << YAML::Value << tc.scale;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<RelationshipComponent>())
        {
            out << YAML::Key << "RelationshipComponent";
            out << YAML::BeginMap;
            auto &rc = entity.getComponent<RelationshipComponent>();
            out << YAML::Key << "Parent" << YAML::Value << static_cast<uint64_t>(rc.parentHandle);
            if (!rc.children.empty())
            {
                out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
                for (const UUID &child : rc.children)
                    out << static_cast<uint64_t>(child);
                out << YAML::EndSeq;
            }
            out << YAML::EndMap;
        }
        if (entity.hasComponent<SpriteRendererComponent>())
        {
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap;
            auto &sprite = entity.getComponent<SpriteRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << sprite.color;
            if (static_cast<uint64_t>(sprite.textureHandle) != 0)
                out << YAML::Key << "TextureHandle" << YAML::Value << static_cast<uint64_t>(sprite.textureHandle);
            out << YAML::EndMap;
        }
        if (entity.hasComponent<MeshComponent>())
        {
            out << YAML::Key << "MeshComponent";
            out << YAML::BeginMap;
            auto &mesh = entity.getComponent<MeshComponent>();
            {
                if (static_cast<uint64_t>(mesh.meshHandle) != 0)
                    out << YAML::Key << "MeshHandle" << YAML::Value << static_cast<uint64_t>(mesh.meshHandle);

                out << YAML::Key << "MemoryOnly" << YAML::Value << mesh.memoryOnly;
                out << YAML::Key << "MemoryMeshType" << YAML::Value << static_cast<uint16_t>(mesh.memoryMeshType);

                if (!mesh.submeshMaterials.empty())
                {
                    out << YAML::Key << "SubmeshMaterials" << YAML::Value << YAML::BeginSeq;
                    for (const auto &materialHandle : mesh.submeshMaterials)
                    {
                        out << static_cast<uint64_t>(materialHandle);
                    }
                    out << YAML::EndSeq;
                }
            }
            out << YAML::EndMap;
        }
        if (entity.hasComponent<DirectionalLightComponent>())
        {
            out << YAML::Key << "DirectionalLightComponent";
            out << YAML::BeginMap;
            auto &directionalLightComponent = entity.getComponent<DirectionalLightComponent>();
            {
                out << YAML::Key << "Color" << YAML::Value << directionalLightComponent.color;
                out << YAML::Key << "Intensity" << YAML::Value << directionalLightComponent.intensity;
                out << YAML::Key << "MainLight" << YAML::Value << directionalLightComponent.mainLight;
            }
            out << YAML::EndMap;
        }
        if (entity.hasComponent<PointLightComponent>())
        {
            out << YAML::Key << "PointLightComponent";
            out << YAML::BeginMap;
            auto &pointLightComponent = entity.getComponent<PointLightComponent>();
            {
                out << YAML::Key << "Color" << YAML::Value << pointLightComponent.color;
                out << YAML::Key << "Intensity" << YAML::Value << pointLightComponent.intensity;
                out << YAML::Key << "Range" << YAML::Value << pointLightComponent.range;
            }
            out << YAML::EndMap;
        }
        if (entity.hasComponent<SpotLightComponent>())
        {
            out << YAML::Key << "SpotLightComponent";
            out << YAML::BeginMap;
            auto &spotLightComponent = entity.getComponent<SpotLightComponent>();
            {
                out << YAML::Key << "Color" << YAML::Value << spotLightComponent.color;
                out << YAML::Key << "Intensity" << YAML::Value << spotLightComponent.intensity;
                out << YAML::Key << "Range" << YAML::Value << spotLightComponent.range;
                out << YAML::Key << "Angle" << YAML::Value << spotLightComponent.angle;
                out << YAML::Key << "Softness" << YAML::Value << spotLightComponent.softness;
            }
            out << YAML::EndMap;
        }
        if (entity.hasComponent<CircleRendererComponent>())
        {
            out << YAML::Key << "CircleRendererComponent";
            out << YAML::BeginMap;
            auto &circleComponent = entity.getComponent<CircleRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << circleComponent.color;
            out << YAML::Key << "Thickness" << YAML::Value << circleComponent.thickness;
            out << YAML::Key << "Fade" << YAML::Value << circleComponent.fade;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<TextComponent>())
        {
            out << YAML::Key << "TextComponent";
            out << YAML::BeginMap;
            auto &textComponent = entity.getComponent<TextComponent>();
            out << YAML::Key << "Text" << YAML::Value << textComponent.textString;
            if (static_cast<uint64_t>(textComponent.fontHandle) != 0)
                out << YAML::Key << "FontHandle" << YAML::Value << static_cast<uint64_t>(textComponent.fontHandle);
            out << YAML::Key << "Color" << YAML::Value << textComponent.color;
            out << YAML::Key << "Kerning" << YAML::Value << textComponent.kerning;
            out << YAML::Key << "LineSpacing" << YAML::Value << textComponent.lineSpacing;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<Rigidbody2DComponent>())
        {
            out << YAML::Key << "Rigidbody2DComponent";
            out << YAML::BeginMap;

            auto &rb = entity.getComponent<Rigidbody2DComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << static_cast<int>(rb.type);
            out << YAML::Key << "FixedRotation" << YAML::Value << rb.fixedRotation;

            out << YAML::EndMap;
        }
        if (entity.hasComponent<BoxCollider2DComponent>())
        {
            out << YAML::Key << "BoxCollider2DComponent";
            out << YAML::BeginMap;

            auto &bc = entity.getComponent<BoxCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << bc.offset;
            out << YAML::Key << "Size" << YAML::Value << bc.size;
            out << YAML::Key << "Density" << YAML::Value << bc.density;
            out << YAML::Key << "Friction" << YAML::Value << bc.friction;
            out << YAML::Key << "Restitution" << YAML::Value << bc.restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc.restitutionThreshold;

            out << YAML::EndMap;
        }
        if (entity.hasComponent<CircleCollider2DComponent>())
        {
            out << YAML::Key << "CircleCollider2DComponent";
            out << YAML::BeginMap;
            auto &cc = entity.getComponent<CircleCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << cc.offset;
            out << YAML::Key << "Radius" << YAML::Value << cc.radius;
            out << YAML::Key << "Density" << YAML::Value << cc.density;
            out << YAML::Key << "Friction" << YAML::Value << cc.friction;
            out << YAML::Key << "Restitution" << YAML::Value << cc.restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc.restitutionThreshold;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<Rigidbody3DComponent>())
        {
            out << YAML::Key << "Rigidbody3DComponent";
            out << YAML::BeginMap;
            auto &rb = entity.getComponent<Rigidbody3DComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << static_cast<int>(rb.type);
            out << YAML::Key << "Mass" << YAML::Value << rb.mass;
            out << YAML::Key << "LinearDamping" << YAML::Value << rb.linearDamping;
            out << YAML::Key << "AngularDamping" << YAML::Value << rb.angularDamping;
            out << YAML::Key << "UseGravity" << YAML::Value << rb.useGravity;
            out << YAML::Key << "FixedRotation" << YAML::Value << rb.fixedRotation;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<BoxCollider3DComponent>())
        {
            out << YAML::Key << "BoxCollider3DComponent";
            out << YAML::BeginMap;
            auto &bc = entity.getComponent<BoxCollider3DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << bc.offset;
            out << YAML::Key << "Size" << YAML::Value << bc.size;
            out << YAML::Key << "Density" << YAML::Value << bc.density;
            out << YAML::Key << "Friction" << YAML::Value << bc.friction;
            out << YAML::Key << "Restitution" << YAML::Value << bc.restitution;
            out << YAML::Key << "IsTrigger" << YAML::Value << bc.isTrigger;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<CircleCollider3DComponent>())
        {
            out << YAML::Key << "CircleCollider3DComponent";
            out << YAML::BeginMap;
            auto &cc = entity.getComponent<CircleCollider3DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << cc.offset;
            out << YAML::Key << "Radius" << YAML::Value << cc.radius;
            out << YAML::Key << "Density" << YAML::Value << cc.density;
            out << YAML::Key << "Friction" << YAML::Value << cc.friction;
            out << YAML::Key << "Restitution" << YAML::Value << cc.restitution;
            out << YAML::Key << "IsTrigger" << YAML::Value << cc.isTrigger;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<CapsuleCollider3DComponent>())
        {
            out << YAML::Key << "CapsuleCollider3DComponent";
            out << YAML::BeginMap;
            auto &cc = entity.getComponent<CapsuleCollider3DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << cc.offset;
            out << YAML::Key << "Radius" << YAML::Value << cc.radius;
            out << YAML::Key << "Height" << YAML::Value << cc.height;
            out << YAML::Key << "Density" << YAML::Value << cc.density;
            out << YAML::Key << "Friction" << YAML::Value << cc.friction;
            out << YAML::Key << "Restitution" << YAML::Value << cc.restitution;
            out << YAML::Key << "IsTrigger" << YAML::Value << cc.isTrigger;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<BoxSensor2DComponent>())
        {
            out << YAML::Key << "BoxSensor2DComponent";
            out << YAML::BeginMap;
            auto &cc = entity.getComponent<BoxSensor2DComponent>();
            out << YAML::Key << "SensorBegin" << YAML::Value << cc.sensorBegin;
            out << YAML::Key << "SensorEnd" << YAML::Value << cc.sensorEnd;
            out << YAML::Key << "Offset" << YAML::Value << cc.offset;
            out << YAML::Key << "Size" << YAML::Value << cc.size;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<CameraComponent>())
        {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap;
            auto &cc = entity.getComponent<CameraComponent>();
            auto &camera = cc.camera;
            out << YAML::Key << "Camera" << YAML::Value;
            out << YAML::BeginMap;
            out << YAML::Key << "ProjectionType" << YAML::Value << static_cast<int>(camera.getProjectionType());
            out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.getPerspectiveFOV();
            out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.getPerspectiveNearClip();
            out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.getPerspectiveFarClip();
            out << YAML::Key << "OrthographicSize" << YAML::Value << camera.getOrthographicSize();
            out << YAML::Key << "OrthographicNear" << YAML::Value << camera.getOrthographicNearClip();
            out << YAML::Key << "OrthographicFar" << YAML::Value << camera.getOrthographicFarClip();
            out << YAML::EndMap;

            out << YAML::Key << "Primary" << YAML::Value << cc.primary;
            out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.fixedAspectRatio;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<ScriptComponent>())
        {
            out << YAML::Key << "ScriptComponent";
            out << YAML::BeginMap;
            auto &sc = entity.getComponent<ScriptComponent>();
            out << YAML::Key << "ClassName" << YAML::Value << sc.className;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<ScriptContainerComponent>())
        {
            out << YAML::Key << "ScriptContainerComponent";
            out << YAML::BeginMap;

            auto &scc = entity.getComponent<ScriptContainerComponent>();

            out << YAML::Key << "ScriptClassNames" << YAML::Value << YAML::BeginSeq;
            for (auto &name : scc.scriptClassNames)
                out << name;
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }
        if (entity.hasComponent<AnimatorComponent>())
        {
            out << YAML::Key << "AnimatorComponent";
            out << YAML::BeginMap;
            auto &ac = entity.getComponent<AnimatorComponent>();
            out << YAML::Key << "SkeletonHandle" << YAML::Value << static_cast<uint64_t>(ac.skeletonHandle);
            out << YAML::Key << "ActiveClipIndex" << YAML::Value << ac.activeClipIndex;
            out << YAML::Key << "Speed" << YAML::Value << ac.speed;
            out << YAML::Key << "Playing" << YAML::Value << ac.playing;
            out << YAML::Key << "Looping" << YAML::Value << ac.looping;
            if (!ac.animationClipHandles.empty())
            {
                out << YAML::Key << "AnimationClips" << YAML::Value << YAML::BeginSeq;
                for (const auto &clipHandle : ac.animationClipHandles)
                    out << static_cast<uint64_t>(clipHandle);
                out << YAML::EndSeq;
            }
            out << YAML::EndMap;
        }

        // Close entity map after writing all components
        out << YAML::EndMap;
    }

    void SceneSerializer::serialize(const std::filesystem::path &filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Name";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        auto view = m_scene->getRegistry().view<TransformComponent>();
        for (auto entityID : view)
        {
            Entity entity = {entityID, m_scene.get()};
            if (!entity)
                continue;
            serializeEntity(out, entity);
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;
        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void SceneSerializer::serializeRuntime(const std::filesystem::path &filepath)
    {
    }

    bool SceneSerializer::deserialize(const std::filesystem::path &filepath)
    {
        std::ifstream stream(filepath);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if (!data["Scene"])
            return false;
        std::string sceneName = data["Scene"].as<std::string>();
        Log::Info(std::format("Deserializing scene '{}'", sceneName));
        const auto &entities = data["Entities"];
        if (entities && entities.IsSequence())
        {
            for (auto entity : entities)
            {
                if (!entity.IsMap())
                {
                    Log::Warn("Skip non-map entity entry in YAML");
                    continue;
                }

                auto idNode = entity["Entity"];
                if (!idNode)
                {
                    Log::Warn("Entity missing ID; skipping");
                    continue;
                }
                uint64_t uuid = idNode.as<uint64_t>();

                std::string name;
                auto tagComponent = entity["TagComponent"];
                if (tagComponent && tagComponent.IsMap() && tagComponent["Tag"])
                {
                    name = tagComponent["Tag"].as<std::string>();
                }
                Log::Info(std::format("Deserialized entity with ID = {0}, name = {1}", uuid, name));
                Entity deserializedEntity = m_scene->createEntityWithUUID(uuid, name);

                auto transformComponent = entity["TransformComponent"];
                if (transformComponent && transformComponent.IsMap())
                {
                    auto &tc = deserializedEntity.getComponent<TransformComponent>();
                    if (auto n = transformComponent["Translation"]; n)
                        tc.translation = n.as<glm::vec3>();
                    if (auto n = transformComponent["Rotation"]; n)
                        tc.setRotationEuler(n.as<glm::vec3>());
                    if (auto n = transformComponent["Scale"]; n)
                        tc.scale = n.as<glm::vec3>();
                }

                auto relationshipComponent = entity["RelationshipComponent"];
                if (relationshipComponent && relationshipComponent.IsMap())
                {
                    auto &rc = deserializedEntity.getComponent<RelationshipComponent>();
                    if (auto n = relationshipComponent["Parent"]; n)
                        rc.parentHandle = UUID(n.as<uint64_t>());
                    if (auto n = relationshipComponent["Children"]; n && n.IsSequence())
                    {
                        rc.children.clear();
                        for (auto childNode : n)
                            rc.children.emplace_back(UUID(childNode.as<uint64_t>()));
                    }
                }

                auto spriteRendererComponent = entity["SpriteRendererComponent"];
                if (spriteRendererComponent && spriteRendererComponent.IsMap())
                {
                    auto &src = deserializedEntity.addComponent<SpriteRendererComponent>();
                    if (auto n = spriteRendererComponent["Color"]; n)
                        src.color = n.as<glm::vec4>();
                    if (auto n = spriteRendererComponent["TextureHandle"]; n)
                    {
                        uint64_t handleValue = n.as<uint64_t>();
                        if (handleValue != 0 )
                        {
                            src.textureHandle = AssetHandle(handleValue);
                            // auto runtimeAssets = Project::getRuntimeAssetManager();
                        }
                    }
                }

                auto meshComponent = entity["MeshComponent"];
                if (meshComponent)
                {

                    auto &src = deserializedEntity.addComponent<MeshComponent>();

                    if (auto n = meshComponent["MemoryMeshType"]; n)
                    {
                        src.memoryMeshType = static_cast<MemoryMeshType>(n.as<uint16_t>());
                    }

                    bool memoryOnly = false;
                    if (auto n = meshComponent["MemoryOnly"]; n)
                    {
                        memoryOnly = n.as<bool>();
                    }

                    src.memoryOnly = memoryOnly;

                    if (memoryOnly)
                    {
                        src.meshHandle = MeshFactory::createMemoryMesh(src.memoryMeshType);
                    }
                    else
                    {
                        if (auto n = meshComponent["MeshHandle"]; n)
                        {
                            uint64_t handleValue = n.as<uint64_t>();
                            if (handleValue != 0)
                            {
                                src.meshHandle = AssetHandle(handleValue);
                            }
                        }
                    }

                    // 加载submeshMaterials数组
                    auto submeshMaterialsNode = meshComponent["SubmeshMaterials"];
                    if (submeshMaterialsNode && submeshMaterialsNode.IsSequence())
                    {
                        src.submeshMaterials.clear();
                        for (size_t i = 0; i < submeshMaterialsNode.size(); ++i)
                        {
                            uint64_t handleValue = submeshMaterialsNode[i].as<uint64_t>();
                            src.submeshMaterials.push_back(AssetHandle(handleValue));
                        }
                    }
                }

                auto directionalLightComponent = entity["DirectionalLightComponent"];
                if (directionalLightComponent)
                {
                    auto &dlc = deserializedEntity.addComponent<DirectionalLightComponent>();
                    if (auto n = directionalLightComponent["Color"]; n)
                        dlc.color = n.as<glm::vec3>();
                    if (auto n = directionalLightComponent["Intensity"]; n)
                        dlc.intensity = n.as<float>();
                    if (auto n = directionalLightComponent["MainLight"]; n)
                        dlc.mainLight = n.as<bool>();
                }

                auto pointLightComponent = entity["PointLightComponent"];
                if (pointLightComponent)
                {
                    auto &plc = deserializedEntity.addComponent<PointLightComponent>();
                    if (auto n = pointLightComponent["Color"]; n)
                        plc.color = n.as<glm::vec3>();
                    if (auto n = pointLightComponent["Intensity"]; n)
                        plc.intensity = n.as<float>();
                    if (auto n = pointLightComponent["Range"]; n)
                        plc.range = n.as<float>();
                }

                auto spotLightComponent = entity["SpotLightComponent"];
                if (spotLightComponent)
                {
                    auto &splc = deserializedEntity.addComponent<SpotLightComponent>();
                    if (auto n = spotLightComponent["Color"]; n)
                        splc.color = n.as<glm::vec3>();
                    if (auto n = spotLightComponent["Intensity"]; n)
                        splc.intensity = n.as<float>();

                    if (auto n = spotLightComponent["Range"])
                        splc.range = n.as<float>();

                    if (auto n = spotLightComponent["Angle"]; n)
                        splc.angle = n.as<float>();
                    if (auto n = spotLightComponent["Softness"]; n)
                        splc.softness = n.as<float>();
                }

                auto circleRendererComponent = entity["CircleRendererComponent"];
                if (circleRendererComponent && circleRendererComponent.IsMap())
                {
                    auto &cr = deserializedEntity.addComponent<CircleRendererComponent>();
                    if (auto n = circleRendererComponent["Color"]; n)
                        cr.color = n.as<glm::vec4>();
                    if (auto n = circleRendererComponent["Thickness"]; n)
                        cr.thickness = n.as<float>();
                    if (auto n = circleRendererComponent["Fade"]; n)
                        cr.fade = n.as<float>();
                }
                auto textComponent = entity["TextComponent"];
                if (textComponent && textComponent.IsMap())
                {
                    auto &tc = deserializedEntity.addComponent<TextComponent>();

                    if (auto n = textComponent["Text"]; n)
                        tc.textString = n.as<std::string>();

                    if (auto n = textComponent["Color"]; n)
                        tc.color = n.as<glm::vec4>();
                    if (auto n = textComponent["Kerning"]; n)
                        tc.kerning = n.as<float>();
                    if (auto n = textComponent["LineSpacing"]; n)
                        tc.lineSpacing = n.as<float>();
                    if (auto n = textComponent["FontHandle"]; n)
                    {
                        uint64_t handleValue = n.as<uint64_t>();
                        if (handleValue != 0)
                        {
                            tc.fontHandle = AssetHandle(handleValue);
                            auto runtimeAssets = Project::getRuntimeAssetManager();
                            tc.fontAsset = runtimeAssets->getAsset<Font>(tc.fontHandle);
                        }
                    }
                }

                auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
                if (rigidbody2DComponent && rigidbody2DComponent.IsMap())
                {
                    auto &rb = deserializedEntity.addComponent<Rigidbody2DComponent>();
                    if (auto n = rigidbody2DComponent["BodyType"]; n)
                        rb.type = static_cast<Rigidbody2DComponent::BodyType>(n.as<int>());
                    if (auto n = rigidbody2DComponent["FixedRotation"]; n)
                        rb.fixedRotation = n.as<bool>();
                }

                auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
                if (boxCollider2DComponent && boxCollider2DComponent.IsMap())
                {
                    auto &bc = deserializedEntity.addComponent<BoxCollider2DComponent>();
                    if (auto n = boxCollider2DComponent["Offset"]; n)
                        bc.offset = n.as<glm::vec2>();
                    if (auto n = boxCollider2DComponent["Size"]; n)
                        bc.size = n.as<glm::vec2>();
                    if (auto n = boxCollider2DComponent["Density"]; n)
                        bc.density = n.as<float>();
                    if (auto n = boxCollider2DComponent["Friction"]; n)
                        bc.friction = n.as<float>();
                    if (auto n = boxCollider2DComponent["Restitution"]; n)
                        bc.restitution = n.as<float>();
                    if (auto n = boxCollider2DComponent["RestitutionThreshold"]; n)
                        bc.restitutionThreshold = n.as<float>();
                }
                auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
                if (circleCollider2DComponent && circleCollider2DComponent.IsMap())
                {
                    auto &cc = deserializedEntity.addComponent<CircleCollider2DComponent>();
                    if (auto n = circleCollider2DComponent["Offset"]; n)
                        cc.offset = n.as<glm::vec2>();
                    if (auto n = circleCollider2DComponent["Radius"]; n)
                        cc.radius = n.as<float>();
                    if (auto n = circleCollider2DComponent["Density"]; n)
                        cc.density = n.as<float>();
                    if (auto n = circleCollider2DComponent["Friction"]; n)
                        cc.friction = n.as<float>();
                    if (auto n = circleCollider2DComponent["Restitution"]; n)
                        cc.restitution = n.as<float>();
                    if (auto n = circleCollider2DComponent["RestitutionThreshold"]; n)
                        cc.restitutionThreshold = n.as<float>();
                }
                auto rigidbody3DComponent = entity["Rigidbody3DComponent"];
                if (rigidbody3DComponent && rigidbody3DComponent.IsMap())
                {
                    auto &rb = deserializedEntity.addComponent<Rigidbody3DComponent>();
                    if (auto n = rigidbody3DComponent["BodyType"]; n)
                        rb.type = static_cast<Rigidbody3DComponent::BodyType>(n.as<int>());
                    if (auto n = rigidbody3DComponent["Mass"]; n)
                        rb.mass = n.as<float>();
                    if (auto n = rigidbody3DComponent["LinearDamping"]; n)
                        rb.linearDamping = n.as<float>();
                    if (auto n = rigidbody3DComponent["AngularDamping"]; n)
                        rb.angularDamping = n.as<float>();
                    if (auto n = rigidbody3DComponent["UseGravity"]; n)
                        rb.useGravity = n.as<bool>();
                    if (auto n = rigidbody3DComponent["FixedRotation"]; n)
                        rb.fixedRotation = n.as<bool>();
                }
                auto boxCollider3DComponent = entity["BoxCollider3DComponent"];
                if (boxCollider3DComponent && boxCollider3DComponent.IsMap())
                {
                    auto &bc = deserializedEntity.addComponent<BoxCollider3DComponent>();
                    if (auto n = boxCollider3DComponent["Offset"]; n)
                        bc.offset = n.as<glm::vec3>();
                    if (auto n = boxCollider3DComponent["Size"]; n)
                        bc.size = n.as<glm::vec3>();
                    if (auto n = boxCollider3DComponent["Density"]; n)
                        bc.density = n.as<float>();
                    if (auto n = boxCollider3DComponent["Friction"]; n)
                        bc.friction = n.as<float>();
                    if (auto n = boxCollider3DComponent["Restitution"]; n)
                        bc.restitution = n.as<float>();
                    if (auto n = boxCollider3DComponent["IsTrigger"]; n)
                        bc.isTrigger = n.as<bool>();
                }
                auto circleCollider3DComponent = entity["CircleCollider3DComponent"];
                if (circleCollider3DComponent && circleCollider3DComponent.IsMap())
                {
                    auto &cc = deserializedEntity.addComponent<CircleCollider3DComponent>();
                    if (auto n = circleCollider3DComponent["Offset"]; n)
                        cc.offset = n.as<glm::vec3>();
                    if (auto n = circleCollider3DComponent["Radius"]; n)
                        cc.radius = n.as<float>();
                    if (auto n = circleCollider3DComponent["Density"]; n)
                        cc.density = n.as<float>();
                    if (auto n = circleCollider3DComponent["Friction"]; n)
                        cc.friction = n.as<float>();
                    if (auto n = circleCollider3DComponent["Restitution"]; n)
                        cc.restitution = n.as<float>();
                    if (auto n = circleCollider3DComponent["IsTrigger"]; n)
                        cc.isTrigger = n.as<bool>();
                }
                auto capsuleCollider3DComponent = entity["CapsuleCollider3DComponent"];
                if (capsuleCollider3DComponent && capsuleCollider3DComponent.IsMap())
                {
                    auto &cc = deserializedEntity.addComponent<CapsuleCollider3DComponent>();
                    if (auto n = capsuleCollider3DComponent["Offset"]; n)
                        cc.offset = n.as<glm::vec3>();
                    if (auto n = capsuleCollider3DComponent["Radius"]; n)
                        cc.radius = n.as<float>();
                    if (auto n = capsuleCollider3DComponent["Height"]; n)
                        cc.height = n.as<float>();
                    if (auto n = capsuleCollider3DComponent["Density"]; n)
                        cc.density = n.as<float>();
                    if (auto n = capsuleCollider3DComponent["Friction"]; n)
                        cc.friction = n.as<float>();
                    if (auto n = capsuleCollider3DComponent["Restitution"]; n)
                        cc.restitution = n.as<float>();
                    if (auto n = capsuleCollider3DComponent["IsTrigger"]; n)
                        cc.isTrigger = n.as<bool>();
                }
                auto boxSensor2DComponent = entity["BoxSensor2DComponent"];
                if (boxSensor2DComponent && boxSensor2DComponent.IsMap())
                {
                    auto &bs2c = deserializedEntity.addComponent<BoxSensor2DComponent>();
                    if (auto n = boxSensor2DComponent["SensorBegin"]; n)
                        bs2c.sensorBegin = n.as<bool>();
                    if (auto n = boxSensor2DComponent["SensorEnd"]; n)
                        bs2c.sensorEnd = n.as<bool>();
                    if (auto n = boxSensor2DComponent["Offset"]; n)
                        bs2c.offset = n.as<glm::vec2>();
                    if (auto n = boxSensor2DComponent["Size"]; n)
                        bs2c.size = n.as<glm::vec2>();
                }

                auto cameraComponent = entity["CameraComponent"];
                if (cameraComponent && cameraComponent.IsMap())
                {
                    auto &cc = deserializedEntity.addComponent<CameraComponent>();
                    auto &camera = cc.camera;
                    auto camNode = cameraComponent["Camera"];
                    if (camNode && camNode.IsMap())
                    {
                        if (auto n = camNode["ProjectionType"]; n)
                            camera.setProjectionType(static_cast<SceneCamera::ProjectionType>(n.as<int>()));
                        if (auto n = camNode["PerspectiveFOV"]; n)
                            camera.setPerspectiveFOV(n.as<float>());
                        if (auto n = camNode["PerspectiveNear"]; n)
                            camera.setPerspectiveNearClip(n.as<float>());
                        if (auto n = camNode["PerspectiveFar"]; n)
                            camera.setPerspectiveFarClip(n.as<float>());
                        if (auto n = camNode["OrthographicSize"]; n)
                            camera.setOrthographicSize(n.as<float>());
                        if (auto n = camNode["OrthographicNear"]; n)
                            camera.setOrthographicNearClip(n.as<float>());
                        if (auto n = camNode["OrthographicFar"]; n)
                            camera.setOrthographicFarClip(n.as<float>());
                    }
                    if (auto n = cameraComponent["Primary"]; n)
                        cc.primary = n.as<bool>();
                    if (auto n = cameraComponent["FixedAspectRatio"]; n)
                        cc.fixedAspectRatio = n.as<bool>();
                }
                auto scriptComponent = entity["ScriptComponent"];
                if (scriptComponent && scriptComponent.IsMap())
                {
                    auto &sc = deserializedEntity.addComponent<ScriptComponent>();
                    if (auto n = scriptComponent["ClassName"]; n)
                    {
                        sc.className = n.as<std::string>();
                        Log::Info(std::format("  ScriptComponent: ClassName = {}", sc.className));
                    }
                }
                auto scriptContainerNode = entity["ScriptContainerComponent"];
                if (scriptContainerNode && scriptContainerNode.IsMap())
                {
                    auto &sc = deserializedEntity.addComponent<ScriptContainerComponent>();

                    auto classNamesNode = scriptContainerNode["ScriptClassNames"];
                    if (classNamesNode && classNamesNode.IsSequence())
                    {
                        for (auto classNode : classNamesNode)
                            sc.scriptClassNames.push_back(classNode.as<std::string>());
                    }
                }
                auto animatorNode = entity["AnimatorComponent"];
                if (animatorNode && animatorNode.IsMap())
                {
                    auto &ac = deserializedEntity.addComponent<AnimatorComponent>();
                    if (auto n = animatorNode["SkeletonHandle"]; n)
                    {
                        uint64_t handleValue = n.as<uint64_t>();
                        if (handleValue != 0)
                            ac.skeletonHandle = AssetHandle(handleValue);
                    }
                    if (auto n = animatorNode["ActiveClipIndex"]; n)
                        ac.activeClipIndex = n.as<uint32_t>();
                    if (auto n = animatorNode["Speed"]; n)
                        ac.speed = n.as<float>();
                    if (auto n = animatorNode["Playing"]; n)
                        ac.playing = n.as<bool>();
                    if (auto n = animatorNode["Looping"]; n)
                        ac.looping = n.as<bool>();
                    auto clipsNode = animatorNode["AnimationClips"];
                    if (clipsNode && clipsNode.IsSequence())
                    {
                        ac.animationClipHandles.clear();
                        for (auto clipNode : clipsNode)
                            ac.animationClipHandles.push_back(AssetHandle(clipNode.as<uint64_t>()));
                    }
                }
            }
        }
        return true;
    }

    bool SceneSerializer::deserializeRuntime(const std::filesystem::path &filepath)
    {
        return false;
    }
} // namespace Fermion
