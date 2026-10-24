#pragma once

#include "Core/Math/Color128.h"
#include "LightComponent.h"

namespace ho
{
class RGameObject;

struct GpuTexture;
using GpuTextureHandle = WeakTypedHandle<GpuTexture, TypedObjectPool>;

class RDirectionalLightComponent : public RLightComponent
{
    HO_INJECT_REFLECTION_CODE(RDirectionalLightComponent)

  public:
    RDirectionalLightComponent() = default;
    ~RDirectionalLightComponent() override = default;

    void OnCreated() override;

    void OnAttached(ObjectHandle<RGameObject> hOwner) override;
    void OnDetached() override;

    void OnUpdate(float deltaTime) override;

    void OnPendingDestroyed() override;
    void OnDestroyed() override;

    int32_t GetCascadeCount() const
    {
        return mCascadeCount;
    }

    void SetCascadeCount(int32_t cascadeCount)
    {
        mCascadeCount = cascadeCount;
    }

    float GetMaxShadowDistance() const
    {
        return mMaxShadowDistance;
    }

    void SetMaxShadowDistance(float maxDistance)
    {
        mMaxShadowDistance = maxDistance;
    }

  private:
    int32_t mCascadeCount = 4;
    float mMaxShadowDistance = 150.0f;
};

} // namespace ho