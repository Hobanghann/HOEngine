#include "SceneManager.h"

#include "Subsystems/Object/ObjectSystem.h"

namespace ho
{
SceneManager& SceneManager::GetInstance()
{
    HO_ASSERT(spInstance, "scene manager is not created");
    return *spInstance;
}

ObjectHandle<RScene> SceneManager::CreateScene(StringHandle hName)
{
    if (mNameToSceneMap.find(hName) != mNameToSceneMap.end())
    {
        return ObjectHandle<RScene>::sNULL;
    }
    ObjectHandle<RScene> hNewScene = mhScenePool->NewObject<RScene>();

    hNewScene->initialize(this, hName, hNewScene);
    hNewScene->OnCreated();

    mNameToSceneMap[hName] = hNewScene;
    return hNewScene;
}

void SceneManager::DestroyScene(ObjectHandle<RScene> hScene)
{
    if (hScene.IsNULL())
    {
        return;
    }

    hScene->OnPendingDestroyed();

    HO_ASSERT(!mpDestroyedSceneQueue.IsFull(), "Destroyed scene queue is full! Memory leak will occur.");
    mpDestroyedSceneQueue.Push(hScene);
}

bool SceneManager::HasScene(StringHandle hName) const
{
    if (hName.IsNULL())
    {
        return false;
    }
    return mNameToSceneMap.find(hName) != mNameToSceneMap.end();
}

ObjectHandle<RScene> SceneManager::GetSceneByName(StringHandle hName)
{
    auto it = mNameToSceneMap.find(hName);
    if (it == mNameToSceneMap.end())
    {
        return ObjectHandle<RScene>::sNULL;
    }
    return it->second;
}

ObjectHandle<const RScene> SceneManager::GetSceneByName(StringHandle hName) const
{
    auto it = mNameToSceneMap.find(hName);
    if (it == mNameToSceneMap.end())
    {
        return ObjectHandle<RScene>::sNULL;
    }
    return it->second;
}

ObjectContainer<RScene> SceneManager::GetScenes()
{
    return mhScenePool->GetObjects<RScene>();
}

ObjectContainer<const RScene> SceneManager::GetScenes() const
{
    return mhScenePool->GetObjects<const RScene>();
}

int32_t SceneManager::GetSceneCount() const
{
    return mhScenePool->GetObjectCount<RScene>();
}

SceneManager::SceneManager()
  : mhScenePool(ObjectSystem::GetInstance().CreateObjectManager())
{
    HO_ASSERT(!spInstance, "Scene Manager must be created by 'createInstance' function.");
}

SceneManager::~SceneManager()
{
    HO_ASSERT(mhScenePool->IsEmpty(), "Every scenes must be destroyed explicitly before scene manager destroyed.");
    ObjectSystem::GetInstance().DestroyObjectManager(mhScenePool);
}

void SceneManager::createInstance()
{
    if (spInstance)
    {
        HO_ASSERT(false, "Scene Manager is already created.");
    }
    else
    {
        spInstance = new SceneManager();
    }
    spInstance->mpDestroyedSceneQueue = FixedQueue<ObjectHandle<RScene>>(sDestroyedSceneQueueSize);
}

void SceneManager::deleteInstance()
{
    if (!spInstance)
    {
        HO_ASSERT(false, "Scene Manager is not created.");
    }
    else
    {
        delete spInstance;
        spInstance = nullptr;
    }
}

bool SceneManager::init()
{
    if (!spInstance)
    {
        HO_ASSERT(false, "Scene Manager is not created.");
        return false;
    }
    else
    {
        return true;
    }
}

void SceneManager::shutdown()
{
    for (const ObjectHandle<RScene> hScene : mhScenePool->GetObjects<RScene>())
    {
        if (!hScene->IsPendingDestroyed())
        {
            DestroyScene(hScene);
        }
    }

    deleteDestroyedScenes();
}

void SceneManager::updateScenes(float deltaTime)
{
    for (const ObjectHandle<RScene> hScene : mhScenePool->GetObjects<RScene>())
    {
        if (hScene->IsActive() && !hScene->IsPaused())
        {
            hScene->OnUpdate(deltaTime);
        }
    }
}

void SceneManager::collectGarbage()
{
    for (const ObjectHandle<RScene> hScene : mhScenePool->GetObjects<RScene>())
    {
        hScene->CollectGarbage();
    }
}

void SceneManager::deleteDestroyedScenes()
{
    while (!mpDestroyedSceneQueue.IsEmpty())
    {
        const ObjectHandle<RScene> hScene = mpDestroyedSceneQueue.Front();

        HO_ASSERT(hScene.IsValid(), "Invalid scene.");
        HO_ASSERT(hScene->IsPendingDestroyed(),
                  "Pending destroy event is ommitted. OnPendingdestryed() must be called before actual destroy event.");

        hScene->CollectGarbage();
        hScene->OnDestroyed();

        mNameToSceneMap.erase(hScene->GetName());
        mhScenePool->DeleteObject(hScene);

        mpDestroyedSceneQueue.Pop();
    }
}

SceneManager* SceneManager::spInstance = nullptr;
} // namespace ho