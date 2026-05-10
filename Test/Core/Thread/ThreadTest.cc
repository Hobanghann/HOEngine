#define THREAD_ENABLED
#include "Core/Thread/Thread.h"

#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace ho;

TEST(ThreadTest, Constructor_SingleThread_ExecutesEntryFunctionCorrectly)
{
    std::atomic<int32_t> value{0};

    auto entry = [](void* pUserData)
    {
        auto* counter = reinterpret_cast<std::atomic<int32_t>*>(pUserData);
        counter->fetch_add(1, std::memory_order_relaxed);
    };

    Thread t(entry, &value);
    t.Join();

    EXPECT_EQ(value.load(), 1);
}

TEST(ThreadTest, Constructor_SingleEnryFunctionMultipleThreads_ExecutesEntryFunctionCorrectly)
{
    std::atomic<int32_t> counter{0};

    auto entry = [](void* pUserData)
    {
        auto* count = reinterpret_cast<std::atomic<int32_t>*>(pUserData);
        for (int32_t i = 0; i < 5000; ++i)
        {
            count->fetch_add(1, std::memory_order_relaxed);
        }
    };

    Thread t1(entry, &counter);
    Thread t2(entry, &counter);
    Thread t3(entry, &counter);

    t1.Join();
    t2.Join();
    t3.Join();

    EXPECT_EQ(counter.load(), 15000);
}

TEST(ThreadTest, Destructor_SingleThread_ImplicitlyJoinsThread)
{
    std::atomic<int32_t> value{0};

    {
        auto entry = [](void* pUserData)
        {
            auto* v = reinterpret_cast<std::atomic<int32_t>*>(pUserData);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            v->store(42, std::memory_order_relaxed);
        };

        Thread t(entry, &value);
        // destructor should implicitly join when going out of scope
    }

    EXPECT_EQ(value.load(), 42);
}

TEST(ThreadTest, Yield_WhenCalled_ExecutesWithoutCrashing)
{
    for (int32_t i = 0; i < 10; ++i)
    {
        Thread::Yield();
    }
    SUCCEED();
}

TEST(ThreadTest, Sleep_GivenMilliseconds_DelaysExecutionAtLeastForTargetTime)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    Thread::Sleep(20);
    auto endTime = std::chrono::high_resolution_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    EXPECT_GE(elapsed, 15);
    EXPECT_LT(elapsed, 200);
}
