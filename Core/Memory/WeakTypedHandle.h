#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include "Macros.h"
#include "TypedObjectPool.h"
#include "UniqueTypedObjectPool.h"

namespace ho
{

template <typename ValueType, template <typename> class PoolType>
class WeakTypedHandle final
{
  public:
    constexpr WeakTypedHandle()
      : mID(0)
      , mpPool(nullptr)
    {
    }

    WeakTypedHandle(PoolType<ValueType>* pool, uint64_t id)
      : mID(id)
      , mpPool(pool)
    {
        setDebugValue();
    }

    [[nodiscard]] bool operator==(const WeakTypedHandle& rhs) const
    {
        return mID == rhs.mID && mpPool == rhs.mpPool;
    }

    [[nodiscard]] bool operator!=(const WeakTypedHandle& rhs) const
    {
        return !(*this == rhs);
    }

    [[nodiscard]] bool IsNULL() const
    {
        return mID == 0ull || (mpPool == nullptr ? true : mpPool->Get(mID) == nullptr);
    }

    [[nodiscard]] bool IsValid() const
    {
        return !IsNULL();
    }

    [[nodiscard]] ValueType* operator->() const
    {
        return Get();
    }

    [[nodiscard]] ValueType& operator*() const
    {
        return *Get();
    }

    [[nodiscard]] ValueType* Get() const
    {
        HO_ASSERT(mpPool, "There is no bound pool.");
        return mpPool->Get(mID);
    }

    [[nodiscard]] constexpr uint64_t GetID() const
    {
        return mID;
    }

    [[nodiscard]] constexpr const PoolType<ValueType>* GetPool() const
    {
        return mpPool;
    }

    static const WeakTypedHandle sNULL;

  private:
    void setDebugValue()
    {
#if !defined(NDEBUG)
        mpDebugValue = (mpPool && mID != 0) ? mpPool->Get(mID) : nullptr;
#endif
    }

    uint64_t mID = 0;

#if !defined(NDEBUG)
    ValueType* mpDebugValue = nullptr;
#endif

    PoolType<ValueType>* mpPool = nullptr;
};

template <typename ValueType, template <typename> class PoolType>
INLINE const WeakTypedHandle<ValueType, PoolType> WeakTypedHandle<ValueType, PoolType>::sNULL =
    WeakTypedHandle<ValueType, PoolType>();
} // namespace ho

namespace std
{
template <typename ValueType, template <typename> class PoolType>
struct hash<ho::WeakTypedHandle<ValueType, PoolType>>
{
    size_t operator()(const ho::WeakTypedHandle<ValueType, PoolType>& k) const noexcept
    {
        uint64_t id = k.GetID();
        size_t seed = std::hash<uint64_t>{}(id);

        if (id != 0)
        {
            size_t h2 = std::hash<const void*>{}(k.GetPool());
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }
};
} // namespace std