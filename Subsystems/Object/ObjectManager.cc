#include "ObjectManager.h"

namespace ho
{
ObjectManager::ObjectManager()
  : mpObjectPools(ReflectionSystem::GetInstance().GetRegisteredTypeCount() + 1) // 0 used as null.
{
}

ObjectManager::~ObjectManager()
{
    for (auto& pObjPool : mpObjectPools)
    {
        delete pObjPool;
    }
}

void ObjectManager::DeleteObject(ObjectHandle<RObject> hObj)
{
    if (hObj.IsNULL())
    {
        return;
    }

    HO_ASSERT(mpObjectPools[static_cast<int32_t>(hObj.mpObjectPool->GetObjectType().GetTypeID())],
              "This type object never been created once in this manager.");
    HO_ASSERT(hObj.mpObjectPool == mpObjectPools[static_cast<int32_t>(hObj.mpObjectPool->GetObjectType().GetTypeID())],
              "Handle was not created from this manager.");
    hObj.mpObjectPool->Delete(hObj.mID);
}

bool ObjectManager::Has(ObjectHandle<RObject> hObj) const
{
    if (hObj.IsNULL())
    {
        return false;
    }

    const Class& objType = hObj.mpObjectPool->GetObjectType();
    if (mpObjectPools[static_cast<int32_t>(objType.GetTypeID())] != hObj.mpObjectPool)
    {
        return false;
    }
    return mpObjectPools[static_cast<int32_t>(objType.GetTypeID())]->Has(hObj.mID);
}

bool ObjectManager::IsEmpty() const
{
    for (auto& pObjPool : mpObjectPools)
    {
        if (pObjPool)
        {
            if (!pObjPool->IsEmpty())
            {
                return false;
            }
        }
    }
    return true;
}

void ObjectManager::ExpandAll()
{
    for (ObjectPool* pObjPool : mpObjectPools)
    {
        if (pObjPool)
        {
            pObjPool->Expand();
        }
    }
}

} // namespace ho