#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

namespace ho
{
template <typename T>
class ObjectPool
{
  public:
    ObjectPool(int32_t size = 1024)
      : mObjectCount(0)
    {
        mPool.emplace_back(std::nullopt);
        mFreeIndices.reserve(size / 2);
    }

    template <typename U = T>
    uint32_t Add(U&& v)
    {
        uint32_t index = 0;

        if (!mFreeIndices.empty())
        {
            index = mFreeIndices.back();
            mFreeIndices.pop_back();
            mPool[index].emplace(std::forward<U>(v));
        }
        else
        {
            index = static_cast<uint32_t>(mPool.size());
            mPool.emplace_back(std::forward<U>(v));
        }

        ++mObjectCount;
        return index;
    }

    template <typename... Args>
    uint32_t Emplace(Args&&... args)
    {
        uint32_t index = 0;

        if (!mFreeIndices.empty())
        {
            index = mFreeIndices.back();
            mFreeIndices.pop_back();
            mPool[index].emplace(std::forward<Args>(args)...);
        }
        else
        {
            index = static_cast<uint32_t>(mPool.size());
            mPool.emplace_back(std::in_place, std::forward<Args>(args)...);
        }

        ++mObjectCount;
        return index;
    }

    bool Remove(uint32_t poolIndex)
    {
        if (poolIndex >= mPool.size() || !mPool[poolIndex].has_value())
        {
            return false;
        }

        mPool[poolIndex].reset();

        mFreeIndices.push_back(poolIndex);
        --mObjectCount;
        return true;
    }

    bool Remove(const T& v)
    {
        for (uint32_t i = 1; i < mPool.size(); ++i)
        {
            if (mPool[i].has_value() && mPool[i].value() == v)
            {
                return Remove(i);
            }
        }
        return false;
    }

    [[nodiscard]] T* Get(uint32_t poolIndex)
    {
        if (poolIndex >= mPool.size() || !mPool[poolIndex].has_value())
        {
            return nullptr;
        }
        return &(mPool[poolIndex].value());
    }

    [[nodiscard]] const T* Get(uint32_t poolIndex) const
    {
        if (poolIndex >= mPool.size() || !mPool[poolIndex].has_value())
        {
            return nullptr;
        }
        return &(mPool[poolIndex].value());
    }

    [[nodiscard]] bool Has(uint32_t poolIndex) const
    {
        return (poolIndex < mPool.size() && mPool[poolIndex].has_value());
    }

    [[nodiscard]] bool Has(const T& v) const
    {
        for (uint32_t i = 1; i < mPool.size(); ++i)
        {
            if (mPool[i].has_value() && mPool[i].value() == v)
            {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t FindIndex(const T& v) const
    {
        for (uint32_t i = 1; i < mPool.size(); ++i)
        {
            if (mPool[i].has_value() && mPool[i].value() == v)
            {
                return i;
            }
        }
        return 0;
    }

    void Clear()
    {
        mObjectCount = 0;
        mPool.resize(1);
        mPool[0].reset();
        mFreeIndices.clear();
    }

    int32_t GetSize() const
    {
        return mObjectCount;
    }

    int32_t GetFullSize() const
    {
        return static_cast<int32_t>(mPool.size());
    }

    const std::deque<std::optional<T>>& GetRawPool() const
    {
        return mPool;
    }

    std::deque<std::optional<T>>& GetMutableRawPool()
    {
        return mPool;
    }

  private:
    int32_t mObjectCount;
    std::deque<std::optional<T>> mPool;
    std::vector<uint32_t> mFreeIndices;
};
} // namespace ho