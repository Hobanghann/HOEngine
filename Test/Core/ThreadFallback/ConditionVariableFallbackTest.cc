#define THREAD_DISABLED
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

#include "Core/Thread/ConditionVariable.h"

using namespace ho;

TEST(ConditionVariableFallbackTest, Wait_WhenCalls_NoOperation)
{
    BinaryMutex mutex;
    ConditionVariable cv;

    int32_t counter = 0;

    auto binaryMutexEntry = [&](void*)
    {
        MutexLock lock(mutex);
        cv.Wait(lock);
        ++counter;
    };

    binaryMutexEntry(nullptr);

    EXPECT_EQ(counter, 1);

    BinaryRecursiveMutex<0> recMutex;

    auto recMutexEntry = [&](void*)
    {
        MutexLock lock(recMutex);
        cv.Wait(lock);
        ++counter;
    };

    recMutexEntry(nullptr);

    EXPECT_EQ(counter, 2);
}

TEST(ConditionVariableFallbackTest, NotifyOne_WhenCalls_NoOperation)
{
    ConditionVariable cv;
    cv.NotifyOne();
    SUCCEED();
}

TEST(ConditionVariableFallbackTest, NotifyAll_WhenCalls_NoOperation)
{
    ConditionVariable cv;
    cv.NotifyAll();
    SUCCEED();
}