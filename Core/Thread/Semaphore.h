#pragma once

#include <condition_variable>
#include <mutex>

#include "Core/Config.h"
#include "Macros.h"

namespace ho
{

#if defined(THREAD_ENABLED)

class Semaphore final
{
  public:
    FORCE_INLINE explicit Semaphore(int32_t count = 0)
      : mCount(count)
    {
    }

    FORCE_INLINE void Acquire()
    {
        std::unique_lock lock(mMutex);
        while (!mCount)
        {
            mCV.wait(lock);
        }
        --mCount;
    }

    [[nodiscard]] FORCE_INLINE bool TryAcquire()
    {
        const std::unique_lock lock(mMutex);
        if (mCount)
        {
            --mCount;
            return true;
        }
        return false;
    }

    FORCE_INLINE void Release()
    {
        const std::unique_lock lock(mMutex);
        ++mCount;
        mCV.notify_one();
    }

  private:
    int32_t mCount;
    std::condition_variable mCV;
    std::mutex mMutex;
};

#else

class Semaphore final
{
  public:
    explicit Semaphore(int32_t = 0) {}

    void Acquire() {}

    bool TryAcquire()
    {
        return true;
    }

    void Release() {}
};

#endif // THREAD_ENABLED

} // namespace ho
