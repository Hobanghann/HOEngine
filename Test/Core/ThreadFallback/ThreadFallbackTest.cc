#define THREAD_DISABLED
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#include "Core/Thread/Thread.h"

using namespace ho;

constexpr int32_t SLEEP_TIME = 50;

TEST(ThreadFallbackTest, Constructor_SingleThread_NoActualThreadCreatedAndEntryFunctionExecutedImmediately)
{
    std::atomic<int32_t> value{0};

    auto entry = [](void* pUseDdata) { reinterpret_cast<std::atomic<int32_t>*>(pUseDdata)->store(123); };

    Thread t(entry, &value);
    EXPECT_EQ(value.load(), 123);
}

TEST(ThreadFallbackTest, Join_WhenCalls_NoOperation)
{
    std::atomic<int32_t> dummy{0};
    Thread t([](void*) {}, &dummy);

    t.Join();

    SUCCEED();
}

TEST(ThreadFallbackTest, Yield_WhenCalls_NoOperation)
{
    Thread::Yield();
    SUCCEED();
}

TEST(ThreadFallbackTest, Sleep_WhenCalls_NoOperation)
{
    auto start = std::chrono::high_resolution_clock::now();
    Thread::Sleep(SLEEP_TIME);
    auto end = std::chrono::high_resolution_clock::now();

    EXPECT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), SLEEP_TIME);
}
