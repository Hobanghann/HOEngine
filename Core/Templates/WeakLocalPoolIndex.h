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
class WeakLocalPoolIndex
{
  public:
    constexpr WeakLocalPoolIndex()
      : mIndex(0)
      , mPool(nullptr)
    {
    }

    WeakLocalPoolIndex(PoolType<ValueType>* pool, uint32_t idx)
      : mPool(pool)
      , mIndex(idx)

    {
    }

    [[nodiscard]] bool operator==(const WeakLocalPoolIndex& rhs) const
    {
        return mIndex == rhs.mIndex && mPool == rhs.mPool;
    }

    [[nodiscard]] bool operator!=(const WeakLocalPoolIndex& rhs) const
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

    [[nodiscard]] constexpr uint32_t GetIndex() const
    {
        return mIndex;
    }

    static const WeakLocalPoolIndex sNull;

  private:
    uint32_t mIndex = 0;
    PoolType<ValueType>* mPool = nullptr;
};

template <typename ValueType, template <typename> class PoolType>
INLINE const WeakLocalPoolIndex<ValueType, PoolType> WeakLocalPoolIndex<ValueType, PoolType>::sNull =
    WeakLocalPoolIndex<ValueType, PoolType>(nullptr, 0);
} // namespace ho

namespace std
{
template <typename ValueType, template <typename> class PoolType>
struct hash<ho::WeakLocalPoolIndex<ValueType, PoolType>>
{
    size_t operator()(const ho::WeakLocalPoolIndex<ValueType, PoolType>& k) const noexcept
    {
        size_t idx = k.GetIndex();
        size_t h2 = idx != 0 ? std::hash<const void*>{}(&k.GetPool()) : 0;
        return idx ^ (h2 << 1);
    }
};
} // namespace std