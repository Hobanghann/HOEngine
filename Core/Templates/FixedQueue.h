#pragma once

#include <memory>

#include "Macros.h"

namespace ho
{
template <typename T>
class FixedQueue
{
  public:
    FixedQueue() = default;

    FixedQueue(int32_t size)
      : mSize(size)
      , mCapacity(size + 1)
    {
        HO_ASSERT(size >= 0, "Size cannot be negative.");
        if (mSize > 0)
        {
            mpQueue = std::make_unique<T[]>(mCapacity);
        }
    }

    FixedQueue(const FixedQueue&) = delete;
    FixedQueue& operator=(const FixedQueue&) = delete;

    FixedQueue(FixedQueue&& other) noexcept
      : mpQueue(std::move(other.mpQueue))
      , mHeadIdx(other.mHeadIdx)
      , mTailIdx(other.mTailIdx)
      , mSize(other.mSize)
      , mCapacity(other.mCapacity)
    {
    }

    FixedQueue& operator=(FixedQueue&& rhs) noexcept
    {
        if (this == &rhs)
        {
            return *this;
        }

        mpQueue = std::move(rhs.mpQueue);
        mHeadIdx = rhs.mHeadIdx;
        mTailIdx = rhs.mTailIdx;
        mSize = rhs.mSize;
        mCapacity = rhs.mCapacity;

        return *this;
    }

    template <typename U = T>
    void Push(U&& v)
    {
        if ((mTailIdx + 1) % mCapacity == mHeadIdx)
        {
            HO_ASSERT(false, "Queue is full.");
            return;
        }

        mpQueue[mTailIdx] = std::forward<U>(v);
        mTailIdx = (mTailIdx + 1) % mCapacity;
    }

    template <typename... Args>
    void Emplace(Args&&... args)
    {
        if ((mTailIdx + 1) % mCapacity == mHeadIdx)
        {
            HO_ASSERT(false, "Queue is full.");
            return;
        }

        mpQueue[mTailIdx] = T(std::forward<Args>(args)...);
        mTailIdx = (mTailIdx + 1) % mCapacity;
    }

    void Pop()
    {
        if (IsEmpty())
        {
            return;
        }

        mHeadIdx = (mHeadIdx + 1) % mCapacity;
    }

    T& Front()
    {
        HO_ASSERT(mHeadIdx != mTailIdx, "Queue is empty.");
        return mpQueue[mHeadIdx];
    }

    const T& Front() const
    {
        HO_ASSERT(mHeadIdx != mTailIdx, "Queue is empty.");
        return mpQueue[mHeadIdx];
    }

    int32_t GetCount() const
    {
        return (mTailIdx + mCapacity - mHeadIdx) % mCapacity;
    }

    int32_t GetSize() const
    {
        return mSize;
    }

    bool IsEmpty() const
    {
        return mHeadIdx == mTailIdx;
    }

    bool IsFull() const
    {
        return (mTailIdx + 1) % mCapacity == mHeadIdx;
    }

    void Release()
    {
        mpQueue.reset();
        mHeadIdx = 0;
        mTailIdx = 0;
        mSize = 0;
        mCapacity = 0;
    }

    bool IsReleased()
    {
        return mpQueue == nullptr;
    }

  private:
    std::unique_ptr<T[]> mpQueue = nullptr;
    int32_t mHeadIdx = 0;
    int32_t mTailIdx = 0;
    int32_t mSize = 0;
    int32_t mCapacity = 0;
};
} // namespace ho