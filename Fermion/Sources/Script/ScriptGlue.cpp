#include "fmpch.hpp"
#include "ScriptGlue.hpp"
#include "ScriptEngine.hpp"
#include "ScriptManager.hpp"
#include "ScriptTypes.hpp"

#include "Core/UUID.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/Input.hpp"

#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Components.hpp"
#include "Physics/Physics2D.hpp"
#include "imgui/ConsolePanel.hpp"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"
#include <box2d/box2d.h>

namespace Fermion
{

    namespace Utils
    {

        std::string monoStringToString(MonoString *string)
        {
            char *cStr = mono_string_to_utf8(string);
            std::string str(cStr);
            mono_free(cStr);
            return str;
        }
    }
#define FM_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Fermion.InternalCalls::" #Name, Name)
    static std::unordered_map<MonoType *, std::function<bool(Entity)>> s_entityHasComponentFuncs;
    static std::unordered_map<MonoType *, std::function<void(Entity)>> s_entitycomponentFactories;

#pragma region Scene
    extern "C" static uint64_t Scene_CreateEntity(MonoString *tag)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null");
        std::string tagStr = Utils::monoStringToString(tag);
        return scene->createEntity(tagStr).getUUID();
    }
#pragma endregion


#pragma region Log
    extern "C" static void NativeLog(MonoString *string, int parameter)
    {
        std::string str = Utils::monoStringToString(string);
        Log::Info(str + " " + std::to_string(parameter));
    }

    extern "C" static void ConsoleLog(MonoString *string)
    {
        std::string str = Utils::monoStringToString(string);
        ConsolePanel::get().addLog(str.c_str());
    }
#pragma endregion

#pragma region Entity
    extern "C" static MonoObject *GetScriptInstance(UUID entityID, std::string className)
    {
        return (MonoObject *)ScriptManager::getManagedInstance(entityID, className).m_instance;
    }

    extern "C" static bool Entity_HasComponent(UUID entityID, MonoReflectionType *componentType)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null");

        MonoType *managedType = mono_reflection_type_get_type(componentType);
        FERMION_ASSERT(s_entityHasComponentFuncs.find(managedType) != s_entityHasComponentFuncs.end(), "Component type is not registered");
        return s_entityHasComponentFuncs.at(managedType)(entity);
    }
    extern "C" static void Entity_AddComponent(uint64_t entityID, MonoReflectionType *componentType)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null");

        MonoType *type = mono_reflection_type_get_type(componentType);
        auto it = s_entitycomponentFactories.find(type);
        FERMION_ASSERT(it != s_entitycomponentFactories.end(), "Component type not registered");

        it->second(entity);
    }

    extern "C" static uint64_t Entity_FindEntityByName(MonoString *name)
    {
        char *nameCStr = mono_string_to_utf8(name);

        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->findEntityByName(nameCStr);
        mono_free(nameCStr);

        if (!entity)
            return 0;

        return entity.getUUID();
    }
#pragma endregion

#pragma region TransformComponent
    extern "C" static void TransformComponent_GetTranslation(UUID entityID, glm::vec3 *outTranslation)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        *outTranslation = entity.getComponent<TransformComponent>().translation;
    }

    extern "C" static void TransformComponent_SetTranslation(UUID entityID, glm::vec3 *translation)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        entity.getComponent<TransformComponent>().translation = *translation;
    }
#pragma endregion


#pragma region SpriteRendererComponent
    extern "C" static void SpriteRendererComponent_SetColor(UUID entityID, glm::vec4 *color)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        entity.getComponent<SpriteRendererComponent>().color = *color;
    }
#pragma region 

#pragma region Box2DComponent
    extern "C" static Rigidbody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
        uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
        b2BodyId bodyId = b2LoadBodyId(storedId);
        b2BodyType bodyType = b2Body_GetType(bodyId);
        return Utils::Rigidbody2DTypeFromBox2DBody(bodyType);
    }

    extern "C" static void Rigidbody2DComponent_SetType(UUID entityID, Rigidbody2DComponent::BodyType bodyType)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
        uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
        b2BodyId bodyId = b2LoadBodyId(storedId);
        b2Body_SetType(bodyId, Utils::Rigidbody2DTypeToBox2DBody(bodyType));
    }

    extern "C" static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2 *impulse, bool wake)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
        uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
        b2BodyId bodyId = b2LoadBodyId(storedId);

        b2Body_ApplyLinearImpulseToCenter(bodyId, b2Vec2{impulse->x, impulse->y}, wake);
    }

    extern "C" static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, glm::vec2 *out)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
        uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
        b2BodyId bodyId = b2LoadBodyId(storedId);

        b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
        out->x = vel.x;
        out->y = vel.y;
    }
    extern "C" static bool BoxSensor2D_SensorBegin(UUID entityID)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();

        return bs2c.sensorBegin;
    }
    extern "C" static bool BoxSensor2D_SensorEnd(UUID entityID)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();

        return bs2c.sensorEnd;
    }
    extern "C" static void BoxSensor2D_SetSize(UUID entityID, glm::vec2 *out)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();
        bs2c.size = *out;
        scene->initPhysicsSensor(entity);
    }
    extern "C" static void BoxSensor2D_GetSize(UUID entityID, glm::vec2 *out)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();
        *out = bs2c.size;
    }
    extern "C" static void BoxSensor2D_SetOffset(UUID entityID, glm::vec2 *out)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();
        bs2c.offset = *out;
        scene->initPhysicsSensor(entity);
    }
    extern "C" static void BoxSensor2D_GetOffset(UUID entityID, glm::vec2 *out)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        auto &bs2c = entity.getComponent<BoxSensor2DComponent>();
        *out = bs2c.offset;
    }
#pragma endregion

#pragma region Input
    extern "C" static bool
    Input_IsKeyDown(KeyCode keycode)
    {
        return Input::isKeyPressed(keycode);
    }
#pragma endregion

    template <typename... Components>
    static void RegisterComponent()
    {
        if constexpr (sizeof...(Components) == 0)
        {

            return;
        }
        else if constexpr (sizeof...(Components) == 1)
        {

            using Component = std::tuple_element_t<0, std::tuple<Components...>>;

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

            MonoImage *image = ScriptManager::getCoreAssemblyImage();
            if (!image)
            {
                Log::Error(std::format("Component registration failed: Core assembly image not available"));
                return;
            }

            MonoClass *monoClass = mono_class_from_name(image, "Fermion", componentName.c_str());
            if (!monoClass)
            {
                Log::Warn(std::format("Component '{}' not found in C# assembly, skipping registration", componentName));
                return;
            }

            MonoType *managedType = mono_class_get_type(monoClass);
            if (!managedType)
            {
                Log::Error(std::format("Failed to get MonoType for component '{}'", componentName));
                return;
            }

            s_entityHasComponentFuncs[managedType] = [](Entity entity)
            { return entity.hasComponent<Component>(); };

            Log::Info(std::format("Registered component: {}", componentName));
        }
        else
        {
            using FirstComponent = std::tuple_element_t<0, std::tuple<Components...>>;
            RegisterComponent<FirstComponent>();

            []<std::size_t... Is>(std::index_sequence<Is...>)
            {
                RegisterComponent<std::tuple_element_t<Is + 1, std::tuple<Components...>>...>();
            }(std::make_index_sequence<sizeof...(Components) - 1>{});
        }
    }

    template <typename Component>
    void registerComponentFactory(MonoImage *image)
    {
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

        s_entitycomponentFactories[type] = [](Entity entity)
        {
            if constexpr (std::is_same_v<Component, ScriptContainerComponent>)
            {
                entity.getComponent<ScriptContainerComponent>();
            }
            else
            {
                auto &component = entity.addComponent<Component>();
                if (entity.hasComponent<BoxSensor2DComponent>())
                {
                    ScriptManager::getSceneContext()->initPhysicsSensor(entity);
                    Log::Warn(std::format("entity uuid{}", entity.getUUID().toString()));
                }
            }
        };
    }
    template <typename... Components>
    void registerAllComponentFactories(MonoImage *image)
    {
        (registerComponentFactory<Components>(image), ...);
    }

    template <typename... Components>
    void registerAllComponentFactories(ComponentGroup<Components...> group, MonoImage *image)
    {
        registerAllComponentFactories<Components...>(image);
    }

    template <typename... Component>
    static void RegisterComponent(ComponentGroup<Component...>)
    {
        RegisterComponent<Component...>();
    }

    void ScriptGlue::registerComponents()
    {
        s_entityHasComponentFuncs.clear();
        RegisterComponent(AllComponents{});
    }
    void ScriptGlue::registerComponentFactories()
    {
        s_entitycomponentFactories.clear();
        MonoImage *image = ScriptManager::getCoreAssemblyImage();
        registerAllComponentFactories(AllComponents{}, image);
    }

    void ScriptGlue::registerFunctions()
    {

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

        FM_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
        FM_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);
        FM_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
        FM_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);

        FM_ADD_INTERNAL_CALL(BoxSensor2D_SensorBegin);
        FM_ADD_INTERNAL_CALL(BoxSensor2D_SensorEnd);
        FM_ADD_INTERNAL_CALL(BoxSensor2D_SetSize);
        FM_ADD_INTERNAL_CALL(BoxSensor2D_GetSize);
        FM_ADD_INTERNAL_CALL(BoxSensor2D_SetOffset);
        FM_ADD_INTERNAL_CALL(BoxSensor2D_GetOffset);

        FM_ADD_INTERNAL_CALL(Input_IsKeyDown);
    }
}