#define THREAD_DISABLED
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

#include "Core/Thread/BinaryRecursiveMutex.h"

using namespace ho;

// BinaryRecursiveMutex<Tag> FALLBACK Tests
TEST(BinaryRecursiveMutexFallbackTest, Lock_WhenCalls_NoOperation)
{
    BinaryRecursiveMutex<0> mutex;
    mutex.Lock();

    SUCCEED();
}

TEST(BinaryRecursiveMutexFallbackTest, Unlock_WhenCalls_NoOperation)
{
    BinaryRecursiveMutex<0> mutex;

    mutex.Unlock();

    SUCCEED();
}

TEST(BinaryRecursiveMutexFallbackTest, TryLock_WhenCalls_AlwaysReturnsTrue)
{
    BinaryRecursiveMutex<0> mutex;

    mutex.Lock();
    EXPECT_EQ(mutex.TryLock(), true);
    mutex.Unlock();
    EXPECT_EQ(mutex.TryLock(), true);
}

// MutexLock<BinaryRecursiveMutex<Tag>> Fallback Tests
TEST(BinaryRecursiveMutexLockFallbackTest, ConstructorAndDestructor_WhenCalls_NoOperation)
{
    BinaryRecursiveMutex<1> mutex;
    MutexLock<BinaryRecursiveMutex<1>> lock(mutex);
    SUCCEED();
}

TEST(BinaryRecursiveMutexLockFallbackTest, Unlock_WhenCalls_NoOperation)
{
    BinaryRecursiveMutex<1> mutex;
    MutexLock<BinaryRecursiveMutex<1>> lock(mutex);
    lock.Unlock();
    SUCCEED();
}

TEST(BinaryRecursiveMutexLockFallbackTest, Relock_WhenCalls_NoOperation)
{
    BinaryRecursiveMutex<1> mutex;
    MutexLock<BinaryRecursiveMutex<1>> lock(mutex);
    lock.Relock();
    SUCCEED();
}