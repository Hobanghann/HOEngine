#include "CameraComponent.h"

#include "Subsystems/Reflection/ClassRegister.h"
#include "Subsystems/Reflection/EnumRegister.h"

namespace ho
{
void RCameraComponent::OnCreated()
{
    RComponent::OnCreated();
}

void RCameraComponent::OnAttached(ObjectHandle<RGameObject> hOwner)
{
    RComponent::OnAttached(hOwner);
}

void RCameraComponent::OnDetached()
{
    RComponent::OnDetached();
}

void RCameraComponent::OnUpdate(float deltaTime)
{
    RComponent::OnUpdate(deltaTime);
}

void RCameraComponent::OnPendingDestroyed()
{
    RComponent::OnPendingDestroyed();
}

void RCameraComponent::OnDestroyed()
{
    RComponent::OnDestroyed();
}

Projection RCameraComponent::GetProjection() const
{
    switch (mProjectionType)
    {
        case eProjectionType::Perspective:
            return Projection::CreatePerspective(mFOV, mAspectRatio, mNearDist, mFarDist);
        case eProjectionType::Orthographic:
            return Projection::CreateOrthographic(mViewHeight, mAspectRatio, mNearDist, mFarDist);
        default:
            HO_ASSERT(false, "Invalid projection type.");
            return Projection::CreatePerspective(mFOV, mAspectRatio, mNearDist, mFarDist);
    }
}

HO_REGISTER_ENUM_BEGIN(RCameraComponent::eProjectionType, "Projection Type")
HO_REGISTER_ENUM_ENTRY("Perspective", RCameraComponent::eProjectionType::Perspective)
HO_REGISTER_ENUM_ENTRY("Orthographic", RCameraComponent::eProjectionType::Orthographic)
HO_REGISTER_ENUM_END()

HO_REGISTER_CLASS_DERIVED_BEGIN(RCameraComponent, "Camera", RComponent)
HO_REGISTER_CLASS_PROPERTY(mProjectionType,
                           "Projection Type",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::ComboBox)

HO_REGISTER_CLASS_PROPERTY(mFOV,
                           "FOV",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragAngle)
HO_REGISTER_CLASS_PROPERTY_MINMAX("FOV", 0.017f, 3.12f)

HO_REGISTER_CLASS_PROPERTY(mViewHeight,
                           "View Height",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragAngle)
HO_REGISTER_CLASS_PROPERTY_MINMAX("View Height", 0.01f, 10000.0f)

HO_REGISTER_CLASS_PROPERTY(mAspectRatio,
                           "Aspect Ratio",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragFloat)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Aspect Ratio", 0.1f, 10.0f)

HO_REGISTER_CLASS_PROPERTY(mNearDist,
                           "Near Clip",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragFloat)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Near Clip", 0.001f, 100.0f)

HO_REGISTER_CLASS_PROPERTY(mFarDist,
                           "Far Clip",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragFloat)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Far Clip", 1.0f, 10000.0f)
HO_REGISTER_CLASS_END()
} // namespace ho