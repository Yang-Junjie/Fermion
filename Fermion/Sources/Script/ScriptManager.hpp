#include "ScriptEngine.hpp"
#include "CSharp/CSharpScriptEngine.hpp"

namespace Fermion
{
    class ScriptManager
    {
    public:
        static bool init()
        {
            s_scriptEngine = std::make_shared<CSharpScriptEngine>();
            return s_scriptEngine->init();
        }

        static void shutdown()
        {
            s_scriptEngine->shutdown();
        }

        static bool loadScript(const std::filesystem::path &path)
        {
            return s_scriptEngine->loadScript(path);
        }

        static std::shared_ptr<IScriptEngine> get()
        {
            return s_scriptEngine;
        }

        static void onRuntimeStart(Scene *scene)
        {
            s_scriptEngine->onRuntimeStart(scene);
        }
        static void onRuntimeStop()
        {
            s_scriptEngine->onRuntimeStop();
        }
        static bool entityClassExists(const std::string &fullClassName)
        {
            return s_scriptEngine->entityClassExists(fullClassName);
        }
        static void onCreateEntity(Entity entity)
        {
            s_scriptEngine->onCreateEntity(entity);
        }
        static void onUpdateEntity(Entity entity, Timestep ts)
        {
            s_scriptEngine->onUpdateEntity(entity, ts);
        }
        static const std::vector<std::string> &getALLEntityClasses()  {
            return s_scriptEngine->getALLEntityClasses();
        }

        static Scene* getSceneContext(){
            return s_scriptEngine->getSceneContext();
        }

    private:
        inline static std::shared_ptr<IScriptEngine> s_scriptEngine;
    };
}
