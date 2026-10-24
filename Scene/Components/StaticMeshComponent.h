#pragma once

#include "MeshComponent.h"
#include "Subsystems/Asset/StaticMeshAsset.h"

namespace ho
{
class RGameObject;

class RStaticMeshComponent : public RMeshComponent
{
    HO_INJECT_REFLECTION_CODE(RStaticMeshComponent)

  public:
    RStaticMeshComponent();
    ~RStaticMeshComponent() override;

    void OnCreated() override;

    void OnAttached(ObjectHandle<RGameObject> hOwner) override;
    void OnDetached() override;

    void OnUpdate(float deltaTime) override;

    void OnPendingDestroyed() override;
    void OnDestroyed() override;

    ObjectHandle<RStaticMeshAsset> GetMesh() const
    {
        return mhMesh;
    }

    void SetMesh(ObjectHandle<RStaticMeshAsset> hMesh)
    {
        mhMesh = hMesh;
    }

  protected:
    ObjectHandle<RStaticMeshAsset> mhMesh;
};

} // namespace ho