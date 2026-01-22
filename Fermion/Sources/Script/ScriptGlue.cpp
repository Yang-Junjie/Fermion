#include "fmpch.hpp"
#include "ScriptGlue.hpp"
#include "ScriptEngine.hpp"
#include "ScriptManager.hpp"
#include "ScriptTypes.hpp"

#include "Core/UUID.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/Input.hpp"

#include "Scene/Scene.hpp"
#include "Scene/EntityManager.hpp"
#include "Renderer/Renderers/SceneRenderer.hpp"
#include "Renderer/Renderers/DebugRenderer.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Components.hpp"
#include "Physics/Physics2D.hpp"
#include "Physics/Physics3D.hpp"
#include "ImGui/ConsolePanel.hpp"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"
#include <box2d/box2d.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace Fermion {
    namespace Utils {
        std::string monoStringToString(MonoString *string) {
            char *cStr = mono_string_to_utf8(string);
            std::string str(cStr);
            mono_free(cStr);
            return str;
        }
    } // namespace Utils
    namespace {
        constexpr JPH::ObjectLayer kNonMovingLayer = 0;
        constexpr JPH::ObjectLayer kMovingLayer = 1;

        JPH::EMotionType toJoltMotionType(Rigidbody3DComponent::BodyType type) {
            switch (type) {
                case Rigidbody3DComponent::BodyType::Static: return JPH::EMotionType::Static;
                case Rigidbody3DComponent::BodyType::Kinematic: return JPH::EMotionType::Kinematic;
                case Rigidbody3DComponent::BodyType::Dynamic:
                default: return JPH::EMotionType::Dynamic;
            }
        }

        Rigidbody3DComponent::BodyType fromJoltMotionType(JPH::EMotionType type) {
            switch (type) {
                case JPH::EMotionType::Static: return Rigidbody3DComponent::BodyType::Static;
                case JPH::EMotionType::Kinematic: return Rigidbody3DComponent::BodyType::Kinematic;
                case JPH::EMotionType::Dynamic:
                default: return Rigidbody3DComponent::BodyType::Dynamic;
            }
        }

        JPH::Vec3 toJoltVec3(const glm::vec3 &value) {
            return {value.x, value.y, value.z};
        }

        glm::vec3 toGlmVec3(JPH::Vec3Arg value) {
            return {value.GetX(), value.GetY(), value.GetZ()};
        }

        bool tryGetBodyInterfaceAndID(Scene *scene, Rigidbody3DComponent &rb, JPH::BodyInterface **outInterface,
                                      JPH::BodyID &outBodyID) {
            if (!scene)
                return false;

            Physics3DWorld *world = scene->getPhysicsWorld3D();
            if (!world || !world->isActive())
                return false;

            JPH::PhysicsSystem *physicsSystem = world->getPhysicsSystem();
            if (!physicsSystem)
                return false;

            if (!rb.runtimeBody)
                return false;

            uint64_t storedValue = reinterpret_cast<uint64_t>(rb.runtimeBody);
            if (storedValue == 0)
                return false;

            JPH::BodyID bodyID(static_cast<uint32_t>(storedValue));
            JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
            if (!bodyInterface.IsAdded(bodyID))
                return false;

            if (outInterface)
                *outInterface = &bodyInterface;
            outBodyID = bodyID;
            return true;
        }
    } // namespace
#define FM_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Fermion.InternalCalls::" #Name, (void *)Name)
    static std::unordered_map<MonoType *, std::function<bool(Entity)> > s_entityHasComponentFuncs;
    static std::unordered_map<MonoType *, std::function<void(Entity)> > s_entitycomponentFactories;

    // #pragma region AssetHandle

    // #pragma endregin
#pragma region Scene
    extern "C" uint64_t Scene_CreateEntity(MonoString *tag) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null");
        std::string tagStr = Utils::monoStringToString(tag);
        return scene->createEntity(tagStr).getUUID();
    }
#pragma endregion

#pragma region Log
    extern "C" void NativeLog(MonoString *string, int parameter) {
        std::string str = Utils::monoStringToString(string);
        Log::Info(str + " " + std::to_string(parameter));
    }

    extern "C" void ConsoleLog(MonoString *string) {
        std::string str = Utils::monoStringToString(string);
        ConsolePanel::get().addLog(str.c_str());
    }
#pragma endregion

#pragma region Entity
    extern "C" MonoObject *GetScriptInstance(UUID entityID, std::string className) {
        return (MonoObject *) ScriptManager::getManagedInstance(entityID, className).m_instance;
    }

    extern "C" bool Entity_HasComponent(UUID entityID, MonoReflectionType *componentType) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null");

        MonoType *managedType = mono_reflection_type_get_type(componentType);
        FERMION_ASSERT(s_entityHasComponentFuncs.find(managedType) != s_entityHasComponentFuncs.end(),
                       "Component type is not registered");
        return s_entityHasComponentFuncs.at(managedType)(entity);
    }

    extern "C" void Entity_AddComponent(uint64_t entityID, MonoReflectionType *componentType) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null");

        MonoType *type = mono_reflection_type_get_type(componentType);
        auto it = s_entitycomponentFactories.find(type);
        FERMION_ASSERT(it != s_entitycomponentFactories.end(), "Component type not registered");

        it->second(entity);
    }

    extern "C" uint64_t Entity_FindEntityByName(MonoString *name) {
        char *nameCStr = mono_string_to_utf8(name);

        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().findEntityByName(nameCStr);
        mono_free(nameCStr);

        if (!entity)
            return 0;

        return entity.getUUID();
    }
#pragma endregion

#pragma region TransformComponent
    extern "C" void TransformComponent_GetTranslation(UUID entityID, glm::vec3 *outTranslation) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        *outTranslation = entity.getComponent<TransformComponent>().translation;
    }

    extern "C" void TransformComponent_SetTranslation(UUID entityID, glm::vec3 *translation) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        entity.getComponent<TransformComponent>().translation = *translation;
    }
#pragma endregion

#pragma region SpriteRendererComponent
    extern "C" void SpriteRendererComponent_SetColor(UUID entityID, glm::vec4 *color) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        entity.getComponent<SpriteRendererComponent>().color = *color;
    }

    extern "C" void SpriteRendererComponent_SetTexture(UUID entityID, uint64_t textureID) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        entity.getComponent<SpriteRendererComponent>().textureHandle = textureID;
    }
#pragma region

#pragma region Box2DComponent
    extern "C" Rigidbody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
        uint64_t storedId = (uint64_t) (uintptr_t) rb2d.runtimeBody;
        b2BodyId bodyId = b2LoadBodyId(storedId);
        b2BodyType bodyType = b2Body_GetType(bodyId);
        return Utils::Rigidbody2DTypeFromBox2DBody(bodyType);
    }

    extern "C" void Rigidbody2DComponent_SetType(UUID entityID, Rigidbody2DComponent::BodyType bodyType) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
        uint64_t storedId = (uint64_t) (uintptr_t) rb2d.runtimeBody;
        b2BodyId bodyId = b2LoadBodyId(storedId);
        b2Body_SetType(bodyId, Utils::Rigidbody2DTypeToBox2DBody(bodyType));
    }

    extern "C" void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2 *impulse, bool wake) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
        uint64_t storedId = (uint64_t) (uintptr_t) rb2d.runtimeBody;
        b2BodyId bodyId = b2LoadBodyId(storedId);

        b2Body_ApplyLinearImpulseToCenter(bodyId, b2Vec2{impulse->x, impulse->y}, wake);
    }

    extern "C" void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, glm::vec2 *out) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
        uint64_t storedId = (uint64_t) (uintptr_t) rb2d.runtimeBody;
        b2BodyId bodyId = b2LoadBodyId(storedId);

        b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
        out->x = vel.x;
        out->y = vel.y;
    }

    extern "C" bool BoxSensor2D_SensorBegin(UUID entityID) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();

        return bs2c.sensorBegin;
    }

    extern "C" bool BoxSensor2D_SensorEnd(UUID entityID) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();

        return bs2c.sensorEnd;
    }

    extern "C" void BoxSensor2D_SetSize(UUID entityID, glm::vec2 *out) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();
        bs2c.size = *out;
        scene->initPhysicsSensor(entity);
    }

    extern "C" void BoxSensor2D_GetSize(UUID entityID, glm::vec2 *out) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();
        *out = bs2c.size;
    }

    extern "C" void BoxSensor2D_SetOffset(UUID entityID, glm::vec2 *out) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();
        bs2c.offset = *out;
        scene->initPhysicsSensor(entity);
    }

    extern "C" void BoxSensor2D_GetOffset(UUID entityID, glm::vec2 *out) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();
        *out = bs2c.offset;
    }
#pragma endregion

#pragma region Rigidbody3DComponent
    extern "C" Rigidbody3DComponent::BodyType Rigidbody3DComponent_GetType(UUID entityID) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb3d = entity.getComponent<Rigidbody3DComponent>();
        JPH::BodyInterface *bodyInterface = nullptr;
        JPH::BodyID bodyID;
        if (!tryGetBodyInterfaceAndID(scene, rb3d, &bodyInterface, bodyID))
            return rb3d.type;

        return fromJoltMotionType(bodyInterface->GetMotionType(bodyID));
    }

    extern "C" void Rigidbody3DComponent_SetType(UUID entityID, Rigidbody3DComponent::BodyType bodyType) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb3d = entity.getComponent<Rigidbody3DComponent>();
        rb3d.type = bodyType;

        JPH::BodyInterface *bodyInterface = nullptr;
        JPH::BodyID bodyID;
        if (!tryGetBodyInterfaceAndID(scene, rb3d, &bodyInterface, bodyID))
            return;

        JPH::EMotionType motionType = toJoltMotionType(bodyType);
        JPH::EActivation activation = motionType == JPH::EMotionType::Static ? JPH::EActivation::DontActivate
                                                                              : JPH::EActivation::Activate;
        bodyInterface->SetMotionType(bodyID, motionType, activation);
        bodyInterface->SetObjectLayer(bodyID,
                                      motionType == JPH::EMotionType::Static ? kNonMovingLayer : kMovingLayer);
    }

    extern "C" void Rigidbody3DComponent_GetLinearVelocity(UUID entityID, glm::vec3 *out) {
        if (!out)
            return;

        *out = glm::vec3{0.0f};

        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb3d = entity.getComponent<Rigidbody3DComponent>();
        JPH::BodyInterface *bodyInterface = nullptr;
        JPH::BodyID bodyID;
        if (!tryGetBodyInterfaceAndID(scene, rb3d, &bodyInterface, bodyID))
            return;

        *out = toGlmVec3(bodyInterface->GetLinearVelocity(bodyID));
    }

    extern "C" void Rigidbody3DComponent_SetLinearVelocity(UUID entityID, glm::vec3 *velocity) {
        if (!velocity)
            return;

        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb3d = entity.getComponent<Rigidbody3DComponent>();
        if (rb3d.type == Rigidbody3DComponent::BodyType::Static)
            return;

        JPH::BodyInterface *bodyInterface = nullptr;
        JPH::BodyID bodyID;
        if (!tryGetBodyInterfaceAndID(scene, rb3d, &bodyInterface, bodyID))
            return;

        bodyInterface->SetLinearVelocity(bodyID, toJoltVec3(*velocity));
    }

    extern "C" void Rigidbody3DComponent_GetAngularVelocity(UUID entityID, glm::vec3 *out) {
        if (!out)
            return;

        *out = glm::vec3{0.0f};

        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb3d = entity.getComponent<Rigidbody3DComponent>();
        JPH::BodyInterface *bodyInterface = nullptr;
        JPH::BodyID bodyID;
        if (!tryGetBodyInterfaceAndID(scene, rb3d, &bodyInterface, bodyID))
            return;

        *out = toGlmVec3(bodyInterface->GetAngularVelocity(bodyID));
    }

    extern "C" void Rigidbody3DComponent_SetAngularVelocity(UUID entityID, glm::vec3 *velocity) {
        if (!velocity)
            return;

        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb3d = entity.getComponent<Rigidbody3DComponent>();
        if (rb3d.type == Rigidbody3DComponent::BodyType::Static)
            return;

        JPH::BodyInterface *bodyInterface = nullptr;
        JPH::BodyID bodyID;
        if (!tryGetBodyInterfaceAndID(scene, rb3d, &bodyInterface, bodyID))
            return;

        bodyInterface->SetAngularVelocity(bodyID, toJoltVec3(*velocity));
    }

    extern "C" void Rigidbody3DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec3 *impulse, bool wake) {
        if (!impulse)
            return;

        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb3d = entity.getComponent<Rigidbody3DComponent>();
        if (rb3d.type == Rigidbody3DComponent::BodyType::Static)
            return;

        JPH::BodyInterface *bodyInterface = nullptr;
        JPH::BodyID bodyID;
        if (!tryGetBodyInterfaceAndID(scene, rb3d, &bodyInterface, bodyID))
            return;

        bodyInterface->AddImpulse(bodyID, toJoltVec3(*impulse));
        if (wake)
            bodyInterface->ActivateBody(bodyID);
    }

    extern "C" void Rigidbody3DComponent_ApplyAngularImpulse(UUID entityID, glm::vec3 *impulse, bool wake) {
        if (!impulse)
            return;

        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb3d = entity.getComponent<Rigidbody3DComponent>();
        if (rb3d.type == Rigidbody3DComponent::BodyType::Static)
            return;

        JPH::BodyInterface *bodyInterface = nullptr;
        JPH::BodyID bodyID;
        if (!tryGetBodyInterfaceAndID(scene, rb3d, &bodyInterface, bodyID))
            return;

        bodyInterface->AddAngularImpulse(bodyID, toJoltVec3(*impulse));
        if (wake)
            bodyInterface->ActivateBody(bodyID);
    }

    extern "C" void Rigidbody3DComponent_AddForce(UUID entityID, glm::vec3 *force, bool wake) {
        if (!force)
            return;

        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb3d = entity.getComponent<Rigidbody3DComponent>();
        if (rb3d.type == Rigidbody3DComponent::BodyType::Static)
            return;

        JPH::BodyInterface *bodyInterface = nullptr;
        JPH::BodyID bodyID;
        if (!tryGetBodyInterfaceAndID(scene, rb3d, &bodyInterface, bodyID))
            return;

        bodyInterface->AddForce(bodyID, toJoltVec3(*force),
                                wake ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
    }
#pragma endregion

#pragma region Input
    extern "C" bool Input_IsKeyDown(KeyCode keycode) {
        return Input::isKeyPressed(keycode);
    }
#pragma endregion

#pragma region DebugDraw
    extern "C" void DebugRenderer_DrawLine(glm::vec3 *start, glm::vec3 *end, glm::vec4 *color) {
        std::shared_ptr<SceneRenderer> renderer = ScriptManager::get()->getSceneRenderer();
        std::shared_ptr<DebugRenderer> debugRenderer = renderer->GetDebugRenderer();
        debugRenderer->DrawLine(*start, *end, *color);
    }

    extern "C" void DebugRenderer_SetLineWidth(float width) {
        std::shared_ptr<SceneRenderer> renderer = ScriptManager::get()->getSceneRenderer();
        std::shared_ptr<DebugRenderer> debugRenderer = renderer->GetDebugRenderer();
        debugRenderer->SetLineWidth(width);
    }
    extern "C" void DebugRenderer_DrawQuadBillboard(glm::vec3* translation, glm::vec2* size, glm::vec4* color) {
        std::shared_ptr<SceneRenderer> renderer = ScriptManager::get()->getSceneRenderer();
        std::shared_ptr<DebugRenderer> debugRenderer = renderer->GetDebugRenderer();
        debugRenderer->drawQuadBillboard(*translation, *size, *color);
    }
#pragma endregion

#pragma region TextComponent
    extern "C" void TextComponent_SetText(UUID entityID, MonoString *string) {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityManager().getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");
        entity.getComponent<TextComponent>().textString = Utils::monoStringToString(string);
    }
#pragma endregion

    template<typename... Components>
    static void RegisterComponent() {
        if constexpr (sizeof...(Components) == 0) {
            return;
        } else if constexpr (sizeof...(Components) == 1) {
            using Component = std::tuple_element_t<0, std::tuple<Components...> >;

            std::string_view typeName = typeid(Component).name();

            std::string componentName(typeName);
            size_t pos = componentName.find("struct ");
            if (pos != std::string::npos)
                componentName = componentName.substr(pos + 7);
            pos = componentName.find("class ");
            if (pos != std::string::npos)
                componentName = componentName.substr(pos + 6);

            pos = componentName.find_last_of("::");
            if (pos != std::string::npos && pos + 1 < componentName.length())
                componentName = componentName.substr(pos + 1);

            MonoImage *image = ScriptManager::getCoreImage();
            if (!image) {
                Log::Error(std::format("Component registration failed: Core assembly image not available"));
                return;
            }

            MonoClass *monoClass = mono_class_from_name(image, "Fermion", componentName.c_str());
            if (!monoClass) {
                Log::Warn(std::format("Component '{}' not found in C# assembly, skipping registration", componentName));
                return;
            }

            MonoType *managedType = mono_class_get_type(monoClass);
            if (!managedType) {
                Log::Error(std::format("Failed to get MonoType for component '{}'", componentName));
                return;
            }

            s_entityHasComponentFuncs[managedType] = [](Entity entity) { return entity.hasComponent<Component>(); };

            Log::Info(std::format("Registered component: {}", componentName));
        } else {
            using FirstComponent = std::tuple_element_t<0, std::tuple<Components...> >;
            RegisterComponent<FirstComponent>();

            []<std::size_t... Is>(std::index_sequence<Is...>) {
                RegisterComponent<std::tuple_element_t<Is + 1, std::tuple<Components...> >...>();
            }(std::make_index_sequence<sizeof...(Components) - 1>{});
        }
    }

    template<typename Component>
    void registerComponentFactory(MonoImage *image) {
        std::string componentName = typeid(Component).name();
        size_t pos = componentName.find("struct ");
        if (pos != std::string::npos)
            componentName = componentName.substr(pos + 7);
        pos = componentName.find("class ");
        if (pos != std::string::npos)
            componentName = componentName.substr(pos + 6);
        pos = componentName.find_last_of("::");
        if (pos != std::string::npos)
            componentName = componentName.substr(pos + 1);

        MonoClass *monoClass = mono_class_from_name(image, "Fermion", componentName.c_str());
        if (!monoClass)
            return;

        MonoType *type = mono_class_get_type(monoClass);
        if (!type)
            return;

        s_entitycomponentFactories[type] = [](Entity entity) {
            if constexpr (std::is_same_v<Component, ScriptContainerComponent>) {
                entity.getComponent<ScriptContainerComponent>();
            } else {
                auto &component = entity.addComponent<Component>();
                if (entity.hasComponent<BoxSensor2DComponent>()) {
                    ScriptManager::getSceneContext()->initPhysicsSensor(entity);
                }
            }
        };
    }

    template<typename... Components>
    void registerAllComponentFactories(MonoImage *image) {
        (registerComponentFactory<Components>(image), ...);
    }

    template<typename... Components>
    void registerAllComponentFactories(ComponentGroup<Components...> group, MonoImage *image) {
        registerAllComponentFactories<Components...>(image);
    }

    template<typename... Component>
    static void RegisterComponent(ComponentGroup<Component...>) {
        RegisterComponent<Component...>();
    }

    void ScriptGlue::registerComponents() {
        s_entityHasComponentFuncs.clear();
        RegisterComponent(AllComponents{});
    }

    void ScriptGlue::registerComponentFactories() {
        s_entitycomponentFactories.clear();
        MonoImage *coreImage = ScriptManager::getCoreImage();
        registerAllComponentFactories(AllComponents{}, coreImage);
    }

    void ScriptGlue::registerFunctions() {
        FM_ADD_INTERNAL_CALL(Scene_CreateEntity);

        FM_ADD_INTERNAL_CALL(NativeLog);
        FM_ADD_INTERNAL_CALL(ConsoleLog);

        FM_ADD_INTERNAL_CALL(GetScriptInstance);
        FM_ADD_INTERNAL_CALL(Entity_HasComponent);
        FM_ADD_INTERNAL_CALL(Entity_AddComponent);
        FM_ADD_INTERNAL_CALL(Entity_FindEntityByName);

        FM_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
        FM_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

        FM_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);
        FM_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTexture);

        FM_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
        FM_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);
        FM_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
        FM_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
        
        FM_ADD_INTERNAL_CALL(Rigidbody3DComponent_GetType);
        FM_ADD_INTERNAL_CALL(Rigidbody3DComponent_SetType);
        FM_ADD_INTERNAL_CALL(Rigidbody3DComponent_ApplyLinearImpulseToCenter);
        FM_ADD_INTERNAL_CALL(Rigidbody3DComponent_ApplyAngularImpulse);
        FM_ADD_INTERNAL_CALL(Rigidbody3DComponent_AddForce);
        FM_ADD_INTERNAL_CALL(Rigidbody3DComponent_GetLinearVelocity);
        FM_ADD_INTERNAL_CALL(Rigidbody3DComponent_SetLinearVelocity);
        FM_ADD_INTERNAL_CALL(Rigidbody3DComponent_GetAngularVelocity);
        FM_ADD_INTERNAL_CALL(Rigidbody3DComponent_SetAngularVelocity);

        FM_ADD_INTERNAL_CALL(BoxSensor2D_SensorBegin);
        FM_ADD_INTERNAL_CALL(BoxSensor2D_SensorEnd);
        FM_ADD_INTERNAL_CALL(BoxSensor2D_SetSize);
        FM_ADD_INTERNAL_CALL(BoxSensor2D_GetSize);
        FM_ADD_INTERNAL_CALL(BoxSensor2D_SetOffset);
        FM_ADD_INTERNAL_CALL(BoxSensor2D_GetOffset);

        FM_ADD_INTERNAL_CALL(Input_IsKeyDown);

        FM_ADD_INTERNAL_CALL(DebugRenderer_DrawLine);
        FM_ADD_INTERNAL_CALL(DebugRenderer_SetLineWidth);
        FM_ADD_INTERNAL_CALL(DebugRenderer_DrawQuadBillboard);

        FM_ADD_INTERNAL_CALL(TextComponent_SetText);
    }
} // namespace Fermion
