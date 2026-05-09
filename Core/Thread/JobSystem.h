#pragma once

#include <cstdint>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "ConditionVariable.h"
#include "Core/Config.h"
#include "Core/Macros.h"
#include "Core/Templates/AtomicNumeric.h"
#include "Mutex.h"
#include "SpinLock.h"
#include "Thread.h"

namespace ho
{

struct JobDesc
{
    using Entry = void (*)(void* pInput, int32_t size);
    Entry EntryFunctionPtr = nullptr;
    void* pInputData = nullptr;
    int32_t InputSize = 0;
    std::shared_ptr<AtomicNumeric<int32_t>> pCounter; // completion counter
};

#if defined(THREAD_ENABLED)
class JobSystem final
{
  public:
    class Worker final : public Thread
    {
      public:
        FORCE_INLINE explicit Worker(JobSystem& pOwner);

        Worker(const Worker&) = delete;
        Worker& operator=(const Worker&) = delete;

      private:
        // worker's entry function
        INLINE static void workerLoop(void* pOwner);
    };

  public:
    FORCE_INLINE JobSystem(const std::string& nameStr, int32_t workerCount);

    FORCE_INLINE ~JobSystem();

    FORCE_INLINE void KickJob(const JobDesc& job);

    FORCE_INLINE void KickJobs(const JobDesc* pJobs, int32_t jobCount);

    FORCE_INLINE void KickJobAndWait(const JobDesc& job);

    FORCE_INLINE void KickJobsAndWait(const JobDesc* pJobs, int32_t jobCount);

    FORCE_INLINE void WaitForCounter(std::shared_ptr<AtomicNumeric<std::int32_t>>& pCounter);

    FORCE_INLINE void WaitForIdle();

  private:
    static constexpr int32_t sSpinCount = 100;

    std::string mNameStr;
    BinaryMutex mMutex;
    ConditionVariable mCV;
    std::queue<JobDesc> mJobQueue;
    std::vector<std::unique_ptr<Worker>> mWorkerPool;
    AtomicNumeric<int32_t> mJobCount{0};
    bool mbRunning;
};

JobSystem::Worker::Worker(JobSystem& pOwner)
  : Thread(&Worker::workerLoop, &pOwner)
{
}

void JobSystem::Worker::workerLoop(void* pOwner)
{
    JobSystem* pJobSystem = static_cast<JobSystem*>(pOwner);
    JobDesc job;

    while (true)
    {
        {
            const MutexLock lock(pJobSystem->mMutex);
            while (pJobSystem->mJobQueue.empty() && pJobSystem->mbRunning)
            {
                pJobSystem->mCV.Wait(lock); // wait until job is kicked in queue
            }
            if (!pJobSystem->mbRunning)
            {
                return; // thread exit point
            }
            job = pJobSystem->mJobQueue.front();
            pJobSystem->mJobQueue.pop();
        }

        job.EntryFunctionPtr(job.pInputData, job.InputSize);

        if (job.pCounter != nullptr)
        {
            // if job was waited by job system
            // decrement count and if count is zero, wake up main thread
            if (job.pCounter->Decrement() == 0)
            {
                const MutexLock lock(pJobSystem->mMutex);
                pJobSystem->mCV.NotifyAll();
            }
        }

        if (pJobSystem->mJobCount.Decrement() == 0)
        {
            const MutexLock lock(pJobSystem->mMutex);
            pJobSystem->mCV.NotifyAll(); // for wake up main thread
        }
    }
}

JobSystem::JobSystem(const std::string& nameStr, int32_t workerCount)
  : mNameStr(nameStr)
  , mbRunning(true)
{
    mWorkerPool.reserve(workerCount);
    for (int32_t i = 0; i < workerCount; i++)
    {
        mWorkerPool.emplace_back(std::make_unique<Worker>(*this));
        mWorkerPool.back()->SetName(nameStr + "_Worker" + std::to_string(i));
    }
}

JobSystem::~JobSystem()
{
    {
        const MutexLock lock(mMutex);
        mbRunning = false;
        mCV.NotifyAll();
    }

    for (auto& worker : mWorkerPool)
    {
        worker->Join();
    }
}

void JobSystem::KickJob(const JobDesc& job)
{
    const MutexLock lock(mMutex);
    mJobCount.Increment();
    mJobQueue.push(job);
    mCV.NotifyOne();
}

void JobSystem::KickJobs(const JobDesc* pJobs, int32_t jobCount)
{
    HO_ASSERT(pJobs, "pJobs is null.");
    const MutexLock lock(mMutex);
    mJobCount.Add(jobCount);
    for (int32_t i = 0; i < jobCount; ++i)
    {
        mJobQueue.push(pJobs[i]);
    }
    mCV.NotifyAll();
}

void JobSystem::KickJobAndWait(const JobDesc& job)
{
    auto pCounter = std::make_shared<AtomicNumeric<int32_t>>(1);
    JobDesc j = job;
    j.pCounter = pCounter;
    KickJob(j);
    WaitForCounter(pCounter);
}

void JobSystem::KickJobsAndWait(const JobDesc* pJobs, int32_t jobCount)
{
    HO_ASSERT(pJobs, "pJobs is null.");
    auto pCounter = std::make_shared<AtomicNumeric<int32_t>>(jobCount);
    static std::vector<JobDesc> sJobsWithCounter;
    sJobsWithCounter.reserve(jobCount);

    for (int32_t i = 0; i < jobCount; ++i)
    {
        JobDesc job = pJobs[i];
        job.pCounter = pCounter;
        sJobsWithCounter.push_back(job);
    }

    KickJobs(sJobsWithCounter.data(), static_cast<int32_t>(sJobsWithCounter.size()));
    sJobsWithCounter.clear();
    WaitForCounter(pCounter);
}

void JobSystem::WaitForCounter(std::shared_ptr<AtomicNumeric<std::int32_t>>& pCounter)
{
    // busy wait while sSpinCount.
    int32_t spinCount = sSpinCount;
    while (pCounter->Get() > 0 && spinCount)
    {
        if (pCounter->Get() == 0)
        {
            return;
        }
        CPU_PAUSE();
        --spinCount;
    }
    if (pCounter->Get() == 0)
    {
        return;
    }

    const MutexLock lock(mMutex);
    while (pCounter->Get() > 0)
    {
        mCV.Wait(lock);
    }
}

void JobSystem::WaitForIdle()
{
    // busy wait while sSpinCount.
    for (int32_t i = 0; i < sSpinCount; ++i)
    {
        if (mJobCount.Get() == 0)
        {
            return;
        }
        CPU_PAUSE();
    }

    const MutexLock lock(mMutex);
    while (mJobCount.Get() > 0)
    {
        mCV.Wait(lock);
    }
}

#else

// Single-threaded fallback version (no threads)
class JobSystem final
{
  public:
    explicit JobSystem(const std::string&, int32_t) {}

    void KickJob(const JobDesc& job)
    {
        job.EntryFunctionPtr(job.pInputData, job.InputSize);
        if (job.pCounter != nullptr)
        {
            job.pCounter->Decrement();
        }
    }

    void KickJobs(const JobDesc* jobs, int32_t jobCount)
    {
        HO_ASSERT(jobs, "jobs is null.");
        for (int32_t i = 0; i < jobCount; ++i)
        {
            KickJob(jobs[i]);
        }
    }

    void WaitForCounter(std::shared_ptr<AtomicNumeric<std::int32_t>> pCounter)
    {
        (void)pCounter; // no-op, all jobs run inline
    }

    void KickJobAndWait(const JobDesc& job)
    {
        auto pCounter = std::make_shared<AtomicNumeric<int32_t>>(1);
        JobDesc j = job;
        j.pCounter = pCounter;
        KickJob(j);
    }

    void KickJobsAndWait(const JobDesc* jobs, int32_t jobCount)
    {
        HO_ASSERT(jobs, "jobs is null.");
        auto pCounter = std::make_shared<AtomicNumeric<int32_t>>(static_cast<int32_t>(jobCount));
        static std::vector<JobDesc> sJobsWithCounter;
        sJobsWithCounter.reserve(jobCount);

        for (int32_t i = 0; i < jobCount; ++i)
        {
            JobDesc job = jobs[i];
            job.pCounter = pCounter;
            sJobsWithCounter.push_back(job);
        }

        KickJobs(sJobsWithCounter.data(), static_cast<int32_t>(sJobsWithCounter.size()));
    }
};

#endif // THREAD_ENABLED

} // namespace ho
