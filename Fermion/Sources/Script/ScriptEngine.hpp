#pragma once
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include "ScriptTypes.hpp"
#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include <map>
#include <vector>

namespace Fermion
{

    class ScriptClass
    {
    public:
        ScriptClass(const std::string &ns, const std::string &name)
            : m_classNamespace(ns), m_className(name) {}

        virtual ~ScriptClass() = default;

        virtual ScriptHandle instantiate() = 0;
        virtual ScriptHandle getMethod(const std::string &name, int parameterCount = 0) = 0;

        virtual void invokeMethod(const ScriptHandle &instance, const ScriptHandle &method, void **params = nullptr) = 0;

        virtual bool getFieldValue(const ScriptHandle &instance, const std::string &name, void *buffer) = 0;
        virtual bool setFieldValue(const ScriptHandle &instance, const std::string &name, const void *value) = 0;

        const std::map<std::string, ScriptField> &getFields() const { return m_fields; }

    protected:
        std::string m_classNamespace;
        std::string m_className;
        std::map<std::string, ScriptField> m_fields;
    };

    class ScriptInstance
    {
    public:
        ScriptInstance(std::shared_ptr<ScriptClass> scriptClass, Entity entity)
            : m_scriptClass(scriptClass)
        {
            m_instance = m_scriptClass->instantiate();

            m_onCreateMethod = m_scriptClass->getMethod("OnCreate", 0);
            m_onUpdateMethod = m_scriptClass->getMethod("OnUpdate", 1);

            UUID uuid = entity.getUUID();
            uint64_t id = (uint64_t)uuid;

            Log::Info(std::format("ScriptInstance: Setting ID field to UUID = {} (uint64_t = {})", uuid.toString(), id));
            bool success = m_scriptClass->setFieldValue(m_instance, "ID", &id);
            Log::Info(std::format("ScriptInstance: setFieldValue result = {}", success));
        }

        virtual ~ScriptInstance() = default;

        void invokeOnCreate()
        {
            if (m_onCreateMethod.isValid())
                m_scriptClass->invokeMethod(m_instance, m_onCreateMethod);
        }

        void invokeOnUpdate(float ts)
        {
            if (m_onUpdateMethod.isValid())
            {
                void *params[1] = {&ts};
                m_scriptClass->invokeMethod(m_instance, m_onUpdateMethod, params);
            }
        }

        template <typename T>
        T getFieldValue(const std::string &name)
        {
            T value;
            if (m_scriptClass->getFieldValue(m_instance, name, &value))
                return value;
            return T();
        }

        template <typename T>
        void setFieldValue(const std::string &name, T value)
        {
            m_scriptClass->setFieldValue(m_instance, name, &value);
        }

        std::shared_ptr<ScriptClass> getScriptClass() const { return m_scriptClass; }

        ScriptHandle getHandle() const { return m_instance; }

    private:
        std::shared_ptr<ScriptClass> m_scriptClass;
        ScriptHandle m_instance;
        ScriptHandle m_onCreateMethod;
        ScriptHandle m_onUpdateMethod;
    };

    class IScriptEngine
    {
    public:
        virtual ~IScriptEngine() = default;

        virtual bool init() = 0;
        virtual void shutdown() = 0;
        virtual bool loadScript(const std::filesystem::path &path) = 0;

        virtual ScriptHandle createInstance(const std::string &className) = 0;
        virtual void destroyInstance(ScriptHandle instance) = 0;

        virtual void invokeMethod(ScriptHandle instance, const std::string &name) = 0;

        virtual bool getFieldValue(const ScriptHandle &instance, const std::string &name, void *buffer) = 0;
        virtual bool setFieldValue(const ScriptHandle &instance, const std::string &name, const void *value) = 0;

        virtual Scene *getSceneContext() const = 0;

        virtual ScriptHandle getManagedInstance(UUID uuid, std::string className) = 0;
        virtual bool entityClassExists(const std::string &fullClassName) = 0;
        
        virtual void onRuntimeStart(Scene *scene) = 0;
        virtual void onRuntimeStop() = 0;

        virtual void onCreateEntity(Entity entity) = 0;
        virtual void onUpdateEntity(Entity entity, Timestep ts) = 0;

        virtual const std::vector<std::string> &getALLEntityClasses() const = 0;

    protected:
        Scene *m_scene = nullptr;
    };
}
