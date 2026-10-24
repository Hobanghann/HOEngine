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
      : mID(0)
      , mpObjectPool(nullptr)
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<T>>, "T is must be Object derived class.");
        setDebugValue();
    }

    template <typename TConvertible>
    ObjectHandle(const ObjectHandle<TConvertible>& other)
      : mID(other.mID)
      , mpObjectPool(other.mpObjectPool)
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<TConvertible>>,
                      "TConvertible must be an Object-derived class.");

        HO_ASSERT(ReflectionSystem::GetInstance().GetClassByType<T>().IsBaseOfRecursive(other.GetActualObjectType()),
                  "T must be an ancestor of the actual object type.");

        setDebugValue();
    }

    template <typename TConvertible>
    ObjectHandle& operator=(const ObjectHandle<TConvertible>& rhs)
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<TConvertible>>,
                      "TConvertible must be an Object-derived class.");

        HO_ASSERT(ReflectionSystem::GetInstance().GetClassByType<T>().IsBaseOfRecursive(rhs.GetActualObjectType()),
                  "T must be an ancestor of the actual object type.");

        mID = rhs.mID;
        mpObjectPool = rhs.mpObjectPool;
        setDebugValue();
        return *this;
    }

    template <typename TAssignable>
    [[nodiscard]] bool IsAssignableFrom(const ObjectHandle<TAssignable>& rhs) const
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<TAssignable>>,
                      "TAssignable is not Object derived class.");
        setDebugValue();
        return GetActualObjectType() == rhs.GetActualObjectType();
    }

    template <typename U>
    [[nodiscard]] ObjectHandle<U> Cast() const
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<U>>, "U is not Object derived class.");
        HO_ASSERT(ReflectionSystem::GetInstance().GetClassByType<U>().IsBaseOfRecursive(GetActualObjectType()),
                  "U must be ancestor class of actual object type.");
        setDebugValue();
        return ObjectHandle<U>(mID, mpObjectPool);
    }

    [[nodiscard]] T* operator->() const
    {
        return Get();
    }

    [[nodiscard]] T& operator*() const
    {
        return *Get();
    }

    template <typename U>
    [[nodiscard]] bool operator==(const ObjectHandle<U>& rhs) const
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<U>>, "U is not Object derived class.");
        setDebugValue();
        return mID == rhs.mID && mpObjectPool == rhs.mpObjectPool;
    }

    template <typename U>
    [[nodiscard]] bool operator!=(const ObjectHandle<U>& rhs) const
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<U>>, "U is not Object derived class.");
        setDebugValue();
        return !(*this == rhs);
    }

    [[nodiscard]] bool IsNULL() const
    {
        setDebugValue();
        return mpObjectPool ? !mpObjectPool->Has(mID) : true;
    }

    [[nodiscard]] bool IsValid() const
    {
        setDebugValue();
        return !IsNULL();
    }

    [[nodiscard]] T* Get() const
    {
        HO_ASSERT(IsValid(), "Attempted to access an invalid/destroyed object handle.");
        setDebugValue();
        return static_cast<T*>(mpObjectPool->Get(mID));
    }

    template <typename U>
    [[nodiscard]] U* GetAs() const
    {
        HO_ASSERT(IsValid(), "Attempted to access an invalid/destroyed object handle.");
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<U>>, "U is not Object derived class.");

        HO_ASSERT(ReflectionSystem::GetInstance().GetClassByType<U>().IsBaseOfRecursive(GetActualObjectType()),
                  "U must be ancestor class of actual object type.");
        setDebugValue();
        return static_cast<U*>(Get());
    }

    [[nodiscard]] const Class& GetActualObjectType() const
    {
        setDebugValue();
        return mpObjectPool ? mpObjectPool->GetObjectType() : ReflectionSystem::GetInstance().GetClassByType<T>();
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

    static const ObjectHandle<T> sNULL;

  private:
    ObjectHandle(uint64_t id, ObjectPool* pObjectPool)
      : mID(id)
      , mpObjectPool(pObjectPool)
    {
        static_assert(std::is_base_of_v<RObject, std::remove_const_t<T>>, "T is not Object derived class.");
        HO_ASSERT(ReflectionSystem::GetInstance().GetClassByType<T>().IsBaseOfRecursive(GetActualObjectType()),
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

    uint64_t mID = 0;
    ObjectPool* mpObjectPool = nullptr;
};

template <typename T>
INLINE const ObjectHandle<T> ObjectHandle<T>::sNULL = ObjectHandle<T>();
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