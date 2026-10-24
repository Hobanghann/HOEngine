#pragma once

#include "Component.h"
#include "Subsystems/Asset/TextureAsset.h"

namespace ho
{
class RGameObject;

class RSkyBoxComponent : public RComponent
{
    HO_INJECT_REFLECTION_CODE(RSkyBoxComponent)

  public:
    RSkyBoxComponent();
    ~RSkyBoxComponent() override;

    void OnCreated() override;

    void OnAttached(ObjectHandle<RGameObject> hOwner) override;
    void OnDetached() override;

    void OnUpdate(float deltaTime) override;

    void OnPendingDestroyed() override;
    void OnDestroyed() override;

    float GetInstensity() const
    {
        return mIntensity;
    }

    void SetIntensity(float intensity)
    {
        mIntensity = intensity;
    }

    ObjectHandle<RTextureAsset> GetSkyMap() const
    {
        return mhSkyMap;
    }

    void SetSkyMap(ObjectHandle<RTextureAsset> hSkyMap)
    {
        mhSkyMap = hSkyMap;
    }

    ObjectHandle<RTextureAsset> GetIrradianceMap() const
    {
        return mhIrradianceMap;
    }

    void SetIrradianceMap(ObjectHandle<RTextureAsset> hIrrMap)
    {
        mhIrradianceMap = hIrrMap;
    }

    ObjectHandle<RTextureAsset> GetPrefilteredEnvMap() const
    {
        return mhPrefilteredEnvMap;
    }

    void SetPrefilteredEnvMap(ObjectHandle<RTextureAsset> hPrefilteredEnvMap)
    {
        mhPrefilteredEnvMap = hPrefilteredEnvMap;
    }

  protected:
    float mIntensity = 0.1f;
    ObjectHandle<RTextureAsset> mhSkyMap;
    ObjectHandle<RTextureAsset> mhIrradianceMap;
    ObjectHandle<RTextureAsset> mhPrefilteredEnvMap;
};

} // namespace ho