#pragma once

namespace Fermion
{
    // 脚本胶水代码类，用于连接 C++ 引擎和脚本运行时
    class ScriptGlue
    {
    public:
        // 注册引擎组件到脚本运行时
        static void registerComponents();
        // 注册引擎内部函数到脚本运行时
        static void registerFunctions();
    };

}
