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
class TypedHandle final
{
  public:
    constexpr TypedHandle()
      : mID(0)
      , mpPool(nullptr)
    {
    }

    explicit TypedHandle(PoolType<ValueType>* pool, const ValueType& val)
      : mpPool(pool)
    {
        HO_ASSERT(mpPool, "There is no bound pool.");
        mID = mpPool->Add(val);
        setDebugValue();
    }

    explicit TypedHandle(PoolType<ValueType>* pool, ValueType&& val)
      : mpPool(pool)
    {
        HO_ASSERT(mpPool, "There is no bound pool.");
        mID = mpPool->Add(std::move(val));
        setDebugValue();
    }

    template <typename... Args>
    [[nodiscard]] static TypedHandle Emplace(PoolType<ValueType>* pool, Args&&... args)
    {
        const uint64_t id = pool->Emplace(std::forward<Args>(args)...);
        return TypedHandle(pool, id);
    }

    [[nodiscard]] bool operator==(const TypedHandle& rhs) const
    {
        return mID == rhs.mID && mpPool == rhs.mpPool;
    }

    [[nodiscard]] bool operator!=(const TypedHandle& rhs) const
    {
        return !(*this == rhs);
    }

    [[nodiscard]] bool IsNULL() const
    {
        return mpPool == nullptr ? true : !mpPool->Has(mID);
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

    bool Release()
    {
        HO_ASSERT(mpPool, "There is no bound pool.");
        if (!IsValid())
        {
            return false;
        }

        const bool result = mpPool->Remove(mID);

        mID = 0;
        mpPool = nullptr;
        setDebugValue();
        return result;
    }

    [[nodiscard]] constexpr uint64_t GetID() const
    {
        return mID;
    }

    [[nodiscard]] const PoolType<ValueType>& GetPool() const
    {
        HO_ASSERT(mpPool, "There is no bound pool.");
        return *mpPool;
    }

    static const TypedHandle sNULL;

  private:
    explicit TypedHandle(PoolType<ValueType>* pool, uint64_t id)
      : mID(id)
      , mpPool(pool)
    {
        setDebugValue();
    }

    void setDebugValue()
    {
#if !defined(NDEBUG)
        mpDebugValue = mpPool ? mpPool->Get(mID) : nullptr;
#endif
    }

    uint64_t mID = 0;

#if !defined(NDEBUG)
    ValueType* mpDebugValue = nullptr;
#endif

    PoolType<ValueType>* mpPool = nullptr;
};

template <typename ValueType, template <typename> class PoolType>
INLINE const TypedHandle<ValueType, PoolType> TypedHandle<ValueType, PoolType>::sNULL =
    TypedHandle<ValueType, PoolType>();
} // namespace ho

namespace std
{
template <typename ValueType, template <typename> class PoolType>
struct hash<ho::TypedHandle<ValueType, PoolType>>
{
    size_t operator()(const ho::TypedHandle<ValueType, PoolType>& k) const noexcept
    {
        size_t id = k.GetID();
        size_t seed = std::hash<size_t>{}(id);

        if (id != 0)
        {
            size_t h2 = std::hash<const void*>{}(&k.GetPool());
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }
};
} // namespace std