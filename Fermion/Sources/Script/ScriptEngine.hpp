#pragma once

#include <filesystem>
#include <string>
#include <map>
#include <memory>
#include <vector>

namespace Fermion
{
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

    class ScriptEngine
    {
    public:
     
        static void init();

        
        static void shutdown();
    };

}

