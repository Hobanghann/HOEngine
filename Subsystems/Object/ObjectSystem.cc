#include "ObjectSystem.h"

#include "ObjectManager.h"
#include "Subsystems/Reflection/ReflectionSystem.h"

namespace ho
{
ObjectSystem& ObjectSystem::GetInstance()
{
    HO_ASSERT(spInstance, "Object system is not created");
    return *spInstance;
}

ObjectManagerHandle ObjectSystem::CreateObjectManager()
{
    const uint64_t newManagerID = mObjectManagerPool.Emplace();
    return ObjectManagerHandle(&mObjectManagerPool, newManagerID);
}

void ObjectSystem::DestroyObjectManager(ObjectManagerHandle hObjManager)
{
    mObjectManagerPool.Remove(hObjManager.GetID());
}

void ObjectSystem::registerObject(ObjectHandle<RObject> hObj)
{
    if (hObj.IsNULL())
    {
        HO_ASSERT(false, "Object handle is null.");
        return;
    }
    if (hObj->mGlobalID != 0)
    {
        HO_ASSERT(false, "Object is already registered.");
        return;
    }

    const uint64_t objGlobalID = mhObjects.Add(hObj);
    hObj->mGlobalID = objGlobalID;
}

void ObjectSystem::unregisterObject(uint64_t objID)
{
    mhObjects.Remove(objID);
}

void ObjectSystem::expandObjectManagers()
{
    for (ObjectManager& manager : mObjectManagerPool)
    {
        manager.ExpandAll();
    }
}

ObjectSystem::ObjectSystem()
{
    HO_ASSERT(!spInstance, "Object system muse be created by 'createInstance' function.");
}

ObjectSystem::~ObjectSystem() = default;

void ObjectSystem::createInstance()
{
    if (spInstance)
    {
        HO_ASSERT(false, "Object system is already created.");
    }
    else
    {
        spInstance = new ObjectSystem();
    }
}

void ObjectSystem::deleteInstance()
{
    if (!spInstance)
    {
        HO_ASSERT(false, "Object system is not created.");
    }
    else
    {
        delete spInstance;
        spInstance = nullptr;
    }
}

bool ObjectSystem::init()
{
    if (!spInstance)
    {
        HO_ASSERT(false, "Object system is not created.");
        return false;
    }
    else
    {
        return true;
    }
}

void ObjectSystem::shutdown() {}

ObjectSystem* ObjectSystem::spInstance = nullptr;
} // namespace ho