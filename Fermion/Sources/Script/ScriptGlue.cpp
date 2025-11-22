#include "ScriptGlue.hpp"
#include "fmpch.hpp"
#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

namespace Fermion
{
    // C++ 实现的内部调用函数示例
    extern "C" void PrintFromCpp()
    {
        Log::Warn("C++ InternalCall called!");
    }

    // 注册组件
    void ScriptGlue::registerComponents()
    {
        // TODO: 注册可在 C# 使用的组件，比如 Transform、RigidBody 等
        // 这里将来会包含遍历实体组件并注册到 Mono 运行时的逻辑
    }

    // 注册内部函数
    void ScriptGlue::registerFunctions()
    {
        // 注册 InternalCall
        // 名字必须完全匹配 C# 的 namespace + 类名 + 方法名
        // C# 定义: namespace Fermion, class TestScript, method PrintFromCpp
        mono_add_internal_call("Sandbox.TestScript::PrintFromCpp", (const void *)&PrintFromCpp);

        
    }
}