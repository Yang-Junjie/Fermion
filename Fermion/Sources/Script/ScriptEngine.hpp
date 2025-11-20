#pragma once

#include <filesystem>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
namespace Fermion
{
#if 0
    class Entity;

    enum class ScriptFieldType
    {
        None,
        Float,
        Double,
        Bool,
        Int,
        Vector2,
        Vector3,
    };

    struct ScriptField
    {
        ScriptFieldType type{};
        std::string name;
    };

    class IScriptFieldInstance
    {
    public:
        virtual ~IScriptFieldInstance() = default;

        virtual ScriptFieldType getType() const = 0;
        virtual const std::string &getName() const = 0;

        virtual void getValue(void *outBuffer) = 0;
        virtual void setValue(const void *value) = 0;
    };

    class ScriptInstance;

    class ScriptClass
    {
    public:
        virtual ~ScriptClass() = default;

        virtual std::string getName() const = 0;
        virtual std::string getNamespace() const = 0;
        virtual std::vector<ScriptField> getFields() const = 0;
        virtual std::vector<std::string> getMethods() const = 0;

        virtual std::unique_ptr<ScriptInstance> instantiate(Entity entity) = 0;
    };

    class ScriptInstance
    {
    public:
        virtual ~ScriptInstance() = default;

        virtual void invokeOnCreate() = 0;
        virtual void invokeOnUpdate(float ts) = 0;

        virtual IScriptFieldInstance *getField(const std::string &name) = 0;
        virtual void setFieldValue(const std::string &name, const void *value) = 0;

        virtual ScriptClass *getScriptClass() = 0;
    };
#endif
    using Func = void(*)(void*);

  
    class ScriptEngine
    {
    public:
        static void init();
        static bool runScripts();
        static bool registerMethod(const std::string& name, Func func);
        static void shutdown();
    private:
        inline static bool s_initialized = false;
        inline static MonoDomain* s_rootDomain = nullptr;
        inline static MonoDomain* s_appDomain = nullptr;
        inline static std::vector<std::pair<std::string, Func>> s_registeredMethods;
    };

}
