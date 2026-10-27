#pragma once

#include "Subsystems/Object/Object.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
class RGameObject;

class RComponent : public RObject
{
    HO_INJECT_REFLECTION_CODE(RComponent)
    friend class RScene;

  public:
    RComponent();
    ~RComponent() override;

    ObjectHandle<RGameObject> GetOwner();

    ObjectHandle<const RGameObject> GetOwner() const;

    ObjectHandle<RComponent> GetHandle();

    ObjectHandle<const RComponent> GetHandle() const;

    virtual void OnCreated();

    virtual void OnAttached(ObjectHandle<RGameObject> hOwner);

    virtual void OnDetached();

    virtual void OnUpdate(float deltaTime);

    virtual void OnPendingDestroyed();
    virtual void OnDestroyed();

    bool IsAttached() const;

    bool IsDestroyed() const;

    bool IsActive() const;

    bool IsPendingDestroyed() const;

    void SetActive(bool bActive);

  protected:
    ObjectHandle<RGameObject> mhOwner;
    bool mbAttached = false;
    bool mbPendingDestroyed = false;
    bool mbDestroyed = false;
    bool mbActive = true;

  private:
    void initialize(ObjectHandle<RComponent> hThis);

    ObjectHandle<RComponent> mhThis;
};
} // namespace ho