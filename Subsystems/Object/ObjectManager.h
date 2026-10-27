#pragma once

#include "Core/Templates/FixedArray.h"
#include "ObjectContainer.h"
#include "ObjectHandle.h"
#include "Subsystems/Reflection/ReflectionSystem.h"

namespace ho
{
class ObjectSystem;
class ObjectPool;

class ObjectManager final
{
    friend ObjectSystem;

  public:
    ObjectManager();
    ~ObjectManager();

    ObjectManager(const ObjectManager&) = delete;
    ObjectManager& operator=(const ObjectManager&) = delete;

    template <typename T>
    ObjectHandle<T> NewObject();

    void DeleteObject(ObjectHandle<RObject> hObj);

    bool Has(ObjectHandle<RObject> hObj) const;

    template <typename T>
    ObjectContainer<T> GetObjects() const;

    template <typename T>
    ObjectContainer<T> GetObjectsExact() const;

    bool IsEmpty() const;

    template <typename T>
    int32_t GetObjectCount() const;

    template <typename T>
    int32_t GetObjectCountExact() const;

    template <typename T>
    void Expand(int32_t newSize);

    void ExpandAll();

  private:
    int32_t getObjectCountRecursive(const Class* pObjType) const;

    FixedArray<ObjectPool*> mpObjectPools;
};

template <typename T>
ObjectHandle<T> ObjectManager::NewObject()
{
    static_assert(std::is_base_of_v<RObject, std::remove_const_t<T>>, "T is not Object drived class.");
    const Class& objType = ReflectionSystem::GetInstance().GetClassByType<T>();

    if (!mpObjectPools[static_cast<int32_t>(objType.GetTypeID())])
    {
        mpObjectPools[static_cast<int32_t>(objType.GetTypeID())] = new ObjectPool(&objType, 8);
    }

    uint64_t newObjID = mpObjectPools[static_cast<int32_t>(objType.GetTypeID())]->New();
    return ObjectHandle<T>(newObjID, mpObjectPools[static_cast<int32_t>(objType.GetTypeID())]);
}

template <typename T>
ObjectContainer<T> ObjectManager::GetObjects() const
{
    static_assert(std::is_base_of_v<RObject, std::remove_const_t<T>>, "T is not Object drived class.");
    const Class& objType = ReflectionSystem::GetInstance().GetClassByType<T>();
    return ObjectContainer<T>(&objType, &mpObjectPools, false);
}

template <typename T>
ObjectContainer<T> ObjectManager::GetObjectsExact() const
{
    static_assert(std::is_base_of_v<RObject, std::remove_const_t<T>>, "T is not Object drived class.");
    const Class& objType = ReflectionSystem::GetInstance().GetClassByType<T>();
    return ObjectContainer<T>(&objType, &mpObjectPools, true);
}

template <typename T>
int32_t ObjectManager::GetObjectCount() const
{
    static_assert(std::is_base_of_v<RObject, std::remove_const_t<T>>, "T is not Object drived class.");
    const Class& objType = ReflectionSystem::GetInstance().GetClassByType<T>();

    return getObjectCountRecursive(&objType);
}

template <typename T>
int32_t ObjectManager::GetObjectCountExact() const
{
    static_assert(std::is_base_of_v<RObject, std::remove_const_t<T>>, "T is not Object drived class.");
    const Class& objType = ReflectionSystem::GetInstance().GetClassByType<T>();

    if (mpObjectPools[static_cast<int32_t>(objType.GetTypeID())])
    {
        return getObjectCountRecursive(&objType);
    }
    return 0;
}

template <typename T>
void ObjectManager::Expand(int32_t newSize)
{
    static_assert(std::is_base_of_v<RObject, std::remove_const_t<T>>, "T is not Object drived class.");
    const Class& objType = ReflectionSystem::GetInstance().GetClassByType<T>();

    if (mpObjectPools[static_cast<int32_t>(objType.GetTypeID())])
    {
        mpObjectPools[static_cast<int32_t>(objType.GetTypeID())]->Expand(newSize);
    }
    else
    {
        mpObjectPools[static_cast<int32_t>(objType.GetTypeID())] = new ObjectPool(&objType, newSize);
    }
}

class ObjectManager;
using ObjectManagerHandle = WeakTypedHandle<ObjectManager, TypedObjectPool>;
} // namespace ho