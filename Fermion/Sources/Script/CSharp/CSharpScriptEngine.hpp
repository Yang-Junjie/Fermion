#pragma once
#include "Script/ScriptEngine.hpp"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <unordered_map>
#include <memory>
#include <string>

namespace Fermion {
// C# 脚本类实现
class CSharpScriptClass : public ScriptClass {
public:
    CSharpScriptClass(MonoDomain *domain, MonoClass *klass, const std::string &ns, const std::string &name);

    virtual ScriptHandle instantiate() override;
    virtual ScriptHandle getMethod(const std::string &name, int parameterCount = 0) override;
    virtual void invokeMethod(const ScriptHandle &instance, const ScriptHandle &method, void **params = nullptr) override;

    virtual bool getFieldValue(const ScriptHandle &instance, const std::string &name, void *buffer) override;
    virtual bool setFieldValue(const ScriptHandle &instance, const std::string &name, const void *value) override;

private:
    MonoDomain *m_domain = nullptr;
    MonoClass *m_class = nullptr;
};
using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

class CSharpScriptEngine : public IScriptEngine {
public:
    virtual bool init() override;
    virtual void shutdown() override;
    virtual bool loadScript(const std::filesystem::path &path) override;

    virtual ScriptHandle createInstance(const std::string &className) override;
    virtual void destroyInstance(ScriptHandle instance) override;

    virtual void invokeMethod(ScriptHandle instance, const std::string &name) override;

    virtual bool getFieldValue(const ScriptHandle &instance, const std::string &name, void *buffer) override;
    virtual bool setFieldValue(const ScriptHandle &instance, const std::string &name, const void *value) override;

    virtual void onRuntimeStart(Scene *scene) override;
    virtual void onRuntimeStop() override;

    virtual Scene *getSceneContext() const override;

    virtual ScriptHandle getManagedInstance(UUID uuid, std::string className) override;

    virtual const std::vector<std::string> &getALLEntityClasses() const override;
    virtual bool entityClassExists(const std::string &fullClassName) override;
    bool isEntityClass(MonoClass *klass);

    virtual void onCreateEntity(Entity entity) override;
    virtual void onUpdateEntity(Entity entity, Timestep ts) override;

    MonoImage *getCoreImage() const {
        return m_coreImage;
    }
    MonoImage *getAppImage() const {
        return m_appImage;
    }

private:
    void setEntityFieldValue(std::shared_ptr<ScriptInstance> instance,
                             const std::string &name,
                             const ScriptFieldInstance &fieldInstance);
    void loadClasses(MonoImage *image);
    bool m_initialized = false;

    MonoDomain *m_rootDomain = nullptr;
    MonoDomain *m_appDomain = nullptr;

    MonoImage *m_coreImage = nullptr;
    MonoImage *m_appImage = nullptr;

    MonoAssembly *m_coreAssembly = nullptr;
    MonoAssembly *m_appAssembly = nullptr;

    std::vector<std::string> m_allEntityClassesNames;
    std::unordered_map<std::string, std::shared_ptr<CSharpScriptClass>> m_classes;

    std::unordered_map<UUID, std::unordered_map<std::string, std::shared_ptr<ScriptInstance>>> m_entityInstances;
    std::unordered_map<UUID, ScriptFieldMap> m_entityScriptFields;
};
} // namespace Fermion
