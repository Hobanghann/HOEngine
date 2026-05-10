#pragma once

#include <condition_variable>

#include "BinaryRecursiveMutex.h"
#include "Config.h"
#include "Mutex.h"

namespace ho
{

#if defined(THREAD_ENABLED)

class ConditionVariable final
{
  public:
    template <typename BinaryMutexT>
    FORCE_INLINE void Wait(const MutexLock<BinaryMutexT>& lock) const
    {
        mCV.wait(lock.mLock);
    }

    template <int32_t Tag>
    FORCE_INLINE void Wait(const MutexLock<BinaryRecursiveMutex<Tag>>& lock) const
    {
        mCV.wait(lock.mMutex.sTlsLock);
    }

    FORCE_INLINE void NotifyOne() const
    {
        mCV.notify_one();
    }

    FORCE_INLINE void NotifyAll() const
    {
        mCV.notify_all();
    }

  private:
    mutable std::condition_variable mCV;
};

#else

class ConditionVariable
{
  public:
    template <typename BinaryMutexT>
    FORCE_INLINE void Wait(const MutexLock<BinaryMutexT>&) const
    {
        // no-op in single-threaded mode
    }

    template <int32_t Tag>
    FORCE_INLINE void Wait(const MutexLock<BinaryRecursiveMutex<Tag>>&) const
    {
        // no-op in single-threaded mode
    }

    FORCE_INLINE void NotifyOne() const {}

    FORCE_INLINE void NotifyAll() const {}
};

#endif // THREAD_ENABLED

} // namespace ho
