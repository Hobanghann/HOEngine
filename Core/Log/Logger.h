#pragma once

#include <chrono>
#include <string>

#ifndef SPDLOG_LEVEL_NAMES
#define SPDLOG_LEVEL_NAMES {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"}
#endif

#include "Config.h"
#include "LogDefs.h"
#include "Macros.h"

namespace spdlog
{
class Logger;
}

namespace ho
{
class LogSinkImgui;

class Logger final
{
  public:
    static void Init();
    static void Shutdown();
    static void Log(eLogLevel level, eLogCategory category, const char* file, int32_t line, const char* fmt, ...);

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
        eLogCategory Category;
        const char* File;
        int32_t Line;
        std::string NameStr;
        std::chrono::steady_clock::time_point StartTime;

        TraceScope(eLogCategory category, const char* file, int32_t line, const std::string& scopeNameStr);
        ~TraceScope();
    };
};
} // namespace ho

#if defined(LOG_ENABLED)
#define HO_LOG_TRACE(category, ...)                                                                                    \
    ho::Logger::Log(ho::eLogLevel::LogLevel_Trace, category, ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)
#define HO_LOG_DEBUG(category, ...)                                                                                    \
    ho::Logger::Log(ho::eLogLevel::LogLevel_Debug, category, ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)
#define HO_LOG_INFO(category, ...)                                                                                     \
    ho::Logger::Log(ho::eLogLevel::LogLevel_Info, category, ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)
#define HO_LOG_WARN(category, ...)                                                                                     \
    ho::Logger::Log(ho::eLogLevel::LogLevel_Warn, category, ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)
#define HO_LOG_ERROR(category, ...)                                                                                    \
    ho::Logger::Log(ho::eLogLevel::LogLevel_Error, category, ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)
#define HO_LOG_CRITICAL(category, ...)                                                                                 \
    ho::Logger::Log(                                                                                                   \
        ho::eLogLevel::LogLevel_Critical, category, ho::Logger::GetFileName(__FILE__), __LINE__, __VA_ARGS__)

#define HO_LOG_TRACE_SCOPE(category, name)                                                                             \
    ho::Logger::TraceScope trace_##__LINE__(category, ho::Logger::GetFileName(__FILE__), __LINE__, name)
#else
#define HO_LOG_TRACE(...) ((void)0)
#define HO_LOG_DEBUG(...) ((void)0)
#define HO_LOG_INFO(...) ((void)0)
#define HO_LOG_WARN(...) ((void)0)
#define HO_LOG_ERROR(...) ((void)0)
#define HO_LOG_CRITICAL(...) ((void)0)

#define HO_LOG_TRACE_SCOPE(name) ((void)0)
#endif