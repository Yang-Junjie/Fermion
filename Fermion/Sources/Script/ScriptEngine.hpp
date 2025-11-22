#pragma once

#include "ScriptTypes.hpp"
#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include <map>
#include <vector>

namespace Fermion
{
    // 抽象的脚本类接口
    class ScriptClass
    {
    public:
        ScriptClass(const std::string &ns, const std::string &name)
            : m_classNamespace(ns), m_className(name) {}

        virtual ~ScriptClass() = default;

        // 实例化脚本对象
        virtual ScriptHandle instantiate() = 0;
        // 获取脚本方法
        virtual ScriptHandle getMethod(const std::string &name, int parameterCount = 0) = 0;
        // 调用脚本方法
        virtual void invokeMethod(const ScriptHandle &instance, const ScriptHandle &method, void **params = nullptr) = 0;

        // 获取字段值
        virtual bool getFieldValue(const ScriptHandle& instance, const std::string& name, void* buffer) = 0;
        // 设置字段值
        virtual bool setFieldValue(const ScriptHandle& instance, const std::string& name, const void* value) = 0;

        // 获取该脚本类的所有字段
        const std::map<std::string, ScriptField> &getFields() const { return m_fields; }

    protected:
        std::string m_classNamespace;
        std::string m_className;
        std::map<std::string, ScriptField> m_fields;
    };

    // 脚本实例封装类，管理脚本对象的生命周期和常用方法
    class ScriptInstance
    {
    public:
        ScriptInstance(std::shared_ptr<ScriptClass> scriptClass)
            : m_scriptClass(scriptClass)
        {
            // 实例化脚本对象
            m_instance = m_scriptClass->instantiate();
            // 获取常用的生命周期方法
            m_onCreateMethod = m_scriptClass->getMethod("OnCreate", 0);
            m_onUpdateMethod = m_scriptClass->getMethod("OnUpdate", 1);
        }

        virtual ~ScriptInstance() = default;

        // 调用 OnCreate 方法
        void invokeOnCreate()
        {
            if (m_onCreateMethod.isValid())
                m_scriptClass->invokeMethod(m_instance, m_onCreateMethod);
        }

        // 调用 OnUpdate 方法
        void invokeOnUpdate(float ts)
        {
            if (m_onUpdateMethod.isValid())
            {
                void *params[1] = {&ts};
                m_scriptClass->invokeMethod(m_instance, m_onUpdateMethod, params);
            }
        }

        // 获取字段值
        template<typename T>
        T getFieldValue(const std::string& name)
        {
            T value;
            if (m_scriptClass->getFieldValue(m_instance, name, &value))
                return value;
            return T();
        }

        // 设置字段值
        template<typename T>
        void setFieldValue(const std::string& name, T value)
        {
            m_scriptClass->setFieldValue(m_instance, name, &value);
        }

        // 获取关联的脚本类
        std::shared_ptr<ScriptClass> getScriptClass() const { return m_scriptClass; }
        // 获取脚本对象句柄
        ScriptHandle getHandle() const { return m_instance; }

    private:
        std::shared_ptr<ScriptClass> m_scriptClass;
        ScriptHandle m_instance;
        ScriptHandle m_onCreateMethod;
        ScriptHandle m_onUpdateMethod;
    };

    // 抽象脚本引擎接口
    class IScriptEngine
    {
    public:
        virtual ~IScriptEngine() = default;

        // 初始化脚本引擎
        virtual bool init() = 0;
        // 关闭脚本引擎
        virtual void shutdown() = 0;

        // 加载脚本程序集
        virtual bool loadScript(const std::filesystem::path &path) = 0;

        // 创建脚本实例
        virtual ScriptHandle createInstance(const std::string &className) = 0;
        // 销毁脚本实例
        virtual void destroyInstance(ScriptHandle instance) = 0;
        // 调用指定名称的方法
        virtual void invokeMethod(ScriptHandle instance, const std::string &name) = 0;

        // 获取字段值
        virtual bool getFieldValue(const ScriptHandle& instance, const std::string& name, void* buffer) = 0;
        // 设置字段值
        virtual bool setFieldValue(const ScriptHandle& instance, const std::string& name, const void* value) = 0;
    };
}
