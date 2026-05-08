#pragma once

#ifndef SPDLOG_LEVEL_NAMES
#define SPDLOG_LEVEL_NAMES {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"}
#endif

#include <mutex>
#include <queue>
#include <spdlog/sinks/base_sink.h>
#include <string>

#include "Core/Macros.h"

namespace ho
{
struct LogEntry
{
    std::string Message;
    eLogLevel Level;
};

class LogSink_imgui final : public spdlog::sinks::base_sink<std::mutex>
{
  public:
    bool IsEmpty()
    {
        std::lock_guard<std::mutex> lock(this->mutex_);
        return mLogQueue.empty();
    }

    void PopLog(LogEntry* outEntry)
    {
        HO_ASSERT(outEntry, "outEntry is null.");
        std::lock_guard<std::mutex> lock(this->mutex_);
        if (mLogQueue.empty())
        {
            return;
        }

        outEntry->Message = std::move(mLogQueue.front().Message);
        outEntry->Level = mLogQueue.front().Level;
        mLogQueue.pop();
    }

  protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        this->formatter_->format(msg, formatted);

        mLogQueue.push({fmt::to_string(formatted), static_cast<eLogLevel>(msg.level)});
    }

    void flush_() override {}

  private:
    std::queue<LogEntry> mLogQueue;
};

} // namespace ho