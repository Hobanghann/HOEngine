#include "Core/Templates/AtomicNumeric.h"

#include <gtest/gtest.h>

using namespace ho;

TEST(AtomicNumericTest, Constructor_WithInitializer_InitializeCorrectly)
{
    AtomicNumeric<int32_t> num(10);
    EXPECT_EQ(num.Get(), 10);
}

TEST(AtomicNumericTest, SetAndGet_MultiThreads_MaintainsAtomicity)
{
    AtomicNumeric<int32_t> num(0);
    const int32_t iterations = 10000;
    const int32_t threadCount = 4;

    std::vector<std::thread> writers;
    for (int32_t i = 0; i < threadCount; ++i)
    {
        writers.emplace_back(
            [&num, i]()
            {
                int32_t value = (i + 1) * 1111;
                for (int32_t j = 0; j < iterations; ++j)
                {
                    num.Set(value);
                }
            });
    }

    for (int32_t i = 0; i < iterations * 2; ++i)
    {
        int32_t val = num.Get();

        bool bValidValue = (val == 0 || val == 1111 || val == 2222 || val == 3333 || val == 4444);
        EXPECT_TRUE(bValidValue);
    }

    for (auto& t : writers)
    {
        t.join();
    }
}

TEST(AtomicNumericTest, IncrementAndDecrement_MultiThreads_MaintainsAtomicity)
{
    AtomicNumeric<int32_t> num(0);
    const int32_t iterations = 10000;

    auto incrementEntry = [&num]()
    {
        for (int32_t i = 0; i < iterations; ++i)
        {
            num.Increment();
        }
    };

    auto decrementEntry = [&num]()
    {
        for (int32_t i = 0; i < iterations; ++i)
        {
            num.Decrement();
        }
    };

    std::thread incThread(incrementEntry);
    std::thread decThread(decrementEntry);

    incThread.join();
    decThread.join();

    EXPECT_EQ(num.Get(), 0);
}

TEST(AtomicNumericTest, AddAndSub_MultiThreads_MaintainsAtomicity)
{
    AtomicNumeric<int32_t> num(0);
    const int32_t iterations = 10000;

    auto addEntry = [&num]()
    {
        for (int32_t i = 0; i < iterations; ++i)
        {
            num.Add(1);
        }
    };

    auto subEntry = [&num]()
    {
        for (int32_t i = 0; i < iterations; ++i)
        {
            num.Sub(1);
        }
    };

    std::thread addThread(addEntry);
    std::thread subThread(subEntry);

    addThread.join();
    subThread.join();

    EXPECT_EQ(num.Get(), 0);
}

TEST(AtomicNumericTest, ExchangeIfGreater_MultipleThreads_FinalValueIsMaximum)
{
    const int32_t initialValue = 0;
    AtomicNumeric<int32_t> num(initialValue);
    const int32_t threadCount = 10;
    const int32_t iterations = 1000;

    std::vector<std::thread> threads;
    for (int32_t i = 0; i < threadCount; ++i)
    {
        threads.emplace_back(
            [&num, i]()
            {
                for (int32_t j = 1; j <= iterations; ++j)
                {
                    int32_t newValue = (i * iterations) + j;
                    num.ExchangeIfGreater(newValue);
                }
            });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(num.Get(), threadCount * iterations);
}

TEST(AtomicNumericTest, ComparisonFunctions_MultipleThreads_ReflectsConcurrentChanges)
{
    AtomicNumeric<int32_t> num(0);
    std::atomic<bool> bStarted{false};

    std::thread t(
        [&]()
        {
            bStarted = true;
            while (!num.TestEqual(100))
            {
                std::this_thread::yield();
            }
        });

    while (!bStarted)
    {
        std::this_thread::yield();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    num.Set(100);

    t.join();
    SUCCEED();
}

TEST(AtomicNumericTest, TestNotEqual_MultipleThreads_ReflectsConcurrentChanges)
{
    AtomicNumeric<int32_t> num(42);
    std::atomic<bool> bStarted{false};

    std::thread t(
        [&]()
        {
            bStarted = true;
            while (!num.TestNotEqual(42))
            {
                std::this_thread::yield();
            }
        });

    while (!bStarted)
    {
        std::this_thread::yield();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    num.Set(100);

    t.join();
    SUCCEED();
}

TEST(AtomicNumericTest, TestLess_MultipleThreads_ReflectsConcurrentChanges)
{
    AtomicNumeric<int32_t> num(100);
    std::atomic<bool> bStarted{false};

    std::thread t(
        [&]()
        {
            bStarted = true;
            while (!num.TestLess(50))
            {
                std::this_thread::yield();
            }
        });

    while (!bStarted)
    {
        std::this_thread::yield();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    num.Set(30);

    t.join();
    SUCCEED();
}

TEST(AtomicNumericTest, TestGreater_MultipleThreads_ReflectsConcurrentChanges)
{
    AtomicNumeric<int32_t> num(10);
    std::atomic<bool> bStarted{false};

    std::thread t(
        [&]()
        {
            bStarted = true;
            while (!num.TestGreater(50))
            {
                std::this_thread::yield();
            }
        });

    while (!bStarted)
    {
        std::this_thread::yield();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    num.Set(70);

    t.join();
    SUCCEED();
}

TEST(AtomicNumericTest, TestLessEqual_MultipleThreads_ReflectsConcurrentChanges)
{
    AtomicNumeric<int32_t> num(100);
    std::atomic<bool> bStarted{false};

    std::thread t(
        [&]()
        {
            bStarted = true;
            while (!num.TestLessEqual(100))
            {
                std::this_thread::yield();
            }
        });

    num.Set(200);

    while (!bStarted)
    {
        std::this_thread::yield();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    num.Set(100);

    t.join();
    SUCCEED();
}

TEST(AtomicNumericTest, TestGreaterEqual_MultipleThreads_ReflectsConcurrentChanges)
{
    AtomicNumeric<int32_t> num(0);
    std::atomic<bool> bStarted{false};

    std::thread t(
        [&]()
        {
            bStarted = true;
            while (!num.TestGreaterEqual(50))
            {
                std::this_thread::yield();
            }
        });

    while (!bStarted)
    {
        std::this_thread::yield();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    num.Set(50);

    t.join();
    SUCCEED();
}