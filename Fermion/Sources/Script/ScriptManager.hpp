#pragma once

#include "ScriptEngine.hpp"
#include "CSharp/CSharpScriptEngine.hpp"

namespace Fermion {
class ScriptManager {
public:
    static bool init() {
        s_scriptEngine = std::make_shared<CSharpScriptEngine>();
        return s_scriptEngine->init();
    }

    static void shutdown() {
        s_scriptEngine->shutdown();
    }

    static bool loadScript(const std::filesystem::path &path) {
        return s_scriptEngine->loadScript(path);
    }

    static std::shared_ptr<IScriptEngine> get() {
        return s_scriptEngine;
    }

    static void onRuntimeStart(Scene *scene) {
        s_scriptEngine->onRuntimeStart(scene);
    }
    static ScriptHandle getManagedInstance(UUID uuid, std::string className) {
        return s_scriptEngine->getManagedInstance(uuid, className);
    }
    static void onRuntimeStop() {
        s_scriptEngine->onRuntimeStop();
    }
    static bool entityClassExists(const std::string &fullClassName) {
        return s_scriptEngine->entityClassExists(fullClassName);
    }
    static void onCreateEntity(Entity entity) {
        s_scriptEngine->onCreateEntity(entity);
    }
    static void onUpdateEntity(Entity entity, Timestep ts) {
        s_scriptEngine->onUpdateEntity(entity, ts);
    }
    static const std::vector<std::string> &getALLEntityClasses() {
        return s_scriptEngine->getALLEntityClasses();
    }

    static std::shared_ptr<ScriptClass> getScriptClass(const std::string &fullClassName) {
        return s_scriptEngine->getScriptClass(fullClassName);
    }

    static std::shared_ptr<ScriptInstance> getEntityScriptInstance(UUID uuid, const std::string &className) {
        return s_scriptEngine->getEntityScriptInstance(uuid, className);
    }

    static Scene *getSceneContext() {
        return s_scriptEngine->getSceneContext();
    }

    static MonoImage *getCoreImage() {
        auto *csharpEngine = dynamic_cast<CSharpScriptEngine *>(s_scriptEngine.get());
        return csharpEngine ? csharpEngine->getCoreImage() : nullptr;
    }
    static MonoImage *getAppImage() {
        auto *csharpEngine = dynamic_cast<CSharpScriptEngine *>(s_scriptEngine.get());
        return csharpEngine ? csharpEngine->getAppImage() : nullptr;
    }
    // void setSceneRenderer(const std::shared_ptr<SceneRenderer>& renderer)
    // {
    //     s_scriptEngine->setSceneRenderer(renderer);
    // }
    // std::shared_ptr<SceneRenderer> getSceneRenderer() const
    // {
    //     return s_scriptEngine->getSceneRenderer();
    // }

private:
    inline static std::shared_ptr<IScriptEngine> s_scriptEngine;
};
} // namespace Fermion
