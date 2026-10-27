#include "StaticMeshComponent.h"

#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{
RStaticMeshComponent::RStaticMeshComponent()
  : mhMesh(ObjectHandle<RStaticMeshAsset>::sNULL)
{
}

RStaticMeshComponent::~RStaticMeshComponent() = default;

void RStaticMeshComponent::OnCreated()
{
    RMeshComponent::OnCreated();

    mType = eMeshType::Static;
    mhMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
}

void RStaticMeshComponent::OnAttached(ObjectHandle<RGameObject> hOwner)
{
    RMeshComponent::OnAttached(hOwner);
}

void RStaticMeshComponent::OnDetached()
{
    RMeshComponent::OnDetached();
}

void RStaticMeshComponent::OnUpdate(float deltaTime)
{
    RMeshComponent::OnUpdate(deltaTime);
}

void RStaticMeshComponent::OnPendingDestroyed()
{
    RMeshComponent::OnPendingDestroyed();
}

void RStaticMeshComponent::OnDestroyed()
{
    RMeshComponent::OnDestroyed();
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RStaticMeshComponent, "Static Mesh", RMeshComponent)
HO_REGISTER_CLASS_PROPERTY(mhMesh,
                           "Mesh",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragAndDropAsset)
HO_REGISTER_CLASS_END()
} // namespace ho