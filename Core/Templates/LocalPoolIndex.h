#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include "Macros.h"
#include "ObjectPool.h"
#include "UniqueObjectPool.h"

namespace ho
{

template <typename ValueType, template <typename> class PoolType>
class LocalPoolIndex
{
  public:
    constexpr LocalPoolIndex()
      : mIndex(0)
      , mPool(nullptr)
    {
    }

    explicit LocalPoolIndex(PoolType<ValueType>* pool, const ValueType& val)
      : mPool(pool)
    {
        HO_ASSERT(mPool, "There is no bound pool.");
        mIndex = mPool->Add(val);
    }

    explicit LocalPoolIndex(PoolType<ValueType>* pool, ValueType&& val)
      : mPool(pool)
    {
        HO_ASSERT(mPool, "There is no bound pool.");
        mIndex = mPool->Add(std::move(val));
    }

    template <typename... Args>
    [[nodiscard]] static LocalPoolIndex Emplace(PoolType<ValueType>* pool, Args&&... args)
    {
        const uint32_t idx = pool->Emplace(std::forward<Args>(args)...);
        return LocalPoolIndex(pool, idx);
    }

    [[nodiscard]] bool operator==(const LocalPoolIndex& rhs) const
    {
        return mIndex == rhs.mIndex && mPool == rhs.mPool;
    }

    [[nodiscard]] bool operator!=(const LocalPoolIndex& rhs) const
    {
        return !(*this == rhs);
    }

    [[nodiscard]] constexpr bool IsNULL() const
    {
        return mIndex == 0ull || (mPool == nullptr ? true : mPool->Get(mIndex) == nullptr);
    }

    [[nodiscard]] constexpr bool IsValid() const
    {
        return !IsNULL();
    }

    [[nodiscard]] ValueType* Get()
    {
        HO_ASSERT(mPool, "There is no bound pool.");
        return mPool->Get(mIndex);
    }

    [[nodiscard]] const ValueType* Get() const
    {
        HO_ASSERT(mPool, "There is no bound pool.");
        return mPool->Get(mIndex);
    }

    bool Release()
    {
        HO_ASSERT(mPool, "There is no bound pool.");
        if (!IsValid())
        {
            return false;
        }

        const bool result = mPool->Remove(mIndex);

        mIndex = 0;
        mPool = nullptr;

        return result;
    }

    [[nodiscard]] constexpr uint32_t GetIndex() const
    {
        return mIndex;
    }

    [[nodiscard]] PoolType<ValueType>& GetMutablePool()
    {
        HO_ASSERT(mPool, "There is no bound pool.");
        return *mPool;
    }

    [[nodiscard]] const PoolType<ValueType>& GetPool() const
    {
        HO_ASSERT(mPool, "There is no bound pool.");
        return *mPool;
    }

    static const LocalPoolIndex sNULL;

  private:
    explicit LocalPoolIndex(PoolType<ValueType>* pool, uint32_t idx)
      : mIndex(idx)
      , mPool(pool)
    {
    }

    uint32_t mIndex = 0;
    PoolType<ValueType>* mPool = nullptr;
};

template <typename ValueType, template <typename> class PoolType>
INLINE const LocalPoolIndex<ValueType, PoolType> LocalPoolIndex<ValueType, PoolType>::sNULL =
    LocalPoolIndex<ValueType, PoolType>(nullptr, 0);
} // namespace ho

namespace std
{
template <typename ValueType, template <typename> class PoolType>
struct hash<ho::LocalPoolIndex<ValueType, PoolType>>
{
    size_t operator()(const ho::LocalPoolIndex<ValueType, PoolType>& k) const noexcept
    {
        size_t idx = k.GetIndex();
        size_t h2 = idx != 0 ? std::hash<const void*>{}(&k.GetPool()) : 0;
        return idx ^ (h2 << 1);
    }
};
} // namespace std