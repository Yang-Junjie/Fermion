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

        static void registerFunction(const std::string &name, Func func)
        {
            s_scriptEngine->registerFunction(name, func);
        }

        static void invokeFunction(const std::string &name)
        {
            s_scriptEngine->invokeFunction(name);
        }

    private:
        inline static std::shared_ptr<IScriptEngine> s_scriptEngine;
    };
}
