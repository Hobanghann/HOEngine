#pragma once

#include "Core/String/StringHandle.h"
#include "Core/Templates/FixedArray.h"
#include "Scene/Components/TransformComponent.h"
#include "Subsystems/Object/Object.h"
#include "Subsystems/Object/ObjectHandle.h"
#include "Subsystems/Reflection/ReflectionSystem.h"

namespace ho
{
// GameObjects have a Transform component by default.
// Therefore, component setter, getter, and find functions handle all components except the Transform component,
// which must be retrieved via 'GetComponent<RTransformComponent>'.
class RGameObject : public RObject
{
    HO_INJECT_REFLECTION_CODE(RGameObject)
    friend class RScene;

  public:
    RGameObject();
    ~RGameObject() override;

    virtual void OnCreated();
    virtual void OnPendingDestroyed();

    virtual void OnDestroyed();

    ObjectHandle<RScene> GetOwner();

    ObjectHandle<const RScene> GetOwner() const;

    ObjectHandle<RGameObject> GetHandle();

    ObjectHandle<const RGameObject> GetHandle() const;

    StringHandle GetName() const;

    template <typename TComponent>
    bool AttachComponent(ObjectHandle<TComponent> hComponent);

    template <typename TComponent>
    bool DetachComponent(ObjectHandle<TComponent> hComponent);

    template <typename TComponent>
    [[nodiscard]] ObjectHandle<TComponent> GetComponent();

    template <typename TComponent>
    [[nodiscard]] ObjectHandle<const TComponent> GetComponent() const;

    template <typename TComponent>
    bool HasComponent() const;

    const FixedArray<ObjectHandle<RComponent>>& GetComponents() const;

    bool IsDestroyed() const;

    bool IsActive() const;

    bool IsPendingDestroyed() const;

    void SetActive(bool bActive);

    bool IsVisibleInEditor() const;

    void SetVisibleInEditor(bool bVisibleInEditor);

    bool IsOpenedInEditor() const;

    void SetOpenedInEditor(bool bOpenedInEditor);

    bool IsFilteredInEditor() const;

    void SetFilteredInEditor(bool bFilteredInEditor);

    bool IsClickableInEditor() const;

    void SetClickableInEditor(bool bClickableInEditor);

    bool IsSelectedInEditor() const;

    void SetSelectedInEditor(bool bSelectedInEditor);

  protected:
    static const int32_t sMinComponentBufCount = 4;

    ObjectHandle<RScene> mhOwner;
    StringHandle mhName;

    int32_t mComponentCount = 0;
    FixedArray<ObjectHandle<RComponent>> mhComponents = FixedArray<ObjectHandle<RComponent>>(sMinComponentBufCount);

    bool mbDestroyed = false;

    // Determines if this object is updated in the scene loop.
    bool mbActive = true;

    bool mbPendingDestroyed = false;

    // NOTE: 'mbVisibleInEditor' and 'mbFilteredInEditor' conceptually belong to
    // the Engine Application (Editor) layer rather than the core Scene hierarchy.
    // However, to avoid the complexity of synchronizing state between the Editor
    // and Scene, they are naively placed here for simplicity.

    // Determines if this object is visible in the Editor Scene View.
    bool mbVisibleInEditor = true;

    bool mbOpenedInEditor = false;

    // Indicates whether this object passed the Editor search filter.
    bool mbFilteredInEditor = true;

    bool mbClickableInEditor = true;

    bool mbSelectedInEditor = false;

  private:
    void initialize(ObjectHandle<RScene> hOwner,
                    StringHandle hName,
                    ObjectHandle<RGameObject> hThis,
                    ObjectHandle<RTransformComponent> hTransformComponent);

    void setName(StringHandle hName);

    void tryExpandComponentBuffer();
    void tryShrinkComponentBuffer();

    ObjectHandle<RGameObject> mhThis;
};

template <typename TComponent>
bool RGameObject::AttachComponent(ObjectHandle<TComponent> hComponent)
{
    static_assert(std::is_base_of_v<RComponent, TComponent>, "TComponent is not component class.");
    HO_ASSERT(hComponent.IsValid(), "Component is invalid.");

    for (int32_t i = 0; i < mComponentCount; ++i)
    {
        if (mhComponents[i]->GetClass() == hComponent->GetClass())
        {
            return false;
        }
    }

    tryExpandComponentBuffer();

    mhComponents[mComponentCount] = hComponent;
    ++mComponentCount;
    hComponent->OnAttached(mhThis);
    return true;
}

template <typename TComponent>
bool RGameObject::DetachComponent(ObjectHandle<TComponent> hComponent)
{
    static_assert(std::is_base_of_v<RComponent, TComponent>, "TComponent is not component class.");
    static_assert(!std::is_base_of_v<RTransformComponent, TComponent>, "TransformComponent attached defaultly.");

    int32_t detachIdx = -1;
    for (int32_t i = 0; i < mComponentCount; ++i)
    {
        if (mhComponents[i] == hComponent)
        {
            detachIdx = i;
            break;
        }
    }

    if (detachIdx == -1)
    {
        return false;
    }

    mhComponents[detachIdx]->OnDetached();
    --mComponentCount;

    for (int32_t i = detachIdx; i < mComponentCount; ++i)
    {
        mhComponents[i] = mhComponents[i + 1];
    }

    for (int32_t i = mComponentCount; i < mhComponents.GetSize(); ++i)
    {
        mhComponents[i] = ObjectHandle<RComponent>::sNULL;
    }

    tryShrinkComponentBuffer();

    return true;
}

template <typename TComponent>
ObjectHandle<TComponent> RGameObject::GetComponent()
{
    static_assert(std::is_base_of_v<RComponent, std::remove_const_t<TComponent>>, "TComponent is not component class.");

    const Class& targetClass = ReflectionSystem::GetInstance().GetClassByType<TComponent>();
    for (int32_t i = 0; i < mComponentCount; ++i)
    {
        if (mhComponents[i]->GetClass() == targetClass)
        {
            return mhComponents[i].Cast<TComponent>();
        }
    }

    return ObjectHandle<TComponent>::sNULL;
}

template <typename TComponent>
ObjectHandle<const TComponent> RGameObject::GetComponent() const
{
    static_assert(std::is_base_of_v<RComponent, std::remove_const_t<TComponent>>, "TComponent is not component class.");

    const Class& targetClass = ReflectionSystem::GetInstance().GetClassByType<TComponent>();
    for (int32_t i = 0; i < mComponentCount; ++i)
    {
        if (mhComponents[i]->GetClass() == targetClass)
        {
            return mhComponents[i].Cast<TComponent>();
        }
    }
    return ObjectHandle<TComponent>::sNULL;
}

template <typename TComponent>
bool RGameObject::HasComponent() const
{
    static_assert(std::is_base_of_v<RComponent, TComponent>, "TComponent is not component class.");
    const Class& targetClass = ReflectionSystem::GetInstance().GetClassByType<TComponent>();
    for (int32_t i = 0; i < mComponentCount; ++i)
    {
        if (mhComponents[i]->GetClass() == targetClass)
        {
            return true;
        }
    }

    return false;
}

} // namespace ho
