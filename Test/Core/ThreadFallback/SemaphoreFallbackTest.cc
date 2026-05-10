#define THREAD_DISABLED
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

#include "Core/Thread/Semaphore.h"

using namespace ho;

TEST(SemaphoreFallbackTest, Acquire_WhenCalls_NoOperation)
{
    Semaphore sem(0);

    sem.Acquire();

    SUCCEED();
}

TEST(SemaphoreFallbackTest, TryAcquire_WhenCalls_AlwaysReturnsTrue)
{
    Semaphore sem(0);

    EXPECT_TRUE(sem.TryAcquire());
    EXPECT_TRUE(sem.TryAcquire());
    EXPECT_TRUE(sem.TryAcquire());
}

TEST(SemaphoreFallbackTest, Release_WhenCalls_NoOperation)
{
    Semaphore sem(0);

    sem.Release();
    sem.Release();

    SUCCEED();
}
