#include "Script/ScriptEngine.hpp"

#include "fmpch.hpp"

#include "ScriptEngine.hpp"

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

        const auto assembliesPath = std::filesystem::current_path();
        mono_set_assemblies_path(assembliesPath.string().c_str());
        mono_config_parse(nullptr);

        s_rootDomain = mono_jit_init_version("FermionRootDomain", "v4.0.30319");
        if (!s_rootDomain)
            return;

        s_appDomain = mono_domain_create_appdomain(const_cast<char *>("FermionScriptDomain"), nullptr);
        if (!s_appDomain)
        {
            mono_jit_cleanup(s_rootDomain);
            s_rootDomain = nullptr;
            return;
        }

        mono_domain_set(s_appDomain, true);

        s_initialized = true;
    }

    bool ScriptEngine::registerMethod(const std::string &name, Func func)
    {
        if (!s_initialized)
            return false;

        // 注册 InternalCall
        mono_add_internal_call(name.c_str(), (const void *)func);
        s_registeredMethods.emplace_back(name, func);
        return true;
    }

    bool ScriptEngine::runScripts()
    {
        if (!s_initialized)
            return false;

        // 简单示例：加载 TestScript.dll 并调用 TestScript.OnCreate
        MonoAssembly *assembly = mono_domain_assembly_open(s_appDomain, "TestScript.dll");
        if (!assembly)
            return false;

        MonoImage *image = mono_assembly_get_image(assembly);
        MonoClass *klass = mono_class_from_name(image, "", "TestScript");
        MonoMethod *method = mono_class_get_method_from_name(klass, "OnCreate", 0);

        mono_runtime_invoke(method, nullptr, nullptr, nullptr);
        return true;
    }

    void ScriptEngine::shutdown()
    {
        if (!s_initialized)
            return;

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

        s_registeredMethods.clear();
        s_initialized = false;
    }

}
