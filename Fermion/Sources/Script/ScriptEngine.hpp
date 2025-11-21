#pragma once

#include <filesystem>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <functional>

namespace Fermion
{
    using Func = void();
    class IScriptEngine
    {
    public:
        virtual ~IScriptEngine() = default;
        virtual bool init() = 0;
        virtual void shutdown() = 0;

        // 加载脚本
        virtual bool loadScript(const std::filesystem::path &path) = 0;

        // 注册函数给脚本 
        virtual void registerFunction(const std::string &name, Func function) = 0;

        // 调用脚本中的函数
        virtual void invokeFunction(const std::string &name) = 0;

    };
}
