#define THREAD_ENABLED
#include "Core/Thread/Mutex.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace ho;

// BinaryMutex Test
TEST(BinaryMutexTest, Lock_WhenCalled_ExecutesWithoutCrashing)
{
    BinaryMutex mutex;

    mutex.Lock();
    mutex.Unlock();

    SUCCEED();
}

TEST(BinaryMutexTest, TryLock_WhenUnlocked_ReturnsTrue)
{
    BinaryMutex mutex;

    EXPECT_TRUE(mutex.TryLock());
    mutex.Unlock();
}

TEST(BinaryMutexTest, TryLock_WhenLocked_ReturnsFalse)
{
    BinaryMutex mutex;
    mutex.Lock();
    EXPECT_FALSE(mutex.TryLock());
    mutex.Unlock();
}

TEST(BinaryMutexTest, TryLock_WhenAlreadyLockedByAnotherThread_ReturnsFalse)
{
    BinaryMutex mutex;

    mutex.Lock();

    std::atomic<bool> bTryFailed = false;

    std::thread t(
        [&]()
        {
            if (!mutex.TryLock())
            {
                bTryFailed = true;
            }
        });

    t.join();

    mutex.Unlock();

    EXPECT_TRUE(bTryFailed);
}

TEST(BinaryMutexTest, Lock_MultipleThreads_EnsuresSequentialAccessToCounter)
{
    BinaryMutex mutex;
    int32_t counter = 0;

    auto entry = [&]()
    {
        for (int32_t i = 0; i < 5000; ++i)
        {
            mutex.Lock();
            ++counter;
            mutex.Unlock();
        }
    };

    std::thread t1(entry);
    std::thread t2(entry);

    t1.join();
    t2.join();

    EXPECT_EQ(counter, 10000);
}

// RecursiveMutex Test
TEST(RecursiveMutexTest, Lock_WhenCalled_ExecutesWithoutCrashing)
{
    RecursiveMutex mutex;

    mutex.Lock();
    mutex.Unlock();

    SUCCEED();
}

TEST(RecursiveMutexTest, TryLock_SameThreadLockRecursively_DoesNotDeadlock)
{
    RecursiveMutex mutex;

    mutex.Lock();
    mutex.Lock();
    mutex.Lock();

    mutex.Unlock();
    mutex.Unlock();
    mutex.Unlock();

    SUCCEED();
}

TEST(RecursiveMutexTest, TryLock_SameThreadTriesRecursively_AlwaysReturnsTrue)
{
    RecursiveMutex mutex;

    EXPECT_TRUE(mutex.TryLock());
    EXPECT_TRUE(mutex.TryLock());
    EXPECT_TRUE(mutex.TryLock());

    mutex.Unlock();
    mutex.Unlock();
    mutex.Unlock();
}

TEST(RecursiveMutexTest, TryLock_WhenLockedByAnotherThread_ReturnsFalse)
{
    RecursiveMutex mutex;

    mutex.Lock();

    std::atomic<bool> bTryFailed{false};

    std::thread t(
        [&]()
        {
            if (!mutex.TryLock())
            {
                bTryFailed = true;
            }
        });

    t.join();

    mutex.Unlock();

    EXPECT_TRUE(bTryFailed);
}

TEST(RecursiveMutexTest, Lock_MultipleThreads_EnsuresSequentialAccessToCounter)
{
    RecursiveMutex mutex;
    int32_t counter = 0;

    auto entry = [&]()
    {
        for (int32_t i = 0; i < 5000; ++i)
        {
            mutex.Lock();
            ++counter;
            mutex.Unlock();
        }
    };

    std::thread t1(entry);
    std::thread t2(entry);

    t1.join();
    t2.join();

    EXPECT_EQ(counter, 10000);
}

TEST(RecursiveMutexTest, Lock_DeepRecursion_MaintainsCorrectCounterState)
{
    RecursiveMutex mutex;

    std::function<void(int32_t& counter, int32_t depth)> recurse;
    recurse = [&](int32_t& counter, int32_t depth)
    {
        mutex.Lock();
        counter++;

        if (depth > 0)
        {
            recurse(counter, depth - 1);
        }

        mutex.Unlock();
    };

    int32_t value = 0;
    recurse(value, 5);

    EXPECT_EQ(value, 6);
}

// MutexLock Test
TEST(MutexLockTest, ConstructorAndDestructor_SuccessiveCalls_LockAndThenUnlockMutex)
{
    BinaryMutex mutex;

    {
        MutexLock lock(mutex);
    }

    EXPECT_TRUE(mutex.TryLock());
    mutex.Unlock();
}

TEST(MutexLockTest, ConstructorAndDestructor_MultipleThreads_EnsureMutualExclusion)
{
    BinaryMutex mutex;
    std::atomic<int32_t> counter = 0;

    auto worker = [&]()
    {
        for (int32_t i = 0; i < 5000; ++i)
        {
            MutexLock lock(mutex);
            ++counter;
        }
    };

    std::thread t1(worker);
    std::thread t2(worker);
    std::thread t3(worker);

    t1.join();
    t2.join();
    t3.join();

    EXPECT_EQ(counter.load(), 15000);
}

TEST(MutexLockTest, Relock_AfterExplicitUnlock_RelockMutex)
{
    BinaryMutex mutex;

    MutexLock lock(mutex);
    lock.Unlock();

    lock.Relock();
    lock.Unlock();

    SUCCEED();
}

TEST(MutexLockTest, Relock_MultipleRelockAndUnlock_ConsistentlyManagesLockState)
{
    BinaryMutex mutex;

    MutexLock lock(mutex);

    for (int32_t i = 0; i < 10; ++i)
    {
        lock.Unlock();
        lock.Relock();
    }

    lock.Unlock();
    EXPECT_TRUE(mutex.TryLock());
    mutex.Unlock();
}

TEST(MutexLockTest, ConstructorAndDestructor_DeepRecursiveLockUnlock_ConsistentlyManagesLockState)
{
    RecursiveMutex mutex;

    std::function<void(int32_t)> recurse = [&](int32_t depth)
    {
        MutexLock lock(mutex);
        if (depth > 0)
        {
            recurse(depth - 1);
        }
    };

    recurse(50);

    SUCCEED();
}

TEST(MutexLockTest, ConstructorAndDestructor_DeepRecursiveLockUnlock_TracksDepthCorrectly)
{
    RecursiveMutex mutex;
    int32_t value = 0;

    std::function<void(int32_t)> func = [&](int32_t depth)
    {
        MutexLock lock(mutex);
        value += 1;

        if (depth > 0)
        {
            func(depth - 1);
        }
    };

    func(5);

    EXPECT_EQ(value, 6);
}