#include "SkyBoxComponent.h"

#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{
RSkyBoxComponent::RSkyBoxComponent()
  : mhSkyMap(ObjectHandle<RTextureAsset>::sNULL)
{
}

RSkyBoxComponent::~RSkyBoxComponent() = default;

void RSkyBoxComponent::OnCreated()
{
    RComponent::OnCreated();
    mIntensity = 0.1f;
}

void RSkyBoxComponent::OnAttached(ObjectHandle<RGameObject> hOwner)
{
    RComponent::OnAttached(hOwner);
}

void RSkyBoxComponent::OnDetached()
{
    RComponent::OnDetached();
}

void RSkyBoxComponent::OnUpdate(float deltaTime)
{
    RComponent::OnUpdate(deltaTime);
}

void RSkyBoxComponent::OnPendingDestroyed()
{
    RComponent::OnPendingDestroyed();
}

void RSkyBoxComponent::OnDestroyed()
{
    RComponent::OnDestroyed();
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RSkyBoxComponent, "Sky Box", RComponent)
HO_REGISTER_CLASS_PROPERTY(mhSkyMap,
                           "Sky Map",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragAndDropAsset)
HO_REGISTER_CLASS_PROPERTY(mhIrradianceMap,
                           "Irradiance Map",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragAndDropAsset)
HO_REGISTER_CLASS_PROPERTY(mhPrefilteredEnvMap,
                           "Pre-filtered Environment Map",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragAndDropAsset)
HO_REGISTER_CLASS_PROPERTY(mIntensity,
                           "Intensity",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragFloat)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Intensity", 0.0f, 2.0f)
HO_REGISTER_CLASS_END()
} // namespace ho