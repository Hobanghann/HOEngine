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
class GlobalTypedHandle final
{
  public:
    constexpr GlobalTypedHandle()
      : mID(0)
    {
    }

    explicit GlobalTypedHandle(const ValueType& val)
      : mID(sPool.Add(val))
    {
        setDebugValue();
    }

    explicit GlobalTypedHandle(ValueType&& val)
      : mID(sPool.Add(std::move(val)))
    {
        setDebugValue();
    }

    template <typename... Args>
    [[nodiscard]] static GlobalTypedHandle Emplace(Args&&... args)
    {
        return GlobalTypedHandle(sPool.Emplace(std::forward<Args>(args)...));
    }

    static const GlobalTypedHandle sNULL;

    [[nodiscard]] bool operator==(const GlobalTypedHandle& rhs) const
    {
        return mID == rhs.mID;
    }

    [[nodiscard]] bool operator!=(const GlobalTypedHandle& rhs) const
    {
        return mID != rhs.mID;
    }

    [[nodiscard]] bool IsNULL() const
    {
        return mID == 0ull || sPool.Get(mID) == nullptr;
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
        ValueType* ptr = Get();
        HO_ASSERT(ptr, "Attempted to dereference an invalid handle.");
        return *ptr;
    }

    [[nodiscard]] ValueType* Get() const
    {
        return sPool.Get(mID);
    }

    bool Release()
    {
        if (mID == 0)
        {
            return false;
        }

        const bool result = sPool.Remove(mID);

        mID = 0;
        setDebugValue();

        return result;
    }

    [[nodiscard]] constexpr uint64_t GetID() const
    {
        return mID;
    }

    [[nodiscard]] static const PoolType<ValueType>& GetPool()
    {
        return sPool;
    }

  private:
    explicit GlobalTypedHandle(uint64_t id)
      : mID(id)
    {
        setDebugValue();
    }

    void setDebugValue()
    {
#if !defined(NDEBUG)
        mpDebugValue = sPool.Get(mID);
#endif
    }

    uint64_t mID = 0;

#if !defined(NDEBUG)
    ValueType* mpDebugValue = nullptr;
#endif

    INLINE static PoolType<ValueType> sPool;
};

template <typename ValueType, template <typename> class PoolType>
INLINE const GlobalTypedHandle<ValueType, PoolType> GlobalTypedHandle<ValueType, PoolType>::sNULL{};

} // namespace ho

namespace std
{
template <typename ValueType, template <typename> class PoolType>
struct hash<ho::GlobalTypedHandle<ValueType, PoolType>>
{
    size_t operator()(const ho::GlobalTypedHandle<ValueType, PoolType>& k) const noexcept
    {
        return std::hash<uint64_t>{}(k.GetID());
    }
};
} // namespace std