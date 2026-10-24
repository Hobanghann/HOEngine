#pragma once

#include "Component.h"
#include "Core/Math/Projection.h"
#include "Core/Math/Transform3D.h"

namespace ho
{
class RCameraComponent : public RComponent
{
    HO_INJECT_REFLECTION_CODE(RCameraComponent)

  public:
    enum class eProjectionType : uint8_t
    {
        Perspective = 0,
        Orthographic = 1,
    };

    RCameraComponent() {}

    ~RCameraComponent() override = default;

    void OnCreated() override;

    void OnAttached(ObjectHandle<RGameObject> hOwner) override;
    void OnDetached() override;

    void OnUpdate(float deltaTime) override;

    void OnPendingDestroyed() override;
    void OnDestroyed() override;

    [[nodiscard]] Projection GetProjection() const;

  private:
    eProjectionType mProjectionType = eProjectionType::Perspective;
    float mFOV = math::PI2 * 0.6f;
    float mViewHeight = 100.0f;
    float mAspectRatio = 4.0f / 3.0f;
    float mNearDist = 0.001f;
    float mFarDist = 1000.0f;
};
} // namespace ho