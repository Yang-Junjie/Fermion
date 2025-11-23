#include "fmpch.hpp"
#include "ScriptGlue.hpp"
#include "ScriptEngine.hpp"
#include "ScriptManager.hpp"

#include "Core/UUID.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/Input.hpp"

#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

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
    static std::unordered_map<MonoType *, std::function<bool(Entity)>> s_entityHasComponentFuncs;
    // C++ 实现的内部调用函数示例
    extern "C" static void NativeLog(MonoString *string, int parameter)
    {
        std::string str = Utils::monoStringToString(string);
        Log::Info(str + " " + std::to_string(parameter));
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

    extern "C" static void TransformComponent_GetTranslation(UUID entityID, glm::vec3 *outTranslation)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        *outTranslation = entity.getComponent<TransformComponent>().translation;
    }

    static void TransformComponent_SetTranslation(UUID entityID, glm::vec3 *translation)
    {
        Scene *scene = ScriptManager::getSceneContext();
        FERMION_ASSERT(scene, "Scene is null!");
        Entity entity = scene->getEntityByUUID(entityID);
        FERMION_ASSERT(entity, "Entity is null!");

        entity.getComponent<TransformComponent>().translation = *translation;
    }

    static bool Input_IsKeyDown(KeyCode keycode)
    {
        return Input::isKeyPressed(keycode);
    }
#define FM_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Fermion.InternalCalls::" #Name, Name)

    // 注册组件
    void ScriptGlue::registerComponents()
    {
        // TODO: 注册可在 C# 使用的组件，比如 Transform、RigidBody 等
        // 这里将来会包含遍历实体组件并注册到 Mono 运行时的逻辑
    }

    // 注册内部函数
    void ScriptGlue::registerFunctions()
    {

        FM_ADD_INTERNAL_CALL(NativeLog);
        FM_ADD_INTERNAL_CALL(Entity_HasComponent);
        FM_ADD_INTERNAL_CALL(Entity_FindEntityByName);

        FM_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
        FM_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
        FM_ADD_INTERNAL_CALL(Input_IsKeyDown);
    }
}