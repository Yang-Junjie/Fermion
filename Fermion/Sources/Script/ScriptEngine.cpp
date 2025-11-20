#include "Script/ScriptEngine.hpp"

#include "fmpch.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/mono-config.h>

namespace Fermion
{
    namespace
    {
        bool s_initialized = false;
        MonoDomain *s_rootDomain = nullptr;
        MonoDomain *s_appDomain = nullptr;

        std::filesystem::path getAssembliesPath()
        {
            // 目前直接使用工作目录（bin），后续可从 Project 配置中获取
            return std::filesystem::current_path();
        }
    }

    void ScriptEngine::init()
    {
        if (s_initialized)
            return;

        FM_PROFILE_FUNCTION();

        const auto assembliesPath = getAssembliesPath();
        mono_set_assemblies_path(assembliesPath.string().c_str());
        mono_config_parse(nullptr);

        s_rootDomain = mono_jit_init_version("FermionRootDomain", "v4.0.30319");
        if (!s_rootDomain)
        {
            Log::Error("ScriptEngine: Failed to initialize Mono JIT");
            return;
        }

        // 独立脚本域，便于后续热重载/卸载
        s_appDomain = mono_domain_create_appdomain(const_cast<char *>("FermionScriptDomain"), nullptr);
        if (!s_appDomain)
        {
            Log::Error("ScriptEngine: Failed to create script app domain");
            mono_jit_cleanup(s_rootDomain);
            s_rootDomain = nullptr;
            return;
        }

        mono_domain_set(s_appDomain, true);

        s_initialized = true;
        Log::Info("ScriptEngine initialized (Mono runtime)");
    }

    void ScriptEngine::shutdown()
    {
        if (!s_initialized)
            return;

        FM_PROFILE_FUNCTION();

        if (s_appDomain)
        {
            mono_domain_set(s_rootDomain, false);
            mono_domain_unload(s_appDomain);
            s_appDomain = nullptr;
        }

        if (s_rootDomain)
        {
            mono_jit_cleanup(s_rootDomain);
            s_rootDomain = nullptr;
        }

        s_initialized = false;
        Log::Info("ScriptEngine shutdown");
    }
}

