#include "MeshComponent.h"

#include "Subsystems/Reflection/ClassRegister.h"
#include "Subsystems/Reflection/EnumRegister.h"

namespace ho
{

void RMeshComponent::OnCreated()
{
    RComponent::OnCreated();

    mType = eMeshType::Static;
    mShadowCastingMode = eShadowCastingMode::None;
}

void RMeshComponent::OnAttached(ObjectHandle<RGameObject> hOwner)
{
    RComponent::OnAttached(hOwner);
}

void RMeshComponent::OnDetached()
{
    RComponent::OnDetached();
}

void RMeshComponent::OnUpdate(float deltaTime)
{
    RComponent::OnUpdate(deltaTime);
}

void RMeshComponent::OnPendingDestroyed()
{
    RComponent::OnPendingDestroyed();
}

void RMeshComponent::OnDestroyed()
{
    RComponent::OnDestroyed();
}

HO_REGISTER_ENUM_BEGIN(RMeshComponent::eShadowCastingMode, "Shadow Casting Mode")
HO_REGISTER_ENUM_ENTRY("None", RMeshComponent::eShadowCastingMode::None)
HO_REGISTER_ENUM_ENTRY("Static", RMeshComponent::eShadowCastingMode::Static)
HO_REGISTER_ENUM_ENTRY("Dynamic", RMeshComponent::eShadowCastingMode::Dynamic)
HO_REGISTER_ENUM_END()

HO_REGISTER_CLASS_DERIVED_BEGIN(RMeshComponent, "Mesh", RComponent)
HO_REGISTER_CLASS_PROPERTY(mType, "Mesh Type");
HO_REGISTER_CLASS_PROPERTY(mShadowCastingMode,
                           "Shadow Casting Mode",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::ComboBox)
HO_REGISTER_CLASS_END()
} // namespace ho