#define THREAD_DISABLED
#include <gtest/gtest.h>
#include <vector>

#include "Core/Thread/JobSystem.h"

using namespace ho;

static void addInt(void* pData, int32_t size)
{
    int32_t* p = reinterpret_cast<int32_t*>(pData);
    ++(*p);
}

TEST(JobSystemFallbackTest, KickJob_SingleJob_KickerThreadExecutesJobImmediately)
{
    JobSystem js(std::string(), 1);

    int32_t value = 0;
    JobDesc job{addInt, &value, sizeof(int32_t), nullptr};

    js.KickJob(job);

    EXPECT_EQ(value, 1);
}

TEST(JobSystemFallbackTest, KickJobs_MultipleJobs_KickerThreadExecutesJobsImmediately)
{
    JobSystem js(std::string(), 1);

    const int32_t jobCount = 16;
    std::vector<int32_t> values(jobCount, 0);

    std::vector<JobDesc> jobs;
    jobs.reserve(jobCount);

    for (int32_t i = 0; i < jobCount; ++i)
    {
        jobs.push_back({addInt, &values[i], sizeof(int32_t), nullptr});
    }

    js.KickJobs(jobs.data(), jobs.size());

    for (auto value : values)
    {
        EXPECT_EQ(value, 1);
    }
}

TEST(JobSystemFallbackTest, KickJobAndWait_SingleJob_KickerThreadExecutesJobImmediately)
{
    JobSystem js(std::string(), 1);

    int32_t value = 0;
    JobDesc job{addInt, &value, sizeof(int32_t), nullptr};

    js.KickJobAndWait(job);
    EXPECT_EQ(value, 1);
}

TEST(JobSystemFallbackTest, KickJobsAndWait_MultipleJob_KickerThreadExecutesJobImmediately)
{
    JobSystem js(std::string(), 1);

    const int32_t jobCount = 25;
    std::vector<int32_t> values(jobCount, 0);

    std::vector<JobDesc> jobs;
    jobs.reserve(jobCount);

    for (int32_t i = 0; i < jobCount; ++i)
    {
        jobs.push_back({addInt, &values[i], sizeof(int32_t), nullptr});
    }

    js.KickJobsAndWait(jobs.data(), jobs.size());

    for (auto value : values)
    {
        EXPECT_EQ(value, 1);
    }
}
