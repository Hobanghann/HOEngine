#pragma once

#include "Component.h"
#include "Core/Math/Color128.h"
#include "Subsystems/Asset/TextureAsset.h"

namespace ho
{
class RGameObject;
struct RTextureAsset;

class RLightComponent : public RComponent
{
    HO_INJECT_REFLECTION_CODE(RLightComponent)

  public:
    enum class eLightType : uint8_t
    {
        Directional = 0,
        Point = 1,
        Spot = 2,
    };

    RLightComponent() = default;
    ~RLightComponent() override = default;

    void OnCreated() override;

    void OnAttached(ObjectHandle<RGameObject> hOwner) override;
    void OnDetached() override;

    void OnUpdate(float deltaTime) override;

    void OnPendingDestroyed() override;
    void OnDestroyed() override;

    void MakeDirty()
    {
        mbIsDirty = true;
    }

    [[nodiscard]] bool IsDirty() const;

    eLightType GetType() const
    {
        return mType;
    }

    void SetCastShadow(bool bCastShadow)
    {
        mbCastShadow = bCastShadow;
    }

    bool IsCastingShadow() const
    {
        return mbCastShadow;
    }

    void SetStaticShadowMap(ObjectHandle<RTextureAsset> hShadowMap)
    {
        mhStaticShadowMap = hShadowMap;
        mbIsDirty = false;
    }

    ObjectHandle<RTextureAsset> GetStaticShadowMap() const
    {
        return mhStaticShadowMap;
    }

    const Color128& GetColor() const
    {
        return mColor;
    }

    void SetColor(const Color128& color)
    {
        mColor = color;
    }

    float GetInstensity() const
    {
        return mIntensity;
    }

    void SetIntensity(float intensity)
    {
        mIntensity = intensity;
    }

  protected:
    eLightType mType = eLightType::Directional;
    bool mbCastShadow = true;
    ObjectHandle<RTextureAsset> mhStaticShadowMap;
    // It means static shadow map need to be updated.
    bool mbIsDirty = true;
    Color128 mColor;
    float mIntensity = 1.0f;
};

} // namespace ho