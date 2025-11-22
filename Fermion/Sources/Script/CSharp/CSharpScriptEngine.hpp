#pragma once
#include "Script/ScriptEngine.hpp"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <unordered_map>
#include <memory>
#include <string>

namespace Fermion
{
    // C# 脚本类实现
    class CSharpScriptClass : public ScriptClass
    {
    public:
        CSharpScriptClass(MonoDomain *domain, MonoClass *klass, const std::string &ns, const std::string &name);

        // 实例化脚本对象
        virtual ScriptHandle instantiate() override;
        // 获取脚本方法
        virtual ScriptHandle getMethod(const std::string &name, int parameterCount = 0) override;
        // 调用脚本方法
        virtual void invokeMethod(const ScriptHandle &instance, const ScriptHandle &method, void **params = nullptr) override;

        // 获取字段值
        virtual bool getFieldValue(const ScriptHandle& instance, const std::string& name, void* buffer) override;
        // 设置字段值
        virtual bool setFieldValue(const ScriptHandle& instance, const std::string& name, const void* value) override;

    private:
        MonoDomain *m_domain = nullptr;
        MonoClass *m_class = nullptr;
    };

    // C# 脚本引擎实现
    class CSharpScriptEngine : public IScriptEngine
    {
    public:
        // 初始化脚本引擎
        virtual bool init() override;
        // 关闭脚本引擎
        virtual void shutdown() override;

        // 加载脚本程序集
        virtual bool loadScript(const std::filesystem::path &path) override;

        // 创建脚本实例
        virtual ScriptHandle createInstance(const std::string &className) override;
        // 销毁脚本实例
        virtual void destroyInstance(ScriptHandle instance) override;
        // 调用指定名称的方法
        virtual void invokeMethod(ScriptHandle instance, const std::string &name) override;

        // 获取字段值
        virtual bool getFieldValue(const ScriptHandle& instance, const std::string& name, void* buffer) override;
        // 设置字段值
        virtual bool setFieldValue(const ScriptHandle& instance, const std::string& name, const void* value) override;

    private:
        bool m_initialized = false;
        MonoDomain *m_rootDomain = nullptr;
        std::unordered_map<std::string, std::shared_ptr<CSharpScriptClass>> m_classes;
    };
}
