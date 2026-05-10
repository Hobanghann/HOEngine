#include "Core/Time/GameTimer.h"

#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#include "Core/Math/MathDefs.h"

using namespace std::chrono_literals;
using namespace ho;

constexpr auto DEFAULT_DELAY = 10ms;

static void advanceTime(std::chrono::milliseconds ms)
{
    std::this_thread::sleep_for(ms);
}

TEST(GameTimerTest, Reset_InitialState_TimeIsZero)
{
    GameTimer timer;
    timer.Reset();

    EXPECT_FLOAT_EQ(timer.GetDeltaTime(), 0.0f);
    EXPECT_NEAR(timer.GetTotalTime(), 0.0f, math::EPSILON_CMP);
}

TEST(GameTimerTest, Tick_AfterTimeElapses_CalculatesCorrectDeltaTime)
{
    GameTimer timer;
    timer.Reset();

    timer.Tick();
    auto start = std::chrono::steady_clock::now();

    advanceTime(DEFAULT_DELAY);
    timer.Tick();

    auto end = std::chrono::steady_clock::now();
    float expectedDelta = std::chrono::duration<float>(end - start).count();

    EXPECT_NEAR(timer.GetDeltaTime(), expectedDelta, math::EPSILON_CMP);
}

TEST(GameTimerTest, Stop_WhenPaused_TotalTimeDoesNotIncrease)
{
    GameTimer timer;
    timer.Reset();

    timer.Tick();
    advanceTime(DEFAULT_DELAY);
    timer.Tick();
    float timeBeforeStop = timer.GetTotalTime();

    timer.Stop();
    advanceTime(DEFAULT_DELAY);
    timer.Tick();

    EXPECT_NEAR(timer.GetTotalTime(), timeBeforeStop, math::EPSILON_CMP);
}

TEST(GameTimerTest, Start_AfterStop_ResumesTotalTimeCalculation)
{
    GameTimer timer;
    timer.Reset();

    timer.Tick();
    timer.Stop();
    advanceTime(DEFAULT_DELAY);

    timer.Start();
    advanceTime(DEFAULT_DELAY);
    timer.Tick();

    EXPECT_GT(timer.GetTotalTime(), 0.0f);
}

TEST(GameTimerTest, Tick_MultipleCalls_IncreasesTotalTime)
{
    GameTimer timer;
    timer.Reset();

    timer.Tick();
    advanceTime(DEFAULT_DELAY);
    timer.Tick();
    float firstTickTime = timer.GetTotalTime();

    advanceTime(DEFAULT_DELAY);
    timer.Tick();
    float secondTickTime = timer.GetTotalTime();

    EXPECT_GT(secondTickTime, firstTickTime);
}

TEST(GameTimerTest, StartAndStop_MultipleTimes_AccumulatesActiveTimeOnly)
{
    GameTimer timer;
    timer.Reset();

    timer.Tick();

    timer.Stop();
    advanceTime(DEFAULT_DELAY);
    timer.Start();
    timer.Tick();
    float timeAfterFirstPause = timer.GetTotalTime();

    timer.Stop();
    advanceTime(DEFAULT_DELAY);
    timer.Start();
    timer.Tick();
    float timeAfterSecondPause = timer.GetTotalTime();

    EXPECT_GT(timeAfterSecondPause, timeAfterFirstPause);
}

TEST(GameTimerTest, Start_WhenAlreadyRunning_HasNoEffect)
{
    GameTimer timer;
    timer.Reset();

    timer.Tick();
    float timeBeforeStart = timer.GetTotalTime();

    timer.Start();
    advanceTime(DEFAULT_DELAY);
    timer.Tick();
    float timeAfterStart = timer.GetTotalTime();

    EXPECT_GT(timeAfterStart, timeBeforeStart);
}