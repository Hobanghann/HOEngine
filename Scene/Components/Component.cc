#include "Component.h"

#include "../GameObjects/GameObject.h"
#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{
RComponent::RComponent()
  : mhOwner(ObjectHandle<RGameObject>::sNULL)
  , mhThis(ObjectHandle<RComponent>::sNULL)
{
}

RComponent::~RComponent() = default;

ObjectHandle<RGameObject> RComponent::GetOwner()
{
    return mhOwner;
}

ObjectHandle<const RGameObject> RComponent::GetOwner() const
{
    return mhOwner;
}

ObjectHandle<RComponent> RComponent::GetHandle()
{
    return mhThis;
}

ObjectHandle<const RComponent> RComponent::GetHandle() const
{
    return mhThis;
}

void RComponent::OnCreated() {}

void RComponent::OnAttached(ObjectHandle<RGameObject> hOwner)
{
    HO_ASSERT(hOwner.IsValid(), "Invalid game object handle.");
    HO_ASSERT(!mbAttached, "Component is already attached.");
    HO_ASSERT(!mbPendingDestroyed && !mbDestroyed, "Component is already destroyed.");

    mhOwner = hOwner;
    mbAttached = true;
}

void RComponent::OnDetached()
{
    HO_ASSERT(mbAttached, "Component is already detached.");

    mhOwner = ObjectHandle<RGameObject>::sNULL;
    mbAttached = false;
}

void RComponent::OnUpdate(float deltaTime)
{
    HO_ASSERT(mbActive, "Inactive component can't be updated.");
}

void RComponent::OnPendingDestroyed()
{
    HO_ASSERT(!mbPendingDestroyed, "Component is already destroyed.");
    mbPendingDestroyed = true;
}

void RComponent::OnDestroyed()
{
    HO_ASSERT(!mbAttached, "Component is must be dettached before destroying.");
    HO_ASSERT(mbPendingDestroyed, "Component must be pending destroyed before actually destroying.");
    HO_ASSERT(!mbDestroyed, "Component is already destroyed.");
    mbDestroyed = true;
}

bool RComponent::IsAttached() const
{
    return mbAttached;
}

bool RComponent::IsDestroyed() const
{
    return mbDestroyed;
}

bool RComponent::IsActive() const
{
    return mbActive;
}

bool RComponent::IsPendingDestroyed() const
{
    return mbPendingDestroyed;
}

void RComponent::SetActive(bool bActive)
{
    mbActive = bActive;
}

void RComponent::initialize(ObjectHandle<RComponent> hThis)
{
    mhThis = hThis;
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RComponent, "Component", RObject)
HO_REGISTER_CLASS_END()
} // namespace ho