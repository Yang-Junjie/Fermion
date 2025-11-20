
// #include "Script/ScriptEngine.hpp"

// extern "C"
// {
//     typedef struct _MonoClass MonoClass;
//     typedef struct _MonoObject MonoObject;
//     typedef struct _MonoMethod MonoMethod;
//     typedef struct _MonoAssembly MonoAssembly;
//     typedef struct _MonoImage MonoImage;
//     typedef struct _MonoClassField MonoClassField;
//     typedef struct _MonoString MonoString;
// }

// namespace Fermion
// {
//     struct CSScriptField : public ScriptField
//     {
//         ScriptFieldType type{};
//         std::string name;

//         MonoClassField *classField;
//     };

//     class CSScriptFieldInstance : public IScriptFieldInstance
//     {
//     public:
//         CSScriptField Field;
//         virtual ~CSScriptFieldInstance() = default;

//         virtual ScriptFieldType getType() override;
//         virtual const std::string &getName() override;

//         virtual void getValue(void *outBuffer) override;
//         virtual void setValue(const void *value) override;

//     private:
//         uint8_t m_buffer[16];

//         friend class ScriptEngine;
//         friend class ScriptInstance;
//     };
// }