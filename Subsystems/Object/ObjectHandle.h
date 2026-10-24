#pragma once

#include <utility>

#include "ObjectPool.h"
#include "Subsystems/Reflection/ReflectionSystem.h"

namespace ho
{

template <typename U>
class ObjectIterator;

template <typename T>
class ObjectHandle final
{
    template <typename TConvertible>
    friend class ObjectHandle;

    template <typename U>
    friend class ObjectIterator;

    friend class ObjectPool;
    friend class ObjectManager;

  public:
    ObjectHandle()
      : mpObjectType(nullptr)
      , mID(0)
      , mpObjectPool(nullptr)
    {
        *this = GetNULL();
        setDebugValue();
    }

    template <typename TConvertible, typename = std::enable_if_t<std::is_convertible_v<TConvertible*, T*>>>
    ObjectHandle(const ObjectHandle<TConvertible>& other)
      : mpObjectType(other.mpObjectType)
      , mID(other.mID)
      , mpObjectPool(other.mpObjectPool)
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<TConvertible>>,
                      "TConvertible is not Object derived class.");
        HO_ASSERT(ReflectionSystem::GetInstance().GetClassByType<TConvertible>().IsBaseOfRecursive(*mpObjectType),
                  "U must be ancestor class of actual object type.");
        setDebugValue();
    }

    template <typename TConvertible, typename = std::enable_if_t<std::is_convertible_v<TConvertible*, T*>>>
    ObjectHandle& operator=(const ObjectHandle<TConvertible>& rhs)
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<TConvertible>>,
                      "TConvertible is not Object derived class.");
        HO_ASSERT(ReflectionSystem::GetInstance().GetClassByType<TConvertible>().IsBaseOfRecursive(*rhs.mpObjectType),
                  "U must be ancestor class of actual object type.");

        mpObjectType = rhs.mpObjectType;
        mID = rhs.mID;
        mpObjectPool = rhs.mpObjectPool;
        setDebugValue();
        return *this;
    }

    template <typename U>
    [[nodiscard]] bool IsAssignableFrom(const ObjectHandle<U>& rhs) const
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<U>>, "U is not Object derived class.");
        HO_ASSERT(mpObjectType, "Handle was construct incorrectly.");
        setDebugValue();
        return mpObjectType->IsBaseOfRecursive(*rhs.mpObjectType);
    }

    template <typename U>
    [[nodiscard]] ObjectHandle<U> Cast() const
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<U>>, "U is not Object derived class.");
        HO_ASSERT(IsValid(), "Attempted to cast an invalid/destroyed object handle.");
        HO_ASSERT(ReflectionSystem::GetInstance().GetClassByType<U>().IsBaseOfRecursive(*mpObjectType),
                  "U must be ancestor class of actual object type.");
        setDebugValue();
        return ObjectHandle<U>(mpObjectType, mID, mpObjectPool);
    }

    [[nodiscard]] T* operator->() const
    {
        HO_ASSERT(mpObjectType, "Handle was construct incorrectly.");
        return Get();
    }

    [[nodiscard]] T& operator*() const
    {
        HO_ASSERT(mpObjectType, "Handle was construct incorrectly.");
        return *Get();
    }

    template <typename U>
    [[nodiscard]] bool operator==(const ObjectHandle<U>& rhs) const
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<U>>, "U is not Object derived class.");
        HO_ASSERT(mpObjectType, "Handle was construct incorrectly.");
        setDebugValue();
        return mID == rhs.mID && mpObjectPool == rhs.mpObjectPool;
    }

    template <typename U>
    [[nodiscard]] bool operator!=(const ObjectHandle<U>& rhs) const
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<U>>, "U is not Object derived class.");
        HO_ASSERT(mpObjectType, "Handle was construct incorrectly.");
        setDebugValue();
        return !(*this == rhs);
    }

    [[nodiscard]] bool IsNULL() const
    {
        HO_ASSERT(mpObjectType, "Handle was construct incorrectly.");
        setDebugValue();
        return mpObjectPool == nullptr ? true : !mpObjectPool->Has(mID);
    }

    [[nodiscard]] bool IsValid() const
    {
        HO_ASSERT(mpObjectType, "Handle was construct incorrectly.");
        setDebugValue();
        return !IsNULL();
    }

    [[nodiscard]] T* Get() const
    {
        HO_ASSERT(mpObjectType, "Handle was construct incorrectly.");
        HO_ASSERT(IsValid(), "Attempted to access an invalid/destroyed object handle.");
        setDebugValue();
        return static_cast<T*>(mpObjectPool->Get(mID));
    }

    template <typename U>
    [[nodiscard]] U* GetAs() const
    {
        HO_ASSERT(mpObjectType, "Handle was construct incorrectly.");
        HO_ASSERT(IsValid(), "Attempted to access an invalid/destroyed object handle.");

        static_assert(std::is_base_of_v<RObject, std::remove_const_t<U>>, "U is not Object derived class.");

        HO_ASSERT(ReflectionSystem::GetInstance().GetClassByType<U>().IsBaseOfRecursive(*mpObjectType),

                  "U must be ancestor class of actual object type.");
        setDebugValue();
        return static_cast<U*>(Get());
    }

    [[nodiscard]] const Class& GetObjectType() const
    {
        HO_ASSERT(mpObjectType, "Handle was construct incorrectly.");
        setDebugValue();
        return *mpObjectType;
    }

    [[nodiscard]] constexpr uint64_t GetID() const
    {
        setDebugValue();
        return mID;
    }

    [[nodiscard]] const ObjectPool& GetPool() const
    {
        HO_ASSERT(mpObjectPool, "There is no bound pool.");
        setDebugValue();
        return *mpObjectPool;
    }

    static ObjectHandle<T> GetNULL()
    {
        return ObjectHandle<T>(&ReflectionSystem::GetInstance().GetClassByType<T>(), 0, nullptr);
    }

  private:
    ObjectHandle(const Class* pObjectType, uint64_t id, ObjectPool* pObjectPool)
      : mpObjectType(pObjectType)
      , mID(id)
      , mpObjectPool(pObjectPool)
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<T>>, "T is not Object derived class.");
        HO_ASSERT(ReflectionSystem::GetInstance().GetClassByType<T>().IsBaseOfRecursive(*mpObjectType),
                  "T must be ancestor class of actual object type.");
        setDebugValue();
    }

    void setDebugValue() const
    {
#if !defined(NDEBUG)
        mpDebugValue = mpObjectPool ? static_cast<T*>(mpObjectPool->Get(mID)) : nullptr;
#endif
    }

#if !defined(NDEBUG)
    mutable T* mpDebugValue = nullptr;
#endif

    const Class* mpObjectType;
    uint64_t mID = 0;
    ObjectPool* mpObjectPool = nullptr;
};
} // namespace ho

namespace std
{
template <typename T>
struct hash<ho::ObjectHandle<T>>
{
    size_t operator()(const ho::ObjectHandle<T>& k) const noexcept
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