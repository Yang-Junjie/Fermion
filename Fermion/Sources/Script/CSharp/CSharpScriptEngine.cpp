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

                // 简单的类型映射
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
        // 创建 Mono 对象
        MonoObject *obj = mono_object_new(m_domain, m_class);
        // 调用默认构造函数
        mono_runtime_object_init(obj);

        ScriptHandle handle;
        handle.m_instance = obj;
        handle.type = ScriptHandleType::Object;
        return handle;
    }

    ScriptHandle CSharpScriptClass::getMethod(const std::string &name, int parameterCount)
    {
        // 获取 Mono 方法
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

        // 调用 Mono 方法
        MonoObject *exception = nullptr;
        mono_runtime_invoke(m, obj, params, &exception);

        if (exception)
        {
            Log::Error("CSharpScriptClass::invokeMethod: script exception!");
            // TODO: 打印异常堆栈
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

        const auto &fields = getFields();
        auto it = fields.find(name);
        if (it == fields.end())
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

        // 设置 Mono 程序集路径为当前目录
        mono_set_assemblies_path(std::filesystem::current_path().string().c_str());
        mono_config_parse(nullptr);

        // 初始化 Mono JIT
        m_rootDomain = mono_jit_init_version("FermionRootDomain", "v4.0.30319");
        if (!m_rootDomain)
        {
            Log::Error("CSharpScriptEngine: Mono JIT init failed!");
            return false;
        }

        m_initialized = true;

        // 注册内部调用
        ScriptGlue::registerFunctions();

        Log::Info("CSharpScriptEngine: initialized");
        return true;
    }

    void CSharpScriptEngine::shutdown()
    {
        if (!m_initialized)
            return;

        // 清理 Mono JIT
        mono_jit_cleanup(m_rootDomain);
        m_rootDomain = nullptr;
        m_initialized = false;

        Log::Info("CSharpScriptEngine: shutdown");
    }

    bool CSharpScriptEngine::loadScript(const std::filesystem::path &path)
    {
        // 加载程序集
        MonoAssembly *assembly = mono_domain_assembly_open(m_rootDomain, path.string().c_str());
        if (!assembly)
        {
            Log::Error("CSharpScriptEngine: load script fail " + path.string());
            return false;
        }

        MonoImage *image = mono_assembly_get_image(assembly);
        if (!image)
            return false;

        // 遍历程序集中的所有类型
        int typeCount = mono_image_get_table_rows(image, MONO_TABLE_TYPEDEF);
        Log::Info(std::format("CSharpScriptEngine: load {}, total {} type", path.string(), typeCount));

        for (int i = 1; i <= typeCount; ++i)
        {
            uint32_t typeToken = mono_metadata_make_token(MONO_TABLE_TYPEDEF, i);
            MonoClass *klass = mono_class_get(image, typeToken);
            if (!klass)
                continue;

            const char *name = mono_class_get_name(klass);
            const char *namesp = mono_class_get_namespace(klass);

            if (!name)
                continue;

            std::string fullName;
            if (namesp && namesp[0] != '\0')
                fullName = std::string(namesp) + "." + name;
            else
                fullName = name;

            Log::Info("  -> load script class : " + fullName);

            // 存储脚本类信息
            m_classes[fullName] = std::make_shared<CSharpScriptClass>(m_rootDomain, klass, namesp ? namesp : "", name);
        }

        Log::Info(std::format("CSharpScriptEngine:  {} script class  was loaded ", m_classes.size()));
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
    void CSharpScriptEngine::onRuntimeStop()
    {
        m_scene = nullptr;
        m_entityInstances.clear();
    }
    bool CSharpScriptEngine::entityClassExists(const std::string &fullClassName)
    {
        return m_classes.find(fullClassName) != m_classes.end();
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
        default:
            Log::Warn(std::format("CSharpScriptEngine: Unsupported field type '{}' for field '{}'",
                                  static_cast<int>(fieldInstance.field.type), name));
            break;
        }
    }
    void CSharpScriptEngine::onCreateEntity(Entity entity)
    {
        const auto &sc = entity.getComponent<ScriptComponent>();
        Log::Info(std::format("CSharpScriptEngine: try to create entity: {}", sc.className));

        if (entityClassExists(sc.className))
        {
            UUID entityID = entity.getUUID();
            Log::Info(std::format("  -> find script class  {}, crate entity (EntityID: {})", sc.className, entityID.toString()));

            std::shared_ptr<ScriptInstance> instance = std::make_shared<ScriptInstance>(m_classes[sc.className], entity);

            m_entityInstances[entityID] = instance;

            if (m_entityScriptFields.find(entityID) != m_entityScriptFields.end())
            {
                const ScriptFieldMap &fieldMap = m_entityScriptFields.at(entityID);
                for (const auto &[name, fieldInstance] : fieldMap)
                {
                    setEntityFieldValue(instance, name, fieldInstance);
                }
            }

            Log::Info("  -> call  OnCreate()");
            instance->invokeOnCreate();
            Log::Info("  -> OnCreate() call done");
        }
        else
        {
            Log::Error(std::format("CSharpScriptEngine: cant find script class : {}", sc.className));
        }
    }
    void CSharpScriptEngine::onUpdateEntity(Entity entity, Timestep ts)
    {
        UUID entityID = entity.getUUID();
        if (m_entityInstances.find(entityID) != m_entityInstances.end())
        {
            std::shared_ptr<ScriptInstance> instance = m_entityInstances[entityID];
            instance->invokeOnUpdate(ts);
        }
        else
        {
            Log::Warn(std::format("CSharpScriptEngine: entity {} hast script entity", entityID.toString()));
        }
    }
}
