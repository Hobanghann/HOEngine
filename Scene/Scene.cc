#include "Scene.h"

#include "Subsystems/Object/ObjectSystem.h"
#include "Subsystems/Reflection/ClassRegister.h"
#include "Subsystems/Reflection/ReflectionSystem.h"

namespace ho
{
RScene::RScene()
  : mhObjectPool(ObjectSystem::GetInstance().CreateObjectManager())
  , mhDestroyedGameObjectQueue(sDestroyedGameObjectQueueSize)
  , mhDestroyedComponentQueue(sDestroyedComponentQueueSize)
  , mhThis(ObjectHandle<RScene>::sNULL)
{
}

RScene::~RScene()
{
    HO_ASSERT(mhObjectPool->IsEmpty(),
              "Every game objects and components must be destroyed explicitly before scene destroyed.");
    ObjectSystem::GetInstance().DestroyObjectManager(mhObjectPool);
}

void RScene::OnCreated() {}

void RScene::OnUpdate(float deltaTime)
{
    HO_ASSERT(mbActive, "Inactive scene can't be updated.");

    for (const ObjectHandle<RComponent> hComp : mhObjectPool->GetObjects<RComponent>())
    {
        if (hComp->IsActive())
        {
            hComp->OnUpdate(deltaTime);
        }
    }
}

void RScene::OnPendingDestroyed()
{
    HO_ASSERT(!mbPendingDestroyed, "Component is already destroyed.");

    for (const ObjectHandle<RGameObject> hObj : mhObjectPool->GetObjects<RGameObject>())
    {
        DestroyGameObject(hObj);
    }
    for (const ObjectHandle<RComponent> hComp : mhObjectPool->GetObjects<RComponent>())
    {
        DestroyComponent(hComp);
    }
    mbPendingDestroyed = true;
}

void RScene::OnDestroyed()
{
    HO_ASSERT(mbPendingDestroyed, "Component must be pending destroyed before actually destroying.");
    HO_ASSERT(!mbDestroyed, "Component is already destroyed.");

    CollectGarbage();
    mbDestroyed = true;
}

ObjectHandle<RGameObject> RScene::GetGameObjectByName(StringHandle hName)
{
    auto it = mNameToGameObjectMap.find(hName);
    if (it == mNameToGameObjectMap.end())
    {
        return ObjectHandle<RGameObject>::sNULL;
    }
    return it->second;
}

ObjectHandle<const RGameObject> RScene::GetGameObjectByName(StringHandle hName) const
{
    auto it = mNameToGameObjectMap.find(hName);
    if (it == mNameToGameObjectMap.end())
    {
        return ObjectHandle<RGameObject>::sNULL;
    }
    return it->second;
}

bool RScene::HasGameObject(StringHandle hName) const
{
    auto it = mNameToGameObjectMap.find(hName);
    return it != mNameToGameObjectMap.end();
}

void RScene::CollectGarbage()
{
    while (!mhDestroyedComponentQueue.IsEmpty())
    {
        const ObjectHandle<RComponent> hDeletedComp = mhDestroyedComponentQueue.Front();
        HO_ASSERT(hDeletedComp->IsPendingDestroyed(),
                  "Pending destroy event is ommitted. OnPendingdestryed() must be called before actual destroy event.");

        if (hDeletedComp->IsAttached())
        {
            hDeletedComp->GetOwner()->DetachComponent(hDeletedComp);
        }

        hDeletedComp->OnDestroyed();

        mhObjectPool->DeleteObject(hDeletedComp);

        mhDestroyedComponentQueue.Pop();
    }

    while (!mhDestroyedGameObjectQueue.IsEmpty())
    {
        const ObjectHandle<RGameObject> hDeletedObj = mhDestroyedGameObjectQueue.Front();
        HO_ASSERT(hDeletedObj->IsPendingDestroyed(),
                  "Pending destroy event is ommitted. OnPendingdestryed() must be called before actual destroy event.");

        hDeletedObj->OnDestroyed();

        mNameToGameObjectMap.erase(hDeletedObj->GetName());
        mhObjectPool->DeleteObject(hDeletedObj);

        mhDestroyedGameObjectQueue.Pop();
    }
}

void RScene::initialize(SceneManager* pOwner, StringHandle hName, ObjectHandle<RScene> hThis)
{
    HO_ASSERT(pOwner, "Invalid scene manager.");
    mpOwner = pOwner;
    mhName = hName;
    mhThis = hThis;
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RScene, "Scene", RObject)
HO_REGISTER_CLASS_PROPERTY(mhName, "Name")
HO_REGISTER_CLASS_END()
} // namespace ho