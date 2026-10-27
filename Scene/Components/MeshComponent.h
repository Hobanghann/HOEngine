#pragma once

#include "Component.h"

namespace ho
{
class RGameObject;

class RMeshComponent : public RComponent
{
    HO_INJECT_REFLECTION_CODE(RMeshComponent)

  public:
    enum class eMeshType : uint8_t
    {
        Static = 0,
        Skeletal = 1,
    };

    enum class eShadowCastingMode : uint8_t
    {
        None = 0,
        Static = 1,
        Dynamic = 2,
    };

    RMeshComponent() = default;
    ~RMeshComponent() override = default;

    void OnCreated() override;

    void OnAttached(ObjectHandle<RGameObject> hOwner) override;
    void OnDetached() override;

    void OnUpdate(float deltaTime) override;

    void OnPendingDestroyed() override;
    void OnDestroyed() override;

    eMeshType GetType() const
    {
        return mType;
    }

    void SetShadowCastingMode(eShadowCastingMode mode)
    {
        mShadowCastingMode = mode;
    }

    eShadowCastingMode GetShadowCastingMode() const
    {
        return mShadowCastingMode;
    }

    bool IsStencilEnabled() const
    {
        return mbStencilEnabled;
    }

    void SetStencilEnabled(bool bEnabled)
    {
        mbStencilEnabled = bEnabled;
    }

    uint8_t GetStencilReference() const
    {
        return mStencilReference;
    }

    void SetStencilReference(uint8_t ref)
    {
        mStencilReference = ref;
    }

  protected:
    eMeshType mType = eMeshType::Static;
    eShadowCastingMode mShadowCastingMode = eShadowCastingMode::None;

    bool mbStencilEnabled = false;
    uint8_t mStencilReference = 0;
};

} // namespace ho