#define THREAD_DISABLED
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

#include "Core/Thread/Mutex.h"

using namespace ho;

// BinaryMutex Fallback Tests
TEST(BinaryMutexFallbackTest, Lock_WhenCalls_NoOperation)
{
    BinaryMutex mutex;
    mutex.Lock();

    SUCCEED();
}

TEST(BinaryMutexFallbackTest, Unlock_WhenCalls_NoOperation)
{
    BinaryMutex mutex;
    mutex.Unlock();

    SUCCEED();
}

TEST(BinaryMutexFallbackTest, TryLock_WhenCalls_AlwaysReturnsTrue)
{
    BinaryMutex mutex;

    mutex.Lock();
    EXPECT_EQ(mutex.TryLock(), true);
    mutex.Unlock();
    EXPECT_EQ(mutex.TryLock(), true);
}

// RecursiveMutex Fallback Tests
TEST(RecursiveMutexFallbackTest, Lock_WhenCalls_NoOperation)
{
    RecursiveMutex mutex;
    EXPECT_TRUE(mutex.TryLock());
    EXPECT_TRUE(mutex.TryLock());
    EXPECT_TRUE(mutex.TryLock());
}

TEST(RecursiveMutexFallbackTest, Unlock_WhenCalls_NoOperation)
{
    RecursiveMutex mutex;
    mutex.Unlock();

    SUCCEED();
}

TEST(RecursiveMutexFallbackTest, TryLock_WhenCalls_AlwaysReturnsTrue)
{
    RecursiveMutex mutex;

    mutex.Lock();
    EXPECT_EQ(mutex.TryLock(), true);
    mutex.Unlock();
    EXPECT_EQ(mutex.TryLock(), true);
}

// MutexLock Fallback Tests
TEST(MutexLockFallbackTest, ConstructorAndDestructor_WhenCalls_NoOperation)
{
    BinaryMutex mutex;
    MutexLock<BinaryMutex> lock(mutex);
    SUCCEED();
}

TEST(MutexLockFallbackTest, Unlock_WhenCalls_NoOperation)
{
    BinaryMutex mutex;
    MutexLock<BinaryMutex> lock(mutex);
    lock.Unlock();
    SUCCEED();
}

TEST(MutexLockFallbackTest, Relock_WhenCalls_NoOperation)
{
    BinaryMutex mutex;
    MutexLock<BinaryMutex> lock(mutex);
    lock.Relock();
    SUCCEED();
}
