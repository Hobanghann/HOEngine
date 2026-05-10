#define THREAD_DISABLED
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

#include "Core/Thread/SpinLock.h"

using namespace ho;

TEST(SpinLockFallbackTest, Lock_WhenCalls_NoOperation)
{
    SpinLock lock;

    lock.Lock();

    SUCCEED();
}

TEST(SpinLockFallbackTest, Unlock_WhenCalls_NoOperation)
{
    SpinLock lock;

    lock.Unlock();

    SUCCEED();
}
