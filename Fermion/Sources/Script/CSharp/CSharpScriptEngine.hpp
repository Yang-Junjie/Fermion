#include "Script/ScriptEngine.hpp"
#include "fmpch.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
namespace Fermion
{
    class CSharpScriptEngine : public IScriptEngine
    {
    public:
        virtual ~CSharpScriptEngine() = default;
        virtual bool init() override;
        virtual void shutdown() override;

        // 加载脚本
        virtual bool loadScript(const std::filesystem::path &path) override;

        // 注册函数给脚本 name是C#中的namespace.函数名
        virtual void registerFunction(const std::string &name, Func function) override;

        // 调用脚本中的函数
        virtual void invokeFunction(const std::string &name) override;

    private:
        bool m_initialized = false;
        MonoDomain *m_rootDomain = nullptr;
        MonoDomain *m_appDomain = nullptr;
        std::unordered_map<std::string, MonoAssembly *> m_assemblies;
        std::unordered_map<std::string, MonoMethod *> m_methods;
        std::unordered_map<std::string, MonoClass *> m_classes;
        std::unordered_map<std::string, Func *> m_functions;
    };
}