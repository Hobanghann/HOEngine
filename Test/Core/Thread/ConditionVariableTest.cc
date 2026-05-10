#define THREAD_ENABLED
#include "Core/Thread/ConditionVariable.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace ho;

TEST(ConditionVariableTest, WaitAndNotify_WithBinaryMutex_SynchronizesThreads)
{
    BinaryMutex mutex;
    ConditionVariable cv;
    bool bReady = false;

    std::thread t(
        [&]()
        {
            MutexLock lock(mutex);
            while (!bReady)
            {
                cv.Wait(lock);
            }
            bReady = false;
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    {
        MutexLock lock(mutex);
        bReady = true;
        cv.NotifyOne();
    }

    t.join();
    SUCCEED();
}

TEST(ConditionVariableTest, WaitAndNotify_WithBinaryRecursiveMutex_SynchronizesThreads)
{
    BinaryRecursiveMutex<0> mutex;
    ConditionVariable cv;
    bool bReady = false;

    std::thread t(
        [&]()
        {
            MutexLock<BinaryRecursiveMutex<0>> lock(mutex);

            while (!bReady)
            {
                cv.Wait(lock);
            }

            bReady = false;
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    {
        MutexLock<BinaryRecursiveMutex<0>> lock(mutex);
        bReady = true;
        cv.NotifyOne();
    }

    t.join();
    SUCCEED();
}

TEST(ConditionVariableTest, WaitAndNotify_WithBinaryMutex_ThreadsWaitAndCurrectlyNotifyAll)
{
    BinaryMutex mutex;
    ConditionVariable cv;
    bool bReady = false;
    const int32_t threadCount = 5;

    std::atomic<int32_t> wakeCount = 0;
    std::atomic<int32_t> waitCount = 0;

    std::vector<std::thread> threads;
    for (int32_t i = 0; i < threadCount; ++i)
    {
        threads.emplace_back(
            [&]()
            {
                MutexLock lock(mutex);
                ++waitCount;
                while (!bReady)
                {
                    cv.Wait(lock);
                }
                ++wakeCount;
            });
    }

    while (waitCount.load() < threadCount)
    {
        std::this_thread::yield();
    }

    {
        MutexLock lock(mutex);
        bReady = true;
        cv.NotifyAll();
    }

    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(wakeCount.load(), threadCount);
}

TEST(ConditionVariableTest, NotifyOne_MultipleThreads_WakeOneThreadOnly)
{
    BinaryMutex mutex;
    ConditionVariable cv;

    bool bReady = false;
    std::atomic<int32_t> wakeCount{0};
    std::atomic<int32_t> waitCount{0};

    auto entry = [&]()
    {
        MutexLock lock(mutex);
        ++waitCount;
        while (!bReady)
        {
            cv.Wait(lock);
        }
        ++wakeCount;
    };

    std::thread t1(entry);
    std::thread t2(entry);

    while (waitCount.load() < 2)
    {
        std::this_thread::yield();
    }

    {
        MutexLock lock(mutex);
        bReady = true;
        cv.NotifyOne();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    EXPECT_EQ(wakeCount.load(), 1);

    {
        MutexLock lock(mutex);
        cv.NotifyAll();
    }

    t1.join();
    t2.join();
}

TEST(ConditionVariableTest, NotifyAll_MultipleThreads_WakeAllThreads)
{
    BinaryMutex mutex;
    ConditionVariable cv;

    bool bReady = false;
    std::atomic<int32_t> wakeCount{0};
    std::atomic<int32_t> waitCount{0};

    auto entry = [&]()
    {
        MutexLock lock(mutex);
        ++waitCount;
        while (!bReady)
        {
            cv.Wait(lock);
        }
        ++wakeCount;
    };

    std::thread t1(entry);
    std::thread t2(entry);

    while (waitCount.load() < 2)
    {
        std::this_thread::yield();
    }

    {
        MutexLock lock(mutex);
        bReady = true;
        cv.NotifyAll();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    EXPECT_EQ(wakeCount.load(), 2);

    t1.join();
    t2.join();
}
