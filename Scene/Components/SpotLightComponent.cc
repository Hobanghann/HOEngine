#include "SpotLightComponent.h"

#include "../GameObjects/GameObject.h"
#include "Core/Math/Color32.h"
#include "Core/Math/Sphere.h"
#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{
void RSpotLightComponent::OnCreated()
{
    RLightComponent::OnCreated();

    mType = eLightType::Spot;

    mInnerCutoff = 0.2618f;
    mOuterCutoff = 0.5236f;

    mConstantAtten = 1.0f;
    mLinearAtten = 0.045f;
    mQuadraticAtten = 0.0075f;
}

void RSpotLightComponent::OnAttached(ObjectHandle<RGameObject> hOwner)
{
    RLightComponent::OnAttached(hOwner);
}

void RSpotLightComponent::OnDetached()
{
    RLightComponent::OnDetached();
}

void RSpotLightComponent::OnUpdate(float deltaTime)
{
    RLightComponent::OnUpdate(deltaTime);
}

void RSpotLightComponent::OnPendingDestroyed()
{
    RLightComponent::OnPendingDestroyed();
}

void RSpotLightComponent::OnDestroyed()
{
    RLightComponent::OnDestroyed();
}

float RSpotLightComponent::GetRadius() const
{
    const float threshold = 0.005f;

    const float maxAtten = mIntensity / threshold;
    if (maxAtten <= mConstantAtten)
    {
        return 0.0f;
    }

    const float c = mConstantAtten - maxAtten;
    const float b = mLinearAtten;
    const float a = mQuadraticAtten;

    if (math::IsEqualApprox(a, 0.0f))
    {
        if (math::IsEqualApprox(b, 0.0f))
        {
            return 0.0f;
        }
        return math::Max(0.0f, -c / b);
    }

    const float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
    {
        return 50.0f;
    }

    const float d = (-b + math::SqrtFast(discriminant)) / (2.0f * a);
    return math::Max(0.0f, d);
}

Sphere RSpotLightComponent::GetBoundingVolume() const
{
    const float dist = GetRadius();
    const Vector3 pos = mhOwner->GetComponent<RTransformComponent>()->GetWorldTransform().GetPosition();

    if (dist <= 0.0f)
    {
        return Sphere(pos, 0.0f);
    }

    const Vector3 dir = mhOwner->GetComponent<RTransformComponent>()->GetWorldTransform().GetForward();

    const float cutoff = math::Clamp(mOuterCutoff, 0.0f, math::PI2 - 0.001f);

    float radius = 0.0f;
    Vector3 center;
    if (cutoff < math::PI4)
    {
        const float outerCos = math::Cos(cutoff);
        radius = dist / (2.0f * outerCos * outerCos);
        center = pos + radius * dir;
    }
    else
    {
        radius = dist * math::Tan(cutoff);
        center = pos + dist * dir;
    }

    return Sphere(center, math::Max(0.0f, radius));
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RSpotLightComponent, "Spot Light", RLightComponent)
HO_REGISTER_CLASS_PROPERTY(mInnerCutoff,
                           "Inner Cutoff",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragAngle)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Inner Cutoff", 0.0f, math::DegToRad(87.5f))

HO_REGISTER_CLASS_PROPERTY(mOuterCutoff,
                           "Outer Cutoff",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragAngle)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Outer Cutoff", 0.0f, math::DegToRad(87.5f))

HO_REGISTER_CLASS_PROPERTY(mConstantAtten,
                           "Constant Attenuation",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragFloat)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Constant Attenuation", 0.001f, 10.0f)

HO_REGISTER_CLASS_PROPERTY(mLinearAtten,
                           "Linear Attenuation",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragFloat)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Linear Attenuation", 0.0f, 1.0f)

HO_REGISTER_CLASS_PROPERTY(mQuadraticAtten,
                           "Quadratic Attenuation",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragFloat)
HO_REGISTER_CLASS_PROPERTY_MINMAX("Quadratic Attenuation", 0.0f, 1.0f)
HO_REGISTER_CLASS_END()
} // namespace ho