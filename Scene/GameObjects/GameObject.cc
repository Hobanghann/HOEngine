#include "GameObject.h"

#include "../Scene.h"
#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{
RGameObject::RGameObject() = default;
RGameObject::~RGameObject() = default;

void RGameObject::OnCreated() {}

void RGameObject::OnPendingDestroyed()
{
    HO_ASSERT(!mbPendingDestroyed, "Game object is already destroyed.");
    mbPendingDestroyed = true;
}

void RGameObject::OnDestroyed()
{
    HO_ASSERT(mbPendingDestroyed, "Game object must be pending destroyed before actually destroying.");
    HO_ASSERT(!mbDestroyed, "Game object is already destroyed.");
    mbDestroyed = true;
}

ObjectHandle<RScene> RGameObject::GetOwner()
{
    return mhOwner;
}

ObjectHandle<const RScene> RGameObject::GetOwner() const
{
    return mhOwner;
}

ObjectHandle<RGameObject> RGameObject::GetHandle()
{
    return mhThis;
}

ObjectHandle<const RGameObject> RGameObject::GetHandle() const
{
    return mhThis;
}

StringHandle RGameObject::GetName() const
{
    return mhName;
}

const FixedArray<ObjectHandle<RComponent>>& RGameObject::GetComponents() const
{
    return mhComponents;
}

bool RGameObject::IsDestroyed() const
{
    return mbDestroyed;
}

bool RGameObject::IsActive() const
{
    return mbActive;
}

bool RGameObject::IsPendingDestroyed() const
{
    return mbPendingDestroyed;
}

void RGameObject::SetActive(bool bActive)
{
    mbActive = bActive;
}

bool RGameObject::IsVisibleInEditor() const
{
    return mbVisibleInEditor;
}

void RGameObject::SetVisibleInEditor(bool bVisibleInEditor)
{
    mbVisibleInEditor = bVisibleInEditor;
}

bool RGameObject::IsFilteredInEditor() const
{
    return mbFilteredInEditor;
}

void RGameObject::SetFilteredInEditor(bool bFilteredInEditor)
{
    mbFilteredInEditor = bFilteredInEditor;
}

bool RGameObject::IsClickableInEditor() const
{
    return mbClickableInEditor;
}

void RGameObject::SetClickableInEditor(bool bClickableInEditor)
{
    mbClickableInEditor = bClickableInEditor;
}

bool RGameObject::IsOpenedInEditor() const
{
    return mbOpenedInEditor;
}

void RGameObject::SetOpenedInEditor(bool bOpenedInEditor)
{
    mbOpenedInEditor = bOpenedInEditor;
}

bool RGameObject::IsSelectedInEditor() const
{
    return mbSelectedInEditor;
}

void RGameObject::SetSelectedInEditor(bool bSelectedInEditor)
{
    mbSelectedInEditor = bSelectedInEditor;
}

void RGameObject::initialize(ObjectHandle<RScene> hOwner,
                             StringHandle hName,
                             ObjectHandle<RGameObject> hThis,
                             ObjectHandle<RTransformComponent> hTransformComponent)
{
    HO_ASSERT(hOwner.IsValid(), "Invalid scene.");

    mhOwner = hOwner;
    mhName = hName;
    mhThis = hThis;

    AttachComponent(hTransformComponent);
}

void RGameObject::setName(StringHandle hName)
{
    mhName = hName;
}

void RGameObject::tryExpandComponentBuffer()
{
    if (mComponentCount < mhComponents.GetSize())
    {
        return;
    }

    FixedArray<ObjectHandle<RComponent>> hNewComponents(mhComponents.GetSize() * 2);
    for (int32_t i = 0; i < mhComponents.GetSize(); ++i)
    {
        hNewComponents[i] = mhComponents[i];
    }
    mhComponents = std::move(hNewComponents);
}

void RGameObject::tryShrinkComponentBuffer()
{
    if (mhComponents.GetSize() > sMinComponentBufCount && mComponentCount < mhComponents.GetSize() / 3)
    {
        FixedArray<ObjectHandle<RComponent>> hNewComponents(mhComponents.GetSize() / 2);
        for (int32_t i = 0; i < mComponentCount; ++i)
        {
            hNewComponents[i] = mhComponents[i];
        }
        mhComponents = std::move(hNewComponents);
    }
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RGameObject, "Game Object", RObject)
HO_REGISTER_CLASS_PROPERTY(mhName, "Name");
HO_REGISTER_CLASS_PROPERTY(mComponentCount, "Component Count");
HO_REGISTER_CLASS_PROPERTY(mhComponents, "Components");
HO_REGISTER_CLASS_END()
} // namespace ho