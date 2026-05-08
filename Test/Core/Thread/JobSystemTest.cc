#define THREAD_ENABLED
#include "Core/Thread/JobSystem.h"

#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace ho;

constexpr int32_t SLEEP_TIME = 20;

static void addInt(void* pData, int32_t size)
{
    int32_t* p = reinterpret_cast<int32_t*>(pData);
    ++(*p);
}

void sleepAndAddInt(void* pData, int32_t size)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    int32_t* p = reinterpret_cast<int32_t*>(pData);
    ++(*p);
}

TEST(JobSystemTest, KickJob_SingleJob_ExecutedByWorkerThread)
{
    JobSystem js(std::string(), 4);

    int32_t value = 0;
    JobDesc job{addInt, &value, sizeof(int32_t), nullptr};

    js.KickJob(job);
    js.WaitForIdle();

    EXPECT_EQ(value, 1);
}

TEST(JobSystemTest, KickJobs_MultipleJobs_ExecutedByWorkerThreads)
{
    JobSystem js(std::string(), 4);

    const int32_t jobCount = 32;
    std::vector<int32_t> values(jobCount, 0);

    std::vector<JobDesc> jobs;
    jobs.reserve(jobCount);

    for (size_t i = 0; i < jobCount; ++i)
    {
        jobs.push_back({addInt, &values[i], sizeof(int32_t), nullptr});
    }

    js.KickJobs(jobs.data(), jobCount);
    js.WaitForIdle();

    for (auto value : values)
    {
        EXPECT_EQ(value, 1);
    }
}

TEST(JobSystemTest, KickJobAndWait_SngleJob_BlocksMainThreadUntilCompletion)
{
    JobSystem js(std::string(), 4);

    int32_t value = 0;
    JobDesc job{sleepAndAddInt, &value, sizeof(int32_t), nullptr};

    auto startTime = std::chrono::steady_clock::now();
    js.KickJobAndWait(job);
    auto endTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    EXPECT_EQ(value, 1);
    EXPECT_GE(elapsed, SLEEP_TIME);
}

TEST(JobSystemTest, KickJobsAndWait_MultipleJobs_BlocksMainThreadUntilCompletion)
{
    JobSystem js(std::string(), 4);

    const int32_t jobCount = 50;
    std::vector<int32_t> values(jobCount, 0);

    std::vector<JobDesc> jobs;
    jobs.reserve(jobCount);

    for (size_t i = 0; i < jobCount; ++i)
    {
        jobs.push_back({sleepAndAddInt, &values[i], sizeof(int32_t), nullptr});
    }

    auto startTime = std::chrono::steady_clock::now();
    js.KickJobsAndWait(jobs.data(), jobCount);
    auto endTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    for (size_t i = 0; i < jobCount; ++i)
    {
        EXPECT_EQ(values[i], 1) << i;
    }

    EXPECT_GE(elapsed, SLEEP_TIME * jobCount / 4);
}

void nestedSlowJob(void* pData, int32_t size)
{
    auto counter = *static_cast<std::atomic<int32_t>**>(pData);

    if (counter->load() % 13 == 0)
    {
        std::this_thread::yield();
    }

    counter->fetch_add(1);
}

TEST(JobSystemTest, KickJobs_MultiplePublishers_ExecutesAllJobsWithoutLoss)
{
    JobSystem js(std::string(), 8);
    const int32_t outerJobCount = 10000;
    std::atomic<int32_t> counter{0};
    std::atomic<int32_t>* pCounter = &counter;

    std::vector<std::thread> publishers;
    for (int32_t t = 0; t < 4; ++t)
    {
        publishers.emplace_back(
            [&js, pCounter, outerJobCount]()
            {
                for (int32_t i = 0; i < outerJobCount; ++i)
                {
                    JobDesc job{[](void* pData, int32_t size)
                                {
                                    auto counter = static_cast<std::atomic<int32_t>*>(pData);
                                    counter->fetch_add(1);
                                },
                                static_cast<void*>(pCounter),
                                sizeof(std::atomic<int32_t>),
                                nullptr};

                    js.KickJob(job);
                }
            });
    }

    for (auto& publisher : publishers)
    {
        publisher.join();
    }

    js.WaitForIdle();

    EXPECT_EQ(counter.load(), 40000);
}
