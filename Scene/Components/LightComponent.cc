#include "LightComponent.h"

#include "Core/Math/Color32.h"
#include "Subsystems/Asset/AssetSystem.h"
#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{

void RLightComponent::OnCreated()
{
    RComponent::OnCreated();

    mType = eLightType::Directional;
    mbCastShadow = false;
    mhStaticShadowMap = ObjectHandle<RTextureAsset>::sNULL;
    mColor = Color128(Color32::sWhite);
    mIntensity = 1.0f;
}

void RLightComponent::OnAttached(ObjectHandle<RGameObject> hOwner)
{
    RComponent::OnAttached(hOwner);
}

void RLightComponent::OnDetached()
{
    RComponent::OnDetached();
}

void RLightComponent::OnUpdate(float deltaTime)
{
    RComponent::OnUpdate(deltaTime);
}

void RLightComponent::OnPendingDestroyed()
{
    RComponent::OnPendingDestroyed();
}

void RLightComponent::OnDestroyed()
{
    RComponent::OnDestroyed();
    AssetSystem::GetInstance().DeleteAsset(mhStaticShadowMap);
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RLightComponent, "Light", RComponent)
HO_REGISTER_CLASS_PROPERTY(mType, "Light Type");
HO_REGISTER_CLASS_PROPERTY(mbCastShadow,
                           "Cast Shadow",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::Checkbox)
HO_REGISTER_CLASS_PROPERTY(mhStaticShadowMap, "Static Shadow Map", PropertyFlags_None, ePropertyUIStyle::None)
HO_REGISTER_CLASS_PROPERTY(mColor,
                           "Color",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::Color3)
HO_REGISTER_CLASS_PROPERTY(mIntensity,
                           "Intensity",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragFloat)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Intensity", 0.0f, 10000.0f)
HO_REGISTER_CLASS_END()
} // namespace ho