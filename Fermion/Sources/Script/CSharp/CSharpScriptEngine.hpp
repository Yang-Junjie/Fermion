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
        virtual bool getFieldValue(const ScriptHandle &instance, const std::string &name, void *buffer) override;
        // 设置字段值
        virtual bool setFieldValue(const ScriptHandle &instance, const std::string &name, const void *value) override;

    private:
        MonoDomain *m_domain = nullptr;
        MonoClass *m_class = nullptr;
    };
    using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;
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
        virtual bool getFieldValue(const ScriptHandle &instance, const std::string &name, void *buffer) override;
        // 设置字段值
        virtual bool setFieldValue(const ScriptHandle &instance, const std::string &name, const void *value) override;

        virtual void onRuntimeStart(Scene *scene) override;
        virtual Scene *getSceneContext() const override;
        virtual void onRuntimeStop() override;

        // 检查类是否存在
        virtual bool entityClassExists(const std::string &fullClassName) override;
        // 创建实例
        virtual void onCreateEntity(Entity entity) override;
        // 更新实例
        virtual void onUpdateEntity(Entity entity, Timestep ts) override;

        virtual const std::vector<std::string> &getALLEntityClasses() const override;

    private:
        // 辅助方法：根据字段类型设置实体字段值
        void setEntityFieldValue(std::shared_ptr<ScriptInstance> instance,
                                 const std::string &name,
                                 const ScriptFieldInstance &fieldInstance);

        bool m_initialized = false;
        MonoDomain *m_rootDomain = nullptr;
        std::vector<std::string> m_allEntityClassesNames;
        std::unordered_map<std::string, std::shared_ptr<CSharpScriptClass>> m_classes;
        std::unordered_map<UUID, std::shared_ptr<ScriptInstance>> m_entityInstances;
        std::unordered_map<UUID, ScriptFieldMap> m_entityScriptFields;
    };
}
