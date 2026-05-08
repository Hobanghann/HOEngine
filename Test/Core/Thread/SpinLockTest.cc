#define THREAD_ENABLED
#include "Core/Thread/SpinLock.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace ho;

TEST(SpinLockTest, Acquire_WhenAlreadyLocked_BlocksUntilUnlock)
{
    int32_t testValue = 0;
    SpinLock lock;

    lock.Lock();

    auto entry = [&](void* pUserData)
    {
        lock.Lock();
        ++testValue;
    };

    std::thread t(entry, nullptr);

    EXPECT_EQ(testValue, 0);

    lock.Unlock();

    t.join();
}

TEST(SpinLockTest, MutualExclusion_MultipleThreads_EnsuresSequentialAccess)
{
    SpinLock lock;
    int32_t counter{0};

    auto entry = [&]()
    {
        for (int32_t i = 0; i < 20000; ++i)
        {
            lock.Lock();
            ++counter;
            lock.Unlock();
        }
    };

    std::thread t1(entry);
    std::thread t2(entry);
    std::thread t3(entry);

    t1.join();
    t2.join();
    t3.join();

    EXPECT_EQ(counter, 60000);
}
