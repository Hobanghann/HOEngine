#include "DirectionalLightComponent.h"

#include "Core/Math/Color32.h"
#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{

void RDirectionalLightComponent::OnCreated()
{
    RLightComponent::OnCreated();

    mType = eLightType::Directional;

    mCascadeCount = 4;
    mMaxShadowDistance = 150.0f;
}

void RDirectionalLightComponent::OnAttached(ObjectHandle<RGameObject> hOwner)
{
    RLightComponent::OnAttached(hOwner);
}

void RDirectionalLightComponent::OnDetached()
{
    RLightComponent::OnDetached();
}

void RDirectionalLightComponent::OnUpdate(float deltaTime)
{
    RLightComponent::OnUpdate(deltaTime);
}

void RDirectionalLightComponent::OnPendingDestroyed()
{
    RLightComponent::OnPendingDestroyed();
}

void RDirectionalLightComponent::OnDestroyed()
{
    RLightComponent::OnDestroyed();
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RDirectionalLightComponent, "Directional Light", RLightComponent)
HO_REGISTER_CLASS_PROPERTY(mCascadeCount,
                           "Cascade Count",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragInt)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Cascade Count", 1, 6)
HO_REGISTER_CLASS_PROPERTY(mMaxShadowDistance,
                           "Max Shadow Distance",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragFloat)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Max Shadow Distance", 10.0f, 500.0f)
HO_REGISTER_CLASS_END()
} // namespace ho