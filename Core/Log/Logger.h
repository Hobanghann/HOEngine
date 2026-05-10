#pragma once

#include <chrono>
#include <string>

#ifndef SPDLOG_LEVEL_NAMES
#define SPDLOG_LEVEL_NAMES {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"}
#endif

#include "LogLevel.h"
#include "LogSinkImgui.h"
#include "Macros.h"

namespace spdlog
{
class Logger;
}

namespace ho
{
class Logger final
{
  public:
    static void Init();
    static void Shutdown();
    static void Log(eLogLevel level, const char* file, int32_t line, const char* fmt, ...);

    static LogSinkImgui* GetImGuiSink();

    static constexpr const char* GetFileName(const char* path)
    {
        const char* file = path;
        while (*path)
        {
            if (*path == '/' || *path == '\\')
            {
                file = path + 1;
            }
            ++path;
        }
        return file;
    }

    struct TraceScope
    {
        std::string NameStr;
        const char* File;
        int32_t Line;
        std::chrono::steady_clock::time_point StartTime;

        TraceScope(const std::string& scopeNameStr, const char* file, int32_t line);
        ~TraceScope();
    };
};
} // namespace ho

#define HO_LOG_TRACE(...)                                                                                              \
    ::ho::Logger::Log(::eLogLevel::Trace, ::ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)
#define HO_LOG_DEBUG(...)                                                                                              \
    ::ho::Logger::Log(::eLogLevel::Debug, ::ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)
#define HO_LOG_INFO(...)                                                                                               \
    ::ho::Logger::Log(::eLogLevel::Info, ::ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)
#define HO_LOG_WARN(...)                                                                                               \
    ::ho::Logger::Log(::eLogLevel::Warn, ::ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)
#define HO_LOG_ERROR(...)                                                                                              \
    ::ho::Logger::Log(::eLogLevel::Error, ::ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)
#define HO_LOG_CRITICAL(...)                                                                                           \
    ::ho::Logger::Log(::eLogLevel::Critical, ::ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)

#define HO_LOG_TRACE_SCOPE(name)                                                                                       \
    ::ho::Logger::TraceScope trace_##__LINE__(name, ::ho::Logger::GetFileName(__FILE__), __LINE__)