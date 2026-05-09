#pragma once

#include <mutex>

#include "Core/Config.h"
#include "Macros.h"

namespace ho
{

#if defined(THREAD_ENABLED)

template <typename MutexT>
class MutexLock;

template <typename MutexT_STL>
class Mutex final
{
    friend class MutexLock<Mutex<MutexT_STL>>;

  public:
    using MutexType_STL = MutexT_STL;

    FORCE_INLINE void Lock() const
    {
        mMutex.lock();
    }

    FORCE_INLINE void Unlock() const
    {
        mMutex.unlock();
    }

    [[nodiscard]] FORCE_INLINE bool TryLock() const
    {
        return mMutex.try_lock();
    }

  private:
    mutable MutexT_STL mMutex;
};

using BinaryMutex = Mutex<std::mutex>;
using RecursiveMutex = Mutex<std::recursive_mutex>;

// Type Contracts
static_assert(std::is_same_v<BinaryMutex, Mutex<std::mutex>>);
static_assert(std::is_same_v<RecursiveMutex, Mutex<std::recursive_mutex>>);

template <typename MutexT>
class MutexLock final
{
    friend class ConditionVariable;

  public:
    FORCE_INLINE explicit MutexLock(const MutexT& mutex)
      : mLock(mutex.mMutex)
    {
    }

    FORCE_INLINE void Relock() const
    {
        mLock.lock();
    }

    FORCE_INLINE void Unlock() const
    {
        mLock.unlock();
    }

  private:
    mutable std::unique_lock<typename MutexT::MutexType_STL> mLock;
};

#else

template <typename MutexT_STL>
class Mutex final
{
  public:
    FORCE_INLINE void Lock() const {}

    FORCE_INLINE void Unlock() const {}

    FORCE_INLINE bool TryLock() const
    {
        return true;
    }
};

using BinaryMutex = Mutex<std::mutex>;
using RecursiveMutex = Mutex<std::recursive_mutex>;

template <typename MutexT>
class MutexLock final
{
    friend class ConditionVariable;

  public:
    FORCE_INLINE explicit MutexLock(const MutexT&) {}

    FORCE_INLINE void Relock() const {}

    FORCE_INLINE void Unlock() const {}
};

#endif // THREAD_ENABLED

} // namespace ho
