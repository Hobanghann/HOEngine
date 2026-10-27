#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

namespace ho
{
template <typename T>
class TypedObjectPool final
{
  public:
    template <typename TElement>
    class Iterator
    {
      public:
        using RawType = std::remove_const_t<TElement>;

        using ContainerType = std::conditional_t<std::is_const_v<TElement>,
                                                 const std::deque<std::optional<RawType>>,
                                                 std::deque<std::optional<RawType>>>;

        Iterator(ContainerType* pPool, uint32_t index)
          : mpPool(pPool)
          , mIndex(index)
        {
            advanceToNextValid();
        }

        TElement& operator*() const
        {
            return (*mpPool)[mIndex].value();
        }

        TElement* operator->() const
        {
            return &((*mpPool)[mIndex].value());
        }

        Iterator& operator++()
        {
            ++mIndex;
            advanceToNextValid();
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const Iterator& rhs) const
        {
            return mIndex == rhs.mIndex;
        }

        bool operator!=(const Iterator& rhs) const
        {
            return !(*this == rhs);
        }

      private:
        void advanceToNextValid()
        {
            while (mpPool && mIndex < mpPool->size() && !(*mpPool)[mIndex].has_value())
            {
                ++mIndex;
            }
        }

        ContainerType* mpPool;
        uint32_t mIndex;
    };

    TypedObjectPool(int32_t size = 1024)
      : mObjectCount(0)
    {
        mPool.emplace_back(std::nullopt);
        mGen.emplace_back(0);
        mFreeIndices.reserve(size / 2);
    }

    template <typename U = T>
    uint64_t Add(U&& v)
    {
        uint32_t gen = 0;
        uint32_t index = 0;

        if (!mFreeIndices.empty())
        {
            index = mFreeIndices.back();
            mFreeIndices.pop_back();
            gen = mGen[index];
            mPool[index].emplace(std::forward<U>(v));
        }
        else
        {
            index = static_cast<uint32_t>(mPool.size());
            gen = 0;
            mGen.push_back(0);
            mPool.emplace_back(std::forward<U>(v));
        }

        ++mObjectCount;
        return (static_cast<uint64_t>(gen) << 32) | index;
    }

    template <typename... Args>
    uint64_t Emplace(Args&&... args)
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
        for (uint32_t i = 1; i < mPool.size(); ++i)
        {
            if (mPool[i].has_value() && mPool[i].value() == v)
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
        for (uint32_t i = 1; i < mPool.size(); ++i)
        {
            if (mPool[i].has_value() && mPool[i].value() == v)
            {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint64_t FindID(const T& v) const
    {
        for (uint32_t i = 1; i < mPool.size(); ++i)
        {
            if (mPool[i].has_value() && mPool[i].value() == v)
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

    // NOLINTBEGIN(readability-identifier-naming)

    [[nodiscard]] Iterator<T> begin()
    {
        return Iterator<T>(&mPool, 1);
    }

    [[nodiscard]] Iterator<T> end()
    {
        return Iterator<T>(&mPool, static_cast<uint32_t>(mPool.size()));
    }

    [[nodiscard]] Iterator<const T> begin() const
    {
        return Iterator<const T>(&mPool, 1);
    }

    [[nodiscard]] Iterator<const T> end() const
    {
        return Iterator<const T>(&mPool, static_cast<uint32_t>(mPool.size()));
    }

    [[nodiscard]] Iterator<const T> cbegin() const
    {
        return begin();
    }

    [[nodiscard]] Iterator<const T> cend() const
    {
        return end();
    }

    // NOLINTEND(readability-identifier-naming)

  private:
    int32_t mObjectCount;
    std::deque<std::optional<T>> mPool;
    std::deque<uint32_t> mGen;
    std::vector<uint32_t> mFreeIndices;
};
} // namespace ho