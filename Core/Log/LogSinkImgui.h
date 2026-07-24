#pragma once

#ifndef SPDLOG_LEVEL_NAMES
#define SPDLOG_LEVEL_NAMES {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"}
#endif

#include <mutex>
#include <queue>
#include <spdlog/sinks/base_sink.h>
#include <string>

#include "LogDefs.h"
#include "Macros.h"

namespace ho
{
struct LogEntry
{
    char Time[16] = {};
    eLogLevel Level;
    eLogCategory Category;
    char SourceLocation[128] = {};
    std::string Message;
};

class LogSinkImgui final : public spdlog::sinks::base_sink<std::mutex>
{
  public:
    bool IsEmpty()
    {
        const std::lock_guard<std::mutex> lock(this->mutex_);
        return mLogQueue.empty();
    }

    void PopLog(LogEntry* outEntry)
    {
        HO_ASSERT(outEntry, "outEntry is null.");
        const std::lock_guard<std::mutex> lock(this->mutex_);
        if (mLogQueue.empty())
        {
            return;
        }

        *outEntry = std::move(mLogQueue.front());
        mLogQueue.pop();
    }

  protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        LogEntry entry;

        auto timet = std::chrono::system_clock::to_time_t(msg.time);
        std::tm tmBuf;
#if defined(_WIN32)
        localtime_s(&tmBuf, &timet);
#else
        localtime_r(&timet, &tmBuf);
#endif
        std::strftime(entry.Time, sizeof(entry.Time), "%H:%M:%S", &tmBuf);

        switch (msg.level)
        {
            case spdlog::level::trace:
                entry.Level = eLogLevel::LogLevel_Trace;
                break;
            case spdlog::level::debug:
                entry.Level = eLogLevel::LogLevel_Debug;
                break;
            case spdlog::level::info:
                entry.Level = eLogLevel::LogLevel_Info;
                break;
            case spdlog::level::warn:
                entry.Level = eLogLevel::LogLevel_Warn;
                break;
            case spdlog::level::err:
                entry.Level = eLogLevel::LogLevel_Error;
                break;
            case spdlog::level::critical:
                entry.Level = eLogLevel::LogLevel_Critical;
                break;
            default:
                entry.Level = eLogLevel::LogLevel_Trace;
                break;
        }

        snprintf(entry.SourceLocation, sizeof(entry.SourceLocation), "%s:%d", msg.source.filename, msg.source.line);

        std::string_view payloadStr(msg.payload.data(), msg.payload.size());

        entry.Category = LogCategory_None;
        size_t spacePos = payloadStr.find(' ');

        if (spacePos != std::string_view::npos)
        {
            std::string categoryStr(payloadStr.substr(0, spacePos));
            entry.Category = static_cast<eLogCategory>(std::stoi(categoryStr));

            entry.Message = payloadStr.substr(spacePos + 1);
        }
        else
        {
            entry.Message = payloadStr;
        }

        mLogQueue.push(std::move(entry));
    }

    void flush_() override {}

  private:
    std::queue<LogEntry> mLogQueue;
};

} // namespace ho