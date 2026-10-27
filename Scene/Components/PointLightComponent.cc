#include "PointLightComponent.h"

#include "../GameObjects/GameObject.h"
#include "Core/Math/Color32.h"
#include "Core/Math/Sphere.h"
#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{

void RPointLightComponent::OnCreated()
{
    RLightComponent::OnCreated();

    mType = eLightType::Point;

    mConstantAtten = 1.0f;
    mLinearAtten = 0.045f;
    mQuadraticAtten = 0.0075f;
}

void RPointLightComponent::OnAttached(ObjectHandle<RGameObject> hOwner)
{
    RLightComponent::OnAttached(hOwner);
}

void RPointLightComponent::OnDetached()
{
    RLightComponent::OnDetached();
}

void RPointLightComponent::OnUpdate(float deltaTime)
{
    RLightComponent::OnUpdate(deltaTime);
}

void RPointLightComponent::OnPendingDestroyed()
{
    RLightComponent::OnPendingDestroyed();
}

void RPointLightComponent::OnDestroyed()
{
    RLightComponent::OnDestroyed();
}

float RPointLightComponent::GetRadius() const
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

Sphere RPointLightComponent::GetBoundingVolume() const
{
    const float radius = GetRadius();
    const Vector3 center = mhOwner->GetComponent<RTransformComponent>()->GetWorldTransform().GetPosition();
    return Sphere(center, radius);
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RPointLightComponent, "Point Light", RLightComponent)
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