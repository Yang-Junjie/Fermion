#include "fmpch.hpp"

#include "Script/ScriptEngine.hpp"
#include "CSharpScriptEngine.hpp"

namespace Fermion
{
    bool CSharpScriptEngine::init()
    {
        if (m_initialized)
            return false;

        const auto assembliesPath = std::filesystem::current_path();
        mono_set_assemblies_path(assembliesPath.string().c_str());
        mono_config_parse(nullptr);

        m_rootDomain = mono_jit_init_version("FermionRootDomain", "v4.0.30319");
        if (!m_rootDomain)
            return false;

        m_appDomain = mono_domain_create_appdomain(const_cast<char *>("FermionScriptDomain"), nullptr);
        if (!m_appDomain)
        {
            mono_jit_cleanup(m_rootDomain);
            m_rootDomain = nullptr;
            return false;
        }

        mono_domain_set(m_appDomain, true);

        m_initialized = true;
        return true;
    }

    void CSharpScriptEngine::shutdown()
    {
        if (!m_initialized)
            return;

        if (m_appDomain)
        {
            mono_domain_set(m_rootDomain, false);
            mono_domain_unload(m_appDomain);
            m_appDomain = nullptr;
        }

        if (m_rootDomain)
        {
            mono_jit_cleanup(m_rootDomain);
            m_rootDomain = nullptr;
        }

        m_methods.clear();
        m_initialized = false;
    }

    bool CSharpScriptEngine::loadScript(const std::filesystem::path &path)
    {
        if (!m_initialized)
            return false;

        MonoAssembly *assembly = mono_domain_assembly_open(m_appDomain, path.string().c_str());
        if (!assembly)
            return false;

        const std::string assemblyName = path.stem().string();
        m_assemblies[assemblyName] = assembly;

        MonoImage *image = mono_assembly_get_image(assembly);
        if (!image)
            return false;

        
        MonoClass *klass = mono_class_from_name(image, "", assemblyName.c_str());
        if (!klass)
            return false;

        m_classes[assemblyName] = klass;

        m_methods.clear();

        return true;
    }

    void CSharpScriptEngine::registerFunction(const std::string &name, Func function)
    {
        if (!m_initialized)
            return;

        mono_add_internal_call(name.c_str(), (const void *)function);
        m_functions[name] = function;
    }

 
    void CSharpScriptEngine::invokeFunction(const std::string &name)
    {
        if (!m_initialized)
            return;

        MonoMethod *method = nullptr;

        auto it = m_methods.find(name);
        if (it != m_methods.end())
        {
            method = it->second;
        }
        else
        {
          
            for (auto &[_, klass] : m_classes)
            {
                method = mono_class_get_method_from_name(klass, name.c_str(), 0);
                if (method)
                {
                    m_methods[name] = method;
                    break;
                }
            }
        }

        if (method)
        {
            mono_runtime_invoke(method, nullptr, nullptr, nullptr);
        }
    }

}

