#pragma once

#include "Core/Math/Color128.h"
#include "LightComponent.h"

namespace ho
{
struct Sphere;
class RGameObject;

class RSpotLightComponent : public RLightComponent
{
    HO_INJECT_REFLECTION_CODE(RSpotLightComponent)

  public:
    RSpotLightComponent() = default;
    ~RSpotLightComponent() override = default;

    void OnCreated() override;

    void OnAttached(ObjectHandle<RGameObject> hOwner) override;
    void OnDetached() override;

    void OnUpdate(float deltaTime) override;

    void OnPendingDestroyed() override;
    void OnDestroyed() override;

    float GetInnerCutoff() const
    {
        return mInnerCutoff;
    }

    void SetInnerCutoff(float cutoff)
    {
        mInnerCutoff = cutoff;
    }

    float GetOuterCutoff() const
    {
        return mOuterCutoff;
    }

    void SetOuterCutoff(float cutoff)
    {
        mOuterCutoff = cutoff;
    }

    float GetConstantAtten() const
    {
        return mConstantAtten;
    }

    void SetConstantAtten(float atten)
    {
        mConstantAtten = atten;
    }

    float GetLinearAtten() const
    {
        return mLinearAtten;
    }

    void SetLinearAtten(float atten)
    {
        mLinearAtten = atten;
    }

    float GetQuadraticAtten() const
    {
        return mQuadraticAtten;
    }

    void SetQuadraticAtten(float atten)
    {
        mQuadraticAtten = atten;
    }

    float GetRadius() const;

    Sphere GetBoundingVolume() const;

  private:
    // Spotlight inner coverage half-angle in radians
    float mInnerCutoff = 0.2618f;
    // Spotlight outter coverage half-angle in radians
    float mOuterCutoff = 0.5236f;

    float mConstantAtten = 1.0f;
    float mLinearAtten = 0.045f;
    float mQuadraticAtten = 0.0075f;
};

} // namespace ho