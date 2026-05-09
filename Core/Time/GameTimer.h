#pragma once

#include <chrono>
#include <cstdint>

#include "Macros.h"

namespace ho
{
using ChronoClock = std::chrono::steady_clock;
using Duration = std::chrono::duration<int64_t, std::nano>;
using TimePoint = std::chrono::time_point<ChronoClock>;

class GameTimer final
{
  public:
    FORCE_INLINE GameTimer()
      : mDeltaTime(Duration(-1))
      , mPausedTime(Duration(0))
      , mBaseTime()
      , mCurrFrameTime()
      , mPrevFrameTime()
      , mStopTime()
      , mbStopped(false)
    {
    }

    FORCE_INLINE void Tick()
    {
        if (mbStopped)
        {
            mDeltaTime = Duration(0);
            return;
        }

        mCurrFrameTime = ChronoClock::now();

        mDeltaTime = mCurrFrameTime - mPrevFrameTime;
        mPrevFrameTime = mCurrFrameTime;

        if (mDeltaTime < Duration(0))
        {
            mDeltaTime = Duration(0);
        }
    }

    [[nodiscard]] FORCE_INLINE float GetDeltaTime()
    {
        return std::chrono::duration<float>(mDeltaTime).count();
    }

    [[nodiscard]] FORCE_INLINE float GetTotalTime()
    {
        if (mbStopped)
        {
            const Duration total = (mStopTime - mBaseTime) - mPausedTime;
            return std::chrono::duration<float>(total).count();
        }
        const TimePoint currTime = ChronoClock::now();
        const Duration total = (currTime - mBaseTime) - mPausedTime;

        return std::chrono::duration<float>(total).count();
    }

    FORCE_INLINE void Reset()
    {
        const TimePoint currTime = ChronoClock::now();

        mBaseTime = currTime;
        mPrevFrameTime = currTime;
        mStopTime = TimePoint();

        mPausedTime = Duration(0);
        mDeltaTime = Duration(0);

        mbStopped = false;
    }

    FORCE_INLINE void Stop()
    {
        if (!mbStopped)
        {
            mStopTime = ChronoClock::now();
            mbStopped = true;
        }
    }

    FORCE_INLINE void Start()
    {
        if (mbStopped)
        {
            const TimePoint currTime = ChronoClock::now();

            mPausedTime += currTime - mStopTime;

            mPrevFrameTime = currTime;
            mStopTime = TimePoint();

            mbStopped = false;

            mDeltaTime = Duration(0);
        }
    }

  private:
    Duration mDeltaTime;
    Duration mPausedTime;

    TimePoint mBaseTime;
    TimePoint mCurrFrameTime;
    TimePoint mPrevFrameTime;

    TimePoint mStopTime;

    bool mbStopped;
};
} // namespace ho