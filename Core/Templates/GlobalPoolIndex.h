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
class GlobalPoolIndex
{
  public:
    constexpr GlobalPoolIndex()
      : mIndex(0)
    {
    }

    explicit GlobalPoolIndex(const ValueType& val)
      : mIndex(mPool.Add(val))
    {
    }

    explicit GlobalPoolIndex(ValueType&& val)
      : mIndex(mPool.Add(std::move(val)))
    {
    }

    template <typename... Args>
    [[nodiscard]] static GlobalPoolIndex Emplace(Args&&... args)
    {
        return GlobalPoolIndex(mPool.Emplace(std::forward<Args>(args)...));
    }

    static const GlobalPoolIndex sNull;

    [[nodiscard]] bool operator==(const GlobalPoolIndex& rhs) const
    {
        return mIndex == rhs.mIndex;
    }

    [[nodiscard]] bool operator!=(const GlobalPoolIndex& rhs) const
    {
        return mIndex != rhs.mIndex;
    }

    [[nodiscard]] constexpr bool IsNULL() const
    {
        return mIndex == 0ull;
    }

    [[nodiscard]] constexpr bool IsValid() const
    {
        return !IsNULL();
    }

    [[nodiscard]] ValueType* Get()
    {
        return mPool.Get(mIndex);
    }

    [[nodiscard]] const ValueType* Get() const
    {
        return mPool.Get(mIndex);
    }

    bool Release()
    {
        if (mIndex == 0)
        {
            return false;
        }

        const bool result = mPool.Remove(mIndex);

        mIndex = 0;

        return result;
    }

    [[nodiscard]] constexpr uint32_t GetIndex() const
    {
        return mIndex;
    }

    [[nodiscard]] static PoolType<ValueType>& GetMutablePool()
    {
        return mPool;
    }

    [[nodiscard]] static const PoolType<ValueType>& GetPool()
    {
        return mPool;
    }

  private:
    explicit constexpr GlobalPoolIndex(uint32_t idx)
      : mIndex(idx)
    {
    }

    uint32_t mIndex = 0;
    INLINE static PoolType<ValueType> mPool;
};

template <typename ValueType, template <typename> class PoolType>
INLINE const GlobalPoolIndex<ValueType, PoolType> GlobalPoolIndex<ValueType, PoolType>::sNull{};

using StringHandle = GlobalPoolIndex<std::string, UniqueObjectPool>;
} // namespace ho

namespace std
{
template <typename ValueType, template <typename> class PoolType>
struct hash<ho::GlobalPoolIndex<ValueType, PoolType>>
{
    size_t operator()(const ho::GlobalPoolIndex<ValueType, PoolType>& k) const noexcept
    {
        return std::hash<uint32_t>{}(k.GetIndex());
    }
};
} // namespace std