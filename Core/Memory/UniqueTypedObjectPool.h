#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

namespace ho
{

template <typename T>
class UniqueTypedObjectPool final
{
  public:
    UniqueTypedObjectPool(int32_t size = 1024)
      : mObjectCount(0)
    {
        mPool.emplace_back(std::nullopt);
        mGen.emplace_back(0);
        mFreeIndices.reserve(size / 2);
    }

    [[nodiscard]] uint64_t Add(const T& v)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(mPool.size()); ++i)
        {
            if (!mPool[i].has_value())
            {
                continue;
            }

            if (mPool[i].value() == v)
            {
                return (static_cast<uint64_t>(mGen[i]) << 32) | i;
            }
        }

        uint32_t gen = 0;
        uint32_t index = 0;

        if (!mFreeIndices.empty())
        {
            index = mFreeIndices.back();
            mFreeIndices.pop_back();
            gen = mGen[index];
            mPool[index].emplace(v);
        }
        else
        {
            index = static_cast<uint32_t>(mPool.size());
            gen = 0;
            mGen.push_back(0);
            mPool.emplace_back(v);
        }
        ++mObjectCount;
        return (static_cast<uint64_t>(gen) << 32) | index;
    }

    [[nodiscard]] uint64_t Add(T&& v)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(mPool.size()); ++i)
        {
            if (!mPool[i].has_value())
            {
                continue;
            }

            if (mPool[i].value() == v)
            {
                return (static_cast<uint64_t>(mGen[i]) << 32) | i;
            }
        }

        uint32_t gen = 0;
        uint32_t index = 0;

        if (!mFreeIndices.empty())
        {
            index = mFreeIndices.back();
            mFreeIndices.pop_back();
            gen = mGen[index];
            mPool[index].emplace(std::move(v));
        }
        else
        {
            index = static_cast<uint32_t>(mPool.size());
            gen = 0;
            mGen.push_back(0);
            mPool.emplace_back(std::move(v));
        }
        ++mObjectCount;
        return (static_cast<uint64_t>(gen) << 32) | index;
    }

    template <typename... Args>
    [[nodiscard]] uint64_t Emplace(Args&&... args)
    {
        uint32_t gen = 0;
        uint32_t index = 0;

        if (!mFreeIndices.empty())
        {
            index = mFreeIndices.back();
            mFreeIndices.pop_back();
            gen = mGen[index];
            mPool[index].emplace(std::forward<Args>(args)...);
        }
        else
        {
            index = static_cast<uint32_t>(mPool.size());
            gen = 0;
            mGen.push_back(0);
            mPool.emplace_back(std::in_place, std::forward<Args>(args)...);
        }

        for (uint32_t i = 0; i < static_cast<uint32_t>(mPool.size()); ++i)
        {
            if (!mPool[i].has_value() || i == index)
            {
                continue;
            }

            if (mPool[i].value() == mPool[index].value())
            {
                mPool[index].reset();
                mFreeIndices.push_back(index);
                return (static_cast<uint64_t>(mGen[i]) << 32) | i;
            }
        }
        ++mObjectCount;
        return (static_cast<uint64_t>(gen) << 32) | index;
    }

    bool Remove(uint64_t id)
    {
        uint32_t gen = static_cast<uint32_t>(id >> 32);
        uint32_t index = static_cast<uint32_t>(id);

        if (index >= mPool.size() || !mPool[index].has_value() || mGen[index] != gen)
        {
            return false;
        }

        mPool[index].reset();
        ++mGen[index];

        mFreeIndices.push_back(index);
        --mObjectCount;
        return true;
    }

    bool Remove(const T& v)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(mPool.size()); ++i)
        {
            if (!mPool[i].has_value())
            {
                continue;
            }

            if (mPool[i].value() == v)
            {
                uint64_t id = (static_cast<uint64_t>(mGen[i]) << 32) | i;
                return Remove(id);
            }
        }
        return false;
    }

    [[nodiscard]] T* Get(uint64_t id)
    {
        uint32_t gen = static_cast<uint32_t>(id >> 32);
        uint32_t index = static_cast<uint32_t>(id);

        if (index >= mPool.size() || !mPool[index].has_value() || mGen[index] != gen)
        {
            return nullptr;
        }
        return &(mPool[index].value());
    }

    [[nodiscard]] const T* Get(uint64_t id) const
    {
        uint32_t gen = static_cast<uint32_t>(id >> 32);
        uint32_t index = static_cast<uint32_t>(id);

        if (index >= mPool.size() || !mPool[index].has_value() || mGen[index] != gen)
        {
            return nullptr;
        }
        return &(mPool[index].value());
    }

    [[nodiscard]] bool Has(uint64_t id) const
    {
        uint32_t gen = static_cast<uint32_t>(id >> 32);
        uint32_t index = static_cast<uint32_t>(id);

        return (index < mPool.size() && mPool[index].has_value() && mGen[index] == gen);
    }

    [[nodiscard]] bool Has(const T& v) const
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(mPool.size()); ++i)
        {
            if (!mPool[i].has_value())
            {
                continue;
            }

            if (mPool[i].value() == v)
            {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint64_t FindID(const T& v) const
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(mPool.size()); ++i)
        {
            if (!mPool[i].has_value())
            {
                continue;
            }

            if (mPool[i].value() == v)
            {
                return (static_cast<uint64_t>(mGen[i]) << 32) | i;
            }
        }
        return 0;
    }

    void Clear()
    {
        mObjectCount = 0;
        mPool.resize(1);
        mGen.resize(1);
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

    bool IsEmpty() const
    {
        return mObjectCount == 0;
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
    std::deque<uint32_t> mGen;
    std::vector<uint32_t> mFreeIndices;
};
} // namespace ho