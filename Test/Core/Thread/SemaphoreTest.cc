#define THREAD_ENABLED
#include "Core/Thread/Semaphore.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace ho;

TEST(SemaphoreTest, Constructor_InitialCount_PermitsSpecificNumberOfAcquires)
{
    Semaphore sem(2);

    EXPECT_TRUE(sem.TryAcquire());
    EXPECT_TRUE(sem.TryAcquire());
    EXPECT_FALSE(sem.TryAcquire()); // no count left
}

TEST(SemaphoreTest, Acquire_WhenNoPermits_BlocksUntilRelease)
{
    Semaphore sem(0);
    std::atomic<bool> acquired{false};

    std::thread t(
        [&]()
        {
            sem.Acquire(); // should block
            acquired = true;
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(acquired.load()); // still blocked

    sem.Release();

    t.join();
    EXPECT_TRUE(acquired.load());
}

TEST(SemaphoreTest, TryAcquire_WhenOnePermit_OnlyOneTrySucceed)
{
    Semaphore sem(1);

    EXPECT_TRUE(sem.TryAcquire());
    EXPECT_FALSE(sem.TryAcquire());

    sem.Release();
    EXPECT_TRUE(sem.TryAcquire());
}

TEST(SemaphoreTest, Acquire_MultipleThreads_AllowsAccessUpToInitialCount)
{
    const int32_t count = 3;
    Semaphore sem(3);

    std::atomic<int32_t> acquiredCount{0};

    std::vector<std::thread> threads;
    for (int32_t i = 0; i < count; ++i)
    {
        threads.emplace_back(
            [&]()
            {
                sem.Acquire();
                ++acquiredCount;
            });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(acquiredCount.load(), count);
}

TEST(SemaphoreTest, Release_WithMultipleWaitingThreads_WakesOneThreadAtATime)
{
    Semaphore sem(0);
    std::atomic<int32_t> wakeCount{0};

    std::thread t1(
        [&]()
        {
            sem.Acquire();
            ++wakeCount;
        });
    std::thread t2(
        [&]()
        {
            sem.Acquire();
            ++wakeCount;
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_EQ(wakeCount.load(), 0);

    sem.Release();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_EQ(wakeCount.load(), 1);

    sem.Release();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_EQ(wakeCount.load(), 2);

    t1.join();
    t2.join();
}
