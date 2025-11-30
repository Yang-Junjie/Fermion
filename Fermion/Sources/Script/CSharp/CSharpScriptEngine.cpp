#include "CSharpScriptEngine.hpp"
#include "Script/ScriptGlue.hpp"
#include "fmpch.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/class.h>
#include <mono/metadata/attrdefs.h>

namespace Fermion
{
    // ====================================================================================================
    // CSharpScriptClass 实现
    // ====================================================================================================

    CSharpScriptClass::CSharpScriptClass(MonoDomain *domain, MonoClass *klass, const std::string &ns, const std::string &name)
        : ScriptClass(ns, name), m_domain(domain), m_class(klass)
    {
        // 缓存所有公共字段
        void *iter = nullptr;
        while (MonoClassField *field = mono_class_get_fields(m_class, &iter))
        {
            uint32_t flags = mono_field_get_flags(field);
            if (flags & MONO_FIELD_ATTR_PUBLIC)
            {
                const char *fieldName = mono_field_get_name(field);
                ScriptFieldType fieldType = ScriptFieldType::None;
                MonoType *type = mono_field_get_type(field);

                int typeEnum = mono_type_get_type(type);
                switch (typeEnum)
                {
                case MONO_TYPE_R4:
                    fieldType = ScriptFieldType::Float;
                    break;
                case MONO_TYPE_R8:
                    fieldType = ScriptFieldType::Double;
                    break;
                case MONO_TYPE_BOOLEAN:
                    fieldType = ScriptFieldType::Bool;
                    break;
                case MONO_TYPE_I4:
                    fieldType = ScriptFieldType::Int;
                    break;
                case MONO_TYPE_U8:
                    fieldType = ScriptFieldType::ULong;
                    break;
                }

                if (fieldType != ScriptFieldType::None)
                {
                    m_fields[fieldName] = {fieldName, fieldType};
                }
            }
        }
    }

    ScriptHandle CSharpScriptClass::instantiate()
    {
        MonoObject *obj = mono_object_new(m_domain, m_class);
        mono_runtime_object_init(obj);

        ScriptHandle handle;
        handle.m_instance = obj;
        handle.type = ScriptHandleType::Object;
        return handle;
    }

    ScriptHandle CSharpScriptClass::getMethod(const std::string &name, int parameterCount)
    {
        MonoMethod *method = mono_class_get_method_from_name(m_class, name.c_str(), parameterCount);
        ScriptHandle handle;
        handle.m_instance = method;
        handle.type = ScriptHandleType::Method;
        return handle;
    }

    void CSharpScriptClass::invokeMethod(const ScriptHandle &instance, const ScriptHandle &method, void **params)
    {
        if (!instance.isValid() || !method.isValid())
        {
            Log::Error("CSharpScriptClass::invokeMethod: invalid handle!");
            return;
        }

        MonoObject *obj = static_cast<MonoObject *>(instance.m_instance);
        MonoMethod *m = static_cast<MonoMethod *>(method.m_instance);

        MonoObject *exception = nullptr;
        mono_runtime_invoke(m, obj, params, &exception);

        if (exception)
        {
            Log::Error("CSharpScriptClass::invokeMethod: script exception!");
        }
    }

    bool CSharpScriptClass::getFieldValue(const ScriptHandle &instance, const std::string &name, void *buffer)
    {
        if (!instance.isValid())
            return false;

        const auto &fields = getFields();
        auto it = fields.find(name);
        if (it == fields.end())
            return false;

        MonoObject *obj = static_cast<MonoObject *>(instance.m_instance);
        MonoClassField *field = mono_class_get_field_from_name(m_class, name.c_str());

        if (!field)
            return false;

        mono_field_get_value(obj, field, buffer);
        return true;
    }

    bool CSharpScriptClass::setFieldValue(const ScriptHandle &instance, const std::string &name, const void *value)
    {
        if (!instance.isValid())
            return false;

        MonoObject *obj = static_cast<MonoObject *>(instance.m_instance);
        MonoClassField *field = mono_class_get_field_from_name(m_class, name.c_str());

        if (!field)
            return false;

        mono_field_set_value(obj, field, (void *)value);
        return true;
    }

    // ====================================================================================================
    // CSharpScriptEngine 实现
    // ====================================================================================================

    bool CSharpScriptEngine::init()
    {
        if (m_initialized)
            return false;

        mono_set_assemblies_path(std::filesystem::current_path().string().c_str());
        mono_config_parse(nullptr);

        m_rootDomain = mono_jit_init_version("FermionRootDomain", "v4.0.30319");
        if (!m_rootDomain)
        {
            Log::Error("CSharpScriptEngine: Mono JIT init failed!");
            return false;
        }
        mono_domain_set(m_rootDomain, true);

        m_coreAssembly = mono_domain_assembly_open(m_rootDomain, "Photon.dll");
        m_coreImage = mono_assembly_get_image(m_coreAssembly);

        m_initialized = true;

        ScriptGlue::registerFunctions();

        Log::Info("CSharpScriptEngine: initialized");
        return true;
    }

    void CSharpScriptEngine::shutdown()
    {
        if (!m_initialized)
            return;

        if (m_appDomain)
            mono_domain_unload(m_appDomain);

        mono_jit_cleanup(m_rootDomain);

        m_rootDomain = nullptr;
        m_appDomain = nullptr;

        m_initialized = false;
        Log::Info("CSharpScriptEngine: shutdown");
    }

    void CSharpScriptEngine::loadClasses(MonoImage *image)
    {
        int typeCount = mono_image_get_table_rows(image, MONO_TABLE_TYPEDEF);
        Log::Info("Type count in DLL: " + std::to_string(typeCount));
        for (int i = 1; i <= typeCount; ++i)
        {
            uint32_t typeToken = mono_metadata_make_token(MONO_TABLE_TYPEDEF, i);
            MonoClass *klass = mono_class_get(image, typeToken);
            if (!klass)
                continue;

            if (!isEntityClass(klass))
                continue;

            const char *name = mono_class_get_name(klass);
            const char *namesp = mono_class_get_namespace(klass);
            std::string fullName = namesp && namesp[0] ? (std::string(namesp) + "." + name) : name;

            m_allEntityClassesNames.push_back(fullName);
            Log::Info("Load Script Class: " + fullName);

            m_classes[fullName] = std::make_shared<CSharpScriptClass>(
                m_appDomain,
                klass,
                namesp ? namesp : "",
                name);
        }
    }

    bool CSharpScriptEngine::loadScript(const std::filesystem::path &path)
    {
        m_appDomain = mono_domain_create_appdomain(const_cast<char *>("AppDomain"), nullptr);
        mono_domain_set(m_appDomain, true);

        mono_domain_assembly_open(m_appDomain, "Photon.dll");

        m_appAssembly = mono_domain_assembly_open(m_appDomain, path.string().c_str());
        if (!m_appAssembly)
            return false;
        m_appImage = mono_assembly_get_image(m_appAssembly);

        m_allEntityClassesNames.clear();
        m_classes.clear();

        loadClasses(m_coreImage);
        loadClasses(m_appImage);

        ScriptGlue::registerComponents();
        ScriptGlue::registerComponentFactories();

        Log::Info("Loading DLL: " + path.string());
        Log::Info(std::format("CSharpScriptEngine: {} script classes loaded", m_classes.size()));
        return true;
    }

    ScriptHandle CSharpScriptEngine::createInstance(const std::string &className)
    {
        auto it = m_classes.find(className);
        if (it == m_classes.end())
        {
            Log::Error("CSharpScriptEngine: cant find script class  " + className);
            return {};
        }

        return it->second->instantiate();
    }

    void CSharpScriptEngine::destroyInstance(ScriptHandle instance)
    {
        // Mono GC 会自动管理对象的生命周期，这里不需要手动释放
    }

    void CSharpScriptEngine::invokeMethod(ScriptHandle instance, const std::string &name)
    {
        if (!instance.isValid())
        {
            Log::Error("CSharpScriptEngine::invokeMethod: invalid handle!");
            return;
        }

        MonoObject *obj = static_cast<MonoObject *>(instance.m_instance);
        MonoClass *klass = mono_object_get_class(obj);
        MonoMethod *method = mono_class_get_method_from_name(klass, name.c_str(), 0);

        if (!method)
        {
            Log::Error("CSharpScriptEngine::invokeMethod: cant find method " + name);
            return;
        }

        MonoObject *exception = nullptr;
        mono_runtime_invoke(method, obj, nullptr, &exception);

        if (exception)
        {
            Log::Error("CSharpScriptEngine::invokeMethod: script exception!");
        }
    }

    bool CSharpScriptEngine::getFieldValue(const ScriptHandle &instance, const std::string &name, void *buffer)
    {
        if (!instance.isValid())
            return false;

        MonoObject *obj = static_cast<MonoObject *>(instance.m_instance);
        MonoClass *klass = mono_object_get_class(obj);
        MonoClassField *field = mono_class_get_field_from_name(klass, name.c_str());

        if (!field)
            return false;

        mono_field_get_value(obj, field, buffer);
        return true;
    }

    bool CSharpScriptEngine::setFieldValue(const ScriptHandle &instance, const std::string &name, const void *value)
    {
        if (!instance.isValid())
            return false;

        MonoObject *obj = static_cast<MonoObject *>(instance.m_instance);
        MonoClass *klass = mono_object_get_class(obj);
        MonoClassField *field = mono_class_get_field_from_name(klass, name.c_str());

        if (!field)
            return false;

        mono_field_set_value(obj, field, (void *)value);
        return true;
    }

    void CSharpScriptEngine::onRuntimeStart(Scene *scene)
    {
        Log::Info("CSharpScriptEngine: runtime start");
        m_scene = scene;
    }

    Scene *CSharpScriptEngine::getSceneContext() const
    {
        return m_scene;
    }

    ScriptHandle CSharpScriptEngine::getManagedInstance(UUID uuid, std::string className)
    {
        FERMION_ASSERT(m_entityInstances.find(uuid) != m_entityInstances.end(), "Entity not found!");
        return m_entityInstances[uuid][className]->getHandle();
    }

    void CSharpScriptEngine::onRuntimeStop()
    {
        m_scene = nullptr;
        m_entityInstances.clear();
        m_entityScriptFields.clear();
    }

    bool CSharpScriptEngine::entityClassExists(const std::string &fullClassName)
    {
        return m_classes.find(fullClassName) != m_classes.end();
    }

    bool CSharpScriptEngine::isEntityClass(MonoClass *klass)
    {
        MonoClass *entityClass = mono_class_from_name(m_coreImage, "Fermion", "Entity");
        if (!entityClass)
            return false;

        MonoClass *current = klass;
        while (current)
        {
            if (current == entityClass)
                return true;

            current = mono_class_get_parent(current);
        }

        return false;
    }

    void CSharpScriptEngine::setEntityFieldValue(std::shared_ptr<ScriptInstance> instance,
                                                 const std::string &name,
                                                 const ScriptFieldInstance &fieldInstance)
    {
        switch (fieldInstance.field.type)
        {
        case ScriptFieldType::Float:
            instance->setFieldValue(name, fieldInstance.getValue<float>());
            break;
        case ScriptFieldType::Double:
            instance->setFieldValue(name, fieldInstance.getValue<double>());
            break;
        case ScriptFieldType::Bool:
            instance->setFieldValue(name, fieldInstance.getValue<bool>());
            break;
        case ScriptFieldType::Int:
            instance->setFieldValue(name, fieldInstance.getValue<int>());
            break;
        case ScriptFieldType::ULong:
            instance->setFieldValue(name, fieldInstance.getValue<uint64_t>());
            break;
        default:
            Log::Warn(std::format("CSharpScriptEngine: Unsupported field type '{}' for field '{}'",
                                  static_cast<int>(fieldInstance.field.type), name));
            break;
        }
    }
    void CSharpScriptEngine::onCreateEntity(Entity entity)
    {
        const auto &sc = entity.getComponent<ScriptContainerComponent>();

        for (auto className : sc.scriptClassNames)
        {
            Log::Info(std::format("CSharpScriptEngine: try to create entity: {}", className));
            if (entityClassExists(className))
            {
                UUID entityID = entity.getUUID();
                Log::Info(std::format(" ScriptCreateEntity:  find script class  {}, crate entity (EntityID: {})", className, entityID.toString()));

                std::shared_ptr<ScriptInstance> instance = std::make_shared<ScriptInstance>(m_classes[className], entity);

                m_entityInstances[entityID][className] = instance;

                if (m_entityScriptFields.find(entityID) != m_entityScriptFields.end())
                {
                    const ScriptFieldMap &fieldMap = m_entityScriptFields.at(entityID);
                    for (const auto &[name, fieldInstance] : fieldMap)
                    {
                        setEntityFieldValue(instance, name, fieldInstance);
                    }
                }
                instance->invokeOnCreate();
            }
            else
            {
                Log::Error(std::format("CSharpScriptEngine: cant find script class : {}", className));
            }
        }
    }
    void CSharpScriptEngine::onUpdateEntity(Entity entity, Timestep ts)
    {
        UUID entityID = entity.getUUID();
        if (m_entityInstances.find(entityID) != m_entityInstances.end())
        {
            for (const auto &[className, instance] : m_entityInstances[entityID])
            {
                std::shared_ptr<ScriptInstance> instanceCp = instance;
                instanceCp->invokeOnUpdate(ts);
            }
        }
        else
        {
            Log::Warn(std::format("CSharpScriptEngine: entity {} hast script entity", entityID.toString()));
        }
    }
    const std::vector<std::string> &CSharpScriptEngine::getALLEntityClasses() const
    {
        return m_allEntityClassesNames;
    }

}
