#pragma once

#include <memory>

#include "Macros.h"

namespace ho
{
template <typename T>
class FixedArray
{
  public:
    FixedArray()
      : mpArray(nullptr)
      , mSize(0)
    {
    }

    FixedArray(int32_t size)
      : mSize(size)
    {
        HO_ASSERT(size >= 0, "Size cannot be negative.");
        if (mSize > 0)
        {
            mpArray = std::make_unique<T[]>(mSize);
        }
    }

    FixedArray(const FixedArray&) = delete;
    FixedArray& operator=(const FixedArray&) = delete;

    FixedArray(FixedArray&& other) noexcept
      : mpArray(std::move(other.mpArray))
      , mSize(other.mSize)
    {
    }

    FixedArray& operator=(FixedArray&& rhs) noexcept
    {
        if (this == &rhs)
        {
            return *this;
        }

        mpArray = std::move(rhs.mpArray);
        mSize = rhs.mSize;
        return *this;
    }

    T& operator[](int32_t index)
    {
        HO_ASSERT(index >= 0 && index < mSize, "Out of bound.");
        return mpArray[index];
    }

    const T& operator[](int32_t index) const
    {
        HO_ASSERT(index >= 0 && index < mSize, "Out of bound.");
        return mpArray[index];
    }

    T* Data()
    {
        return mpArray.get();
    }

    const T* Data() const
    {
        return mpArray.get();
    }

    // NOLINTBEGIN

    T* begin()
    {
        return mpArray.get();
    }

    T* end()
    {
        return mpArray.get() + mSize;
    }

    const T* begin() const
    {
        return mpArray.get();
    }

    const T* end() const
    {
        return mpArray.get() + mSize;
    }

    // NOLINTEND

    int32_t GetSize() const
    {
        return mSize;
    }

    void Release()
    {
        mpArray.reset();
        mSize = 0;
    }

    bool IsReleased()
    {
        return mpArray == nullptr;
    }

    void Fill(const T& value)
    {
        HO_ASSERT(mpArray != nullptr, "FixedArray is released.");
        if (mpArray)
        {
            for (int32_t i = 0; i < mSize; ++i)
            {
                mpArray[i] = value;
            }
        }
    }

  private:
    std::unique_ptr<T[]> mpArray;
    int32_t mSize;
};
} // namespace ho