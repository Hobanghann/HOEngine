#include "Logger.h"

#include <cstdarg>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "Core/Log/LogSinkImgui.h"

namespace ho
{
static std::shared_ptr<spdlog::logger> gLogger;
static std::shared_ptr<LogSinkImgui> gSink_imgui;

void Logger::Init()
{
    spdlog::init_thread_pool(8192, 1);

    gSink_imgui = std::make_shared<LogSinkImgui>();

    gLogger = std::make_shared<spdlog::async_logger>("HOEngineLogger", gSink_imgui, spdlog::thread_pool());

    gLogger->set_level(spdlog::level::trace);
    spdlog::register_logger(gLogger);
}

void Logger::Shutdown()
{
    if (gLogger)
    {
        gLogger->flush();
    }

    spdlog::shutdown();

    gLogger.reset();
}

void Logger::Log(eLogLevel level, eLogCategory category, const char* file, int32_t line, const char* fmt, ...)
{
    if (!gLogger)
    {
        return;
    }
    char msgBuf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msgBuf, sizeof(msgBuf), fmt, args);
    va_end(args);

    const spdlog::source_loc loc{file, line, nullptr};

    spdlog::level::level_enum spdLevel = spdlog::level::off;
    switch (level)
    {
        case eLogLevel::LogLevel_Trace:
            spdLevel = spdlog::level::trace;
            break;
        case eLogLevel::LogLevel_Debug:
            spdLevel = spdlog::level::debug;
            break;
        case eLogLevel::LogLevel_Info:
            spdLevel = spdlog::level::info;
            break;
        case eLogLevel::LogLevel_Warn:
            spdLevel = spdlog::level::warn;
            break;
        case eLogLevel::LogLevel_Error:
            spdLevel = spdlog::level::err;
            break;
        case eLogLevel::LogLevel_Critical:
            spdLevel = spdlog::level::critical;
            break;
    }

    char categoryMsgBuf[1200];
    snprintf(categoryMsgBuf, sizeof(categoryMsgBuf), "%d %s", category, msgBuf);

    gLogger->log(loc, spdLevel, categoryMsgBuf);
}

LogSinkImgui* Logger::GetImGuiSink()
{
    return gSink_imgui.get();
}

Logger::TraceScope::TraceScope(eLogCategory category, const char* file, int32_t line, const std::string& scopeNameStr)
  : Category(category)
  , File(file)
  , Line(line)
  , NameStr(scopeNameStr)
  , StartTime(std::chrono::steady_clock::now())
{
    Logger::Log(eLogLevel::LogLevel_Trace, category, file, line, "[START] %s", NameStr.c_str());
}

Logger::TraceScope::~TraceScope()
{
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - StartTime).count();
    Logger::Log(eLogLevel::LogLevel_Trace,
                Category,
                File,
                Line,
                "[END] %s (Took %f sec)",
                NameStr.c_str(),
                static_cast<double>(duration) / 1000.0);
}
} // namespace ho