#include "Core/Log.hpp"
#include <iostream>

namespace Fermion
{
    std::shared_ptr<spdlog::logger> Log::s_Logger = nullptr;
    std::vector<spdlog::sink_ptr> Log::s_Sinks;

    void Log::Init(const std::string &logFile, LogLevel level)
    {
        try
        {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true);

            s_Sinks = {console_sink, file_sink};
            s_Logger = std::make_shared<spdlog::logger>("EngineLogger", s_Sinks.begin(), s_Sinks.end());

            s_Logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

            // 调用 SetLevel 来统一设置 logger 和 sink
            SetLevel(level);

            // 错误及以上等级立即刷新
            s_Logger->flush_on(spdlog::level::err);

            s_Logger->info("Engine log initialized!");
        }
        catch (const spdlog::spdlog_ex &ex)
        {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
    }

    void Log::SetLevel(LogLevel level)
    {
        if (!s_Logger)
            return;

        spdlog::level::level_enum spdLevel;
        switch (level)
        {
        case LogLevel::Trace:
            spdLevel = spdlog::level::trace;
            break;
        case LogLevel::Debug:
            spdLevel = spdlog::level::debug;
            break;
        case LogLevel::Info:
            spdLevel = spdlog::level::info;
            break;
        case LogLevel::Warn:
            spdLevel = spdlog::level::warn;
            break;
        case LogLevel::Error:
            spdLevel = spdlog::level::err;
            break;
        case LogLevel::Critical:
            spdLevel = spdlog::level::critical;
            break;
        case LogLevel::Off:
            spdLevel = spdlog::level::off;
            break;
        default:
            spdLevel = spdlog::level::info;
            break;
        }

        // 同步 logger 等级
        s_Logger->set_level(spdLevel);

        // 同步所有 sink 等级
        for (auto &sink : s_Sinks)
            sink->set_level(spdLevel);
    }

    void Log::Trace(const std::string &msg)
    {
        if (s_Logger)
            s_Logger->trace(msg);
    }
    void Log::Debug(const std::string &msg)
    {
        if (s_Logger)
            s_Logger->debug(msg);
    }
    void Log::Info(const std::string &msg)
    {
        if (s_Logger)
            s_Logger->info(msg);
    }
    void Log::Warn(const std::string &msg)
    {
        if (s_Logger)
            s_Logger->warn(msg);
    }
    void Log::Error(const std::string &msg)
    {
        if (s_Logger)
            s_Logger->error(msg);
    }
    void Log::Critical(const std::string &msg)
    {
        if (s_Logger)
            s_Logger->critical(msg);
    }
}
