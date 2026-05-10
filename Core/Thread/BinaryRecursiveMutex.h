#pragma once

#include <mutex>
#include <thread>

#include "Config.h"
#include "Macros.h"
#include "Mutex.h"

// This class defines a recursive mutex that can be used with a condition
// variable. To achieve this, a lock and count are stored in TLS (thread-local
// storage) to avoid data races, while the underlying mutex is defined as a
// static member of the class so that the TLS lock can reference it. Therefore,
// instances of this mutex are distinguished by the class's template argument.

namespace ho
{

#if defined(THREAD_ENABLED)

template <int32_t Tag>
class BinaryRecursiveMutex
{
    friend class ConditionVariable;

  public:
    FORCE_INLINE void Lock() const
    {
        if (sTlsLockCount == 0)
        {
            sTlsLock.lock();
        }
        ++sTlsLockCount;
    }

    HO_DISABLE_COMPILER_WARNING_PUSH();
    HO_DISABLE_COMPILER_WARNING_FAILING_TO_HOLD_LOCK();

    FORCE_INLINE void Unlock() const
    {
        HO_ASSERT(sTlsLockCount > 0, "Mutex is not locked currently.");
        --sTlsLockCount;
        if (sTlsLockCount == 0)
        {
            sTlsLock.unlock();
        }
    }

    HO_DISABLE_COMPILER_WARNING_POP();

    [[nodiscard]] FORCE_INLINE bool TryLock() const
    {
        if (sTlsLockCount > 0)
        {
            ++sTlsLockCount;
            return true;
        }
        else if (sTlsLock.try_lock())
        {
            sTlsLockCount = 1;
            return true;
        }
        return false;
    }

  private:
    static std::mutex sMutex;
    static thread_local std::unique_lock<std::mutex> sTlsLock;
    static thread_local int32_t sTlsLockCount;
};

template <int32_t Tag>
std::mutex BinaryRecursiveMutex<Tag>::sMutex;

template <int32_t Tag>
thread_local std::unique_lock<std::mutex> BinaryRecursiveMutex<Tag>::sTlsLock(BinaryRecursiveMutex<Tag>::sMutex,
                                                                              std::defer_lock);

template <int32_t Tag>
thread_local int32_t BinaryRecursiveMutex<Tag>::sTlsLockCount = 0;

template <int32_t Tag>
class MutexLock<BinaryRecursiveMutex<Tag>>
{
    friend class ConditionVariable;

  public:
    FORCE_INLINE explicit MutexLock(const BinaryRecursiveMutex<Tag>& mutex)
      : mMutex(mutex)
    {
        mMutex.Lock();
    }

    FORCE_INLINE ~MutexLock()
    {
        mMutex.Unlock();
    }

    FORCE_INLINE void Relock() const
    {
        mMutex.Lock();
    }

    FORCE_INLINE void Unlock() const
    {
        mMutex.Unlock();
    }

  private:
    const BinaryRecursiveMutex<Tag>& mMutex; // NOLINT
};

#else

template <int32_t Tag>
class BinaryRecursiveMutex
{
  public:
    FORCE_INLINE void Lock() const {}

    FORCE_INLINE void Unlock() const {}

    FORCE_INLINE bool TryLock() const
    {
        return true;
    }
};

template <int32_t Tag>
class MutexLock<BinaryRecursiveMutex<Tag>>
{
  public:
    explicit MutexLock(const BinaryRecursiveMutex<Tag>&) {}

    ~MutexLock() {}

    FORCE_INLINE void Relock() const {}

    FORCE_INLINE void Unlock() const {}
};

#endif // THREAD_ENABLED

} // namespace ho
