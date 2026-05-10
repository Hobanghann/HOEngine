#define THREAD_ENABLED
#include "Core/Thread/BinaryRecursiveMutex.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace ho;

// BinaryRecursiveMutex<Tag> Tests
TEST(BinaryRecursiveMutexTest, Lock_WhenCalled_ExecutesWithoutCrashing)
{
    BinaryRecursiveMutex<0> mutex;

    mutex.Lock();
    mutex.Unlock();
}

TEST(BinaryRecursiveMutexTest, Lock_Recursive_ExecutesWithoutDeadLock)
{
    BinaryRecursiveMutex<0> mutex;

    mutex.Lock();
    mutex.Lock();
    mutex.Lock();

    mutex.Unlock();
    mutex.Unlock();
    mutex.Unlock();

    SUCCEED();
}

TEST(BinaryRecursiveMutexTest, TryLock_WhenUnlocked_ReturnsTrue)
{
    BinaryRecursiveMutex<0> mutex;

    EXPECT_TRUE(mutex.TryLock());
    mutex.Unlock();
}

TEST(BinaryRecursiveMutexTest, TryLock_WhenLocked_ReturnsTrue)
{
    BinaryRecursiveMutex<0> mutex;
    mutex.Lock();
    EXPECT_TRUE(mutex.TryLock());
    mutex.Unlock();
    mutex.Unlock();
}

TEST(BinaryRecursiveMutexTest, TryLock_WhenAlreadyLockedByAnotherThread_ReturnsFalse)
{
    BinaryRecursiveMutex<0> mutex;

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

TEST(BinaryRecursiveMutexTest, Lock_MultipleThreads_EnsuresSequentialAccessToCounter)
{
    BinaryRecursiveMutex<0> mutex;
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

TEST(BinaryRecursiveMutexTest, Lock_DeepRecursion_MaintainsCorrectCounterState)
{
    BinaryRecursiveMutex<0> mutex;

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

TEST(BinaryRecursiveMutexTest, TemplateArgument_MultipleTags_ThreadsDistinctByTag)
{
    BinaryRecursiveMutex<0> mutexA;
    BinaryRecursiveMutex<1> mutexB;

    std::atomic<int32_t> countA{0};
    std::atomic<int32_t> countB{0};

    auto entryA = [&]()
    {
        for (int32_t i = 0; i < 5000; ++i)
        {
            mutexA.Lock();
            ++countA;
            mutexA.Unlock();
        }
    };

    auto entryB = [&]()
    {
        for (int32_t i = 0; i < 5000; ++i)
        {
            mutexB.Lock();
            ++countB;
            mutexB.Unlock();
        }
    };

    std::thread t1(entryA);
    std::thread t2(entryB);
    std::thread t3(entryA);
    std::thread t4(entryB);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    EXPECT_EQ(countA.load(), 10000);
    EXPECT_EQ(countB.load(), 10000);
}

// MutexLock<BinaryRecursiveMutex<Tag>> Tests
TEST(BinaryRecursiveMutexLockTest, ConstructorAndDestructor_NestedCalls_LockAndThenUnlockMutex)
{
    BinaryRecursiveMutex<0> mutex;

    {
        MutexLock<BinaryRecursiveMutex<0>> lock(mutex);
        {
            MutexLock<BinaryRecursiveMutex<0>> lock2(mutex);
        }
    }

    EXPECT_TRUE(mutex.TryLock());
    mutex.Unlock();
}

TEST(BinaryRecursiveMutexLockTest, ConstructorAndDestructor_DeepRecursiveLockUnlock_ConsistentlyManagesLockState)
{
    BinaryRecursiveMutex<0> mutex;

    std::function<void(int32_t)> recurse = [&](int32_t depth)
    {
        MutexLock<BinaryRecursiveMutex<0>> lock(mutex);
        if (depth > 0)
        {
            recurse(depth - 1);
        }
    };

    recurse(30);

    EXPECT_TRUE(true);
}
