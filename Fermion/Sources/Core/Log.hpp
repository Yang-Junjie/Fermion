/*
    Log.hpp
    本文件定义了日志系统
*/
#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <format>

namespace Fermion {
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off
};

class Log {
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

#ifdef NDEBUG
#define FERMION_ASSERT(condition, message) ((void)0)
#else
#define FERMION_ASSERT(condition, message)                                                                               \
    do {                                                                                                                 \
        if (!(condition)) {                                                                                              \
            Fermion::Log::Error(std::format("Assertion failed: {}, file {}, line {}", message, __FILE__, __LINE__));     \
            std::cerr << "Assertion failed: " << message << ", file " << __FILE__ << ", line " << __LINE__ << std::endl; \
            assert(false);                                                                                               \
        }                                                                                                                \
    } while (0)
#endif

class FMAssert {
public:
    static void Assert(bool condition, const std::string &message, const char *file, int line) {
        if (!condition) {
            Log::Error("Assertion failed: " + message + ", file " + file + ", line" + std::to_string(line));
            assert(false);
        }
    }
};
} // namespace Fermion
