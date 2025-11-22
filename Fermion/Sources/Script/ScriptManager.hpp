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

    private:
        inline static std::shared_ptr<IScriptEngine> s_scriptEngine;
    };
}
