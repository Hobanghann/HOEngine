#include "Logger.h"

#include <cstdarg>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace ho
{
static std::shared_ptr<spdlog::logger> gLogger;
static std::shared_ptr<LogSink_imgui> gSink_imgui;

void Logger::Init()
{
    spdlog::init_thread_pool(8192, 1);

    gSink_imgui = std::make_shared<LogSink_imgui>();

    gLogger = std::make_shared<spdlog::async_logger>("HOEngine", gSink_imgui, spdlog::thread_pool());

    gLogger->set_level(spdlog::level::trace);
    spdlog::register_logger(gLogger);
    gLogger->set_pattern("[%T] [%^%l%$] %v");
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

void Logger::Log(eLogLevel level, const char* file, int32_t line, const char* fmt, ...)
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

    char finalMsgBuf[1152];
    snprintf(finalMsgBuf, sizeof(finalMsgBuf), "[%s:%d] %s", file, line, msgBuf);

    switch (level)
    {
        case eLogLevel::Trace:
            gLogger->trace(finalMsgBuf);
            break;
        case eLogLevel::Debug:
            gLogger->debug(finalMsgBuf);
            break;
        case eLogLevel::Info:
            gLogger->info(finalMsgBuf);
            break;
        case eLogLevel::Warn:
            gLogger->warn(finalMsgBuf);
            break;
        case eLogLevel::Error:
            gLogger->error(finalMsgBuf);
            break;
        case eLogLevel::Critical:
            gLogger->critical(finalMsgBuf);
            break;
    }
}

LogSink_imgui* Logger::GetImGuiSink()
{
    return gSink_imgui.get();
}

Logger::TraceScope::TraceScope(const std::string& scopeNameStr, const char* file, int32_t line)
  : NameStr(scopeNameStr)
  , File(file)
  , Line(line)
  , StartTime(std::chrono::steady_clock::now())
{
    Logger::Log(eLogLevel::Trace, file, line, "[START] %s", NameStr.c_str());
}

Logger::TraceScope::~TraceScope()
{
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - StartTime).count();
    Logger::Log(eLogLevel::Trace, File, Line, "[END] %s (Took %lld ms)", NameStr.c_str(), duration);
}
} // namespace ho