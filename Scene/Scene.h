#pragma once

#include <unordered_map>

#include "Core/Templates/FixedQueue.h"
#include "Scene/Components/Components.h"
#include "Scene/GameObjects/GameObject.h"
#include "Subsystems/Object/ObjectManager.h"

namespace ho
{
class RScene final : public RObject
{
    friend class SceneManager;

    HO_INJECT_REFLECTION_CODE(RScene)

  public:
    RScene();
    ~RScene() override;

    void OnCreated();

    void OnUpdate(float deltaTime);

    void OnPendingDestroyed();
    void OnDestroyed();

    StringHandle GetName() const
    {
        return mhName;
    }

    template <typename TGameObject>
    [[nodiscard]] ObjectHandle<TGameObject> CreateGameObject(StringHandle hName);

    template <typename TGameObject>
    void DestroyGameObject(ObjectHandle<TGameObject> hObj);

    template <typename TGameObject>
    void DestroyGameObjectAndDescendantRecursive(ObjectHandle<TGameObject> hObj);

    template <typename TGameObject>
    void RenameGameObject(ObjectHandle<TGameObject> hObj, StringHandle hName);

    template <typename TComponent>
    [[nodiscard]] ObjectHandle<TComponent> CreateComponent();

    template <typename TComponent>
    void DestroyComponent(ObjectHandle<TComponent> hComp);

    [[nodiscard]] ObjectHandle<RGameObject> GetGameObjectByName(StringHandle hName);

    [[nodiscard]] ObjectHandle<const RGameObject> GetGameObjectByName(StringHandle hName) const;

    [[nodiscard]] bool HasGameObject(StringHandle hName) const;

    template <typename TGameObject>
    ObjectContainer<TGameObject> GetGameObjects();

    template <typename TGameObject>
    ObjectContainer<TGameObject> GetGameObjectsExact();

    template <typename TGameObject>
    ObjectContainer<const TGameObject> GetGameObjects() const;

    template <typename TGameObject>
    ObjectContainer<const TGameObject> GetGameObjectsExact() const;

    template <typename TGameObject>
    int32_t GetGameObjectCount() const;

    template <typename TGameObject>
    int32_t GetGameObjectCountExact() const;

    template <typename TComponent>
    ObjectContainer<TComponent> GetComponents();

    template <typename TComponent>
    ObjectContainer<TComponent> GetComponentsExact();

    template <typename TComponent>
    ObjectContainer<const TComponent> GetComponents() const;

    template <typename TComponent>
    ObjectContainer<const TComponent> GetComponentsExact() const;

    template <typename TComponent>
    int32_t GetComponentCount() const;

    template <typename TComponent>
    int32_t GetComponentCountExact() const;

    ObjectHandle<RCameraComponent> GetMainCamera()
    {
        return mhMainCamera;
    }

    ObjectHandle<RSkyBoxComponent> GetMainSkyBox()
    {
        return mhMainSkybox;
    }

    ObjectHandle<const RCameraComponent> GetMainCamera() const
    {
        return mhMainCamera;
    }

    ObjectHandle<const RSkyBoxComponent> GetMainSkyBox() const
    {
        return mhMainSkybox;
    }

    void SetMainCamera(ObjectHandle<RCameraComponent> hComp)
    {
        mhMainCamera = hComp;
    }

    void SetMainSkyBox(ObjectHandle<RSkyBoxComponent> hComp)
    {
        mhMainSkybox = hComp;
    }

    void CollectGarbage();

    bool IsActive() const
    {
        return mbActive;
    }

    void SetActive(bool bActive)
    {
        mbActive = bActive;
    }

    bool IsPaused() const
    {
        return mbPaused;
    }

    void SetPaused(bool bPaused)
    {
        mbPaused = bPaused;
    }

    bool IsDestroyed() const
    {
        return mbDestroyed;
    }

    bool IsPendingDestroyed() const
    {
        return mbPendingDestroyed;
    }

  private:
    static const int32_t sDestroyedGameObjectQueueSize = 1024;
    static const int32_t sDestroyedComponentQueueSize = 1024;

    void initialize(SceneManager* pOwner, StringHandle hName, ObjectHandle<RScene> hThis);

    SceneManager* mpOwner = nullptr;
    StringHandle mhName;

    ObjectManagerHandle mhObjectPool;
    ObjectHandle<RCameraComponent> mhMainCamera;
    ObjectHandle<RSkyBoxComponent> mhMainSkybox;

    std::unordered_map<StringHandle, ObjectHandle<RGameObject>> mNameToGameObjectMap;

    FixedQueue<ObjectHandle<RGameObject>> mhDestroyedGameObjectQueue;
    FixedQueue<ObjectHandle<RComponent>> mhDestroyedComponentQueue;

    bool mbActive = true;
    bool mbPaused = false;
    bool mbDestroyed = false;
    bool mbPendingDestroyed = false;

    ObjectHandle<RScene> mhThis;
};

template <typename TGameObject>
ObjectHandle<TGameObject> RScene::CreateGameObject(StringHandle hName)
{
    static_assert(std::is_base_of_v<RGameObject, TGameObject>, "TGameObject is not game object class.");
    HO_ASSERT(hName.IsValid() && !hName.IsEmpty(), "Name is invalid");

    const auto it = mNameToGameObjectMap.find(hName);
    if (it != mNameToGameObjectMap.end())
    {
        HO_ASSERT(false, "There is game object has same name.");
        return ObjectHandle<TGameObject>::sNULL;
    }

    ObjectHandle<TGameObject> hNewObj = mhObjectPool->NewObject<TGameObject>();
    ObjectHandle<RTransformComponent> hNewTransComp = CreateComponent<RTransformComponent>();

    hNewObj->initialize(mhThis, hName, hNewObj, hNewTransComp);
    hNewObj->OnCreated();

    mNameToGameObjectMap[hName] = hNewObj;

    return hNewObj;
}

template <typename TGameObject>
void RScene::DestroyGameObject(ObjectHandle<TGameObject> hObj)
{
    HO_ASSERT(hObj.IsValid(), "Game object never be created or already deleted.");
    if (hObj->IsPendingDestroyed())
    {
        return;
    }

    for (auto& hComp : hObj->GetComponents())
    {
        if (hComp.IsValid())
        {
            DestroyComponent(hComp);
        }
    }

    hObj->OnPendingDestroyed();

    HO_ASSERT(!mhDestroyedGameObjectQueue.IsFull(),
              "Queue stored destoryed game object is full!. Memory leak will occur.");
    mhDestroyedGameObjectQueue.Push(hObj);
}

template <typename TGameObject>
void RScene::DestroyGameObjectAndDescendantRecursive(ObjectHandle<TGameObject> hObj)
{
    ObjectHandle<RTransformComponent> hCurrChild = hObj->template GetComponent<RTransformComponent>()->GetFirstChild();
    while (hCurrChild.IsValid())
    {
        DestroyGameObjectAndDescendantRecursive(hCurrChild->GetOwner()->GetHandle());
        hCurrChild = hCurrChild->GetNextSibling();
    }
    DestroyGameObject(hObj);
}

template <typename TGameObject>
void RScene::RenameGameObject(ObjectHandle<TGameObject> hObj, StringHandle hName)
{
    HO_ASSERT(hObj.IsValid(), "Game object never be created or already deleted.");
    HO_ASSERT(hName.IsValid() && !hName.IsEmpty(), "Name is invalid");
    const auto it = mNameToGameObjectMap.find(hName);
    if (it != mNameToGameObjectMap.end())
    {
        return;
    }
    hObj->setName(hName);
    mNameToGameObjectMap.erase(hObj->GetName());
    mNameToGameObjectMap[hName] = hObj;
}

template <typename TComponent>
ObjectHandle<TComponent> RScene::CreateComponent()
{
    static_assert(std::is_base_of_v<RComponent, TComponent>, "TComponent is not component class.");

    ObjectHandle<TComponent> hNewComp = mhObjectPool->NewObject<TComponent>();

    hNewComp->initialize(hNewComp);
    hNewComp->OnCreated();

    return hNewComp;
}

template <typename TComponent>
void RScene::DestroyComponent(ObjectHandle<TComponent> hComp)
{
    HO_ASSERT(hComp.IsValid(), "Component never be created or already deleted.");
    if (hComp->IsPendingDestroyed())
    {
        return;
    }

    hComp->OnPendingDestroyed();

    HO_ASSERT(!mhDestroyedComponentQueue.IsFull(),
              "Queue stored destoryed component is full!. Memory leak will occur.");
    mhDestroyedComponentQueue.Push(hComp);
}

template <typename TGameObject>
ObjectContainer<TGameObject> RScene::GetGameObjects()
{
    return mhObjectPool->GetObjects<TGameObject>();
}

template <typename TGameObject>
ObjectContainer<TGameObject> RScene::GetGameObjectsExact()
{
    return mhObjectPool->GetObjectsExact<TGameObject>();
}

template <typename TGameObject>
ObjectContainer<const TGameObject> RScene::GetGameObjects() const
{
    return mhObjectPool->GetObjects<const TGameObject>();
}

template <typename TGameObject>
ObjectContainer<const TGameObject> RScene::GetGameObjectsExact() const
{
    return mhObjectPool->GetObjectsExact<const TGameObject>();
}

template <typename TGameObject>
int32_t RScene::GetGameObjectCount() const
{
    int32_t count = 0;
    for (const auto& hObj : GetGameObjects<TGameObject>())
    {
        ++count;
    }
    return count;
}

template <typename TGameObject>
int32_t RScene::GetGameObjectCountExact() const
{
    int32_t count = 0;
    for (const auto& hObj : GetGameObjectsExact<TGameObject>())
    {
        ++count;
    }
    return count;
}

template <typename TComponent>
ObjectContainer<TComponent> RScene::GetComponents()
{
    return mhObjectPool->GetObjects<TComponent>();
}

template <typename TComponent>
ObjectContainer<TComponent> RScene::GetComponentsExact()
{
    return mhObjectPool->GetObjectsExact<TComponent>();
}

template <typename TComponent>
ObjectContainer<const TComponent> RScene::GetComponents() const
{
    return mhObjectPool->GetObjects<const TComponent>();
}

template <typename TComponent>
ObjectContainer<const TComponent> RScene::GetComponentsExact() const
{
    return mhObjectPool->GetObjectsExact<const TComponent>();
}

template <typename TComponent>
int32_t RScene::GetComponentCount() const
{
    int32_t count = 0;
    for (const auto& hComp : GetComponents<TComponent>())
    {
        ++count;
    }
    return count;
}

template <typename TComponent>
int32_t RScene::GetComponentCountExact() const
{
    int32_t count = 0;
    for (const auto& hComp : GetComponentsExact<TComponent>())
    {
        ++count;
    }
    return count;
}

} // namespace ho