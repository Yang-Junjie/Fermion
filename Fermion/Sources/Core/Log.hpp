#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Fermion
{
    enum class LogLevel
    {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Critical,
        Off
    };

    class Log
    {
    public:
        static void Init(const std::string &logFile = "engine.log", LogLevel level = LogLevel::Info);
        static void SetLevel(LogLevel level);

        static void Trace(const std::string &msg);
        static void Debug(const std::string &msg);
        static void Info(const std::string &msg);
        static void Warn(const std::string &msg);
        static void Error(const std::string &msg);
        static void Critical(const std::string &msg);

    private:
        Log() = default;
        static std::shared_ptr<spdlog::logger> s_Logger;
        static std::vector<spdlog::sink_ptr> s_Sinks;
    };
}
