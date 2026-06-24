#pragma once

#include <atomic>

#include "Config.h"
#include "Macros.h"
#include "Thread.h"

namespace ho
{

#if defined(THREAD_ENABLED)

static_assert(std::atomic_bool::is_always_lock_free, "std::atomic<bool> must be lock-free for this SpinLock.");

class SpinLock final
{
  public:
    FORCE_INLINE void Lock() const
    {
        while (true)
        {
            bool bExpected = false;
            if (mbLocked.compare_exchange_weak(bExpected, true, std::memory_order_acquire, std::memory_order_relaxed))
            {
                break; // acquired
            }
            // brief backoff while contended
            do
            {
                CPU_PAUSE();
            } while (mbLocked.load(std::memory_order_relaxed));
        }
    }

    FORCE_INLINE void Unlock() const
    {
        mbLocked.store(false, std::memory_order_release);
    }

  private:
    union
    {
        mutable std::atomic<bool> mbLocked = false;
        char mAligner[Thread::sCacheLineBytes]; // reduce false sharing
    };
};

#else

class SpinLock final
{
  public:
    FORCE_INLINE void Lock() const {}

    FORCE_INLINE void Unlock() const {}
};

#endif // THREADS_ENABLED

} // namespace ho