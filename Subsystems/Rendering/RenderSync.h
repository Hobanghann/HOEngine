#pragma once
#include <atomic>

#include "Config.h"
#include "Core/Thread/Thread.h"
#include "Macros.h"

namespace ho
{
class RenderSync final
{
  public:
    RenderSync()
      : mbRenderDone(true)
    {
    }

    FORCE_INLINE void WaitUntilRenderDone() const
    {
        while (!mbRenderDone.load(std::memory_order_acquire))
        {
            CPU_PAUSE();
        }
    }

    FORCE_INLINE void NotifyRenderDone()
    {
        mbRenderDone.store(true, std::memory_order_release);
    }

    FORCE_INLINE void WaitUntilEngineSwapComplete() const
    {
        while (mbRenderDone.load(std::memory_order_acquire))
        {
            CPU_PAUSE();
        }
    }

    FORCE_INLINE void NotifyEngineSwapComplete()
    {
        mbRenderDone.store(false, std::memory_order_release);
    }

    FORCE_INLINE bool IsRenderDone() const
    {
        return mbRenderDone.load(std::memory_order_acquire);
    }

  private:
    union
    {
        mutable std::atomic<bool> mbRenderDone;

        char mAligner[Thread::sCacheLineBytes];
    };
};

} // namespace ho