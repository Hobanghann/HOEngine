#pragma once

#include "AssetDefs.h"
#include "Core/Math/Color128.h"
#include "Core/Math/Matrix3x3.h"
#include "IAsset.h"
#include "ShaderAsset.h"
#include "Subsystems/Object/ObjectHandle.h"
#include "TextureAsset.h"

namespace ho
{
struct GpuMaterial;
using GpuMaterialHandle = WeakTypedHandle<GpuMaterial, TypedObjectPool>;

struct RMaterialAsset final : public RIAsset
{
    HO_INJECT_REFLECTION_CODE(RMaterialAsset)

    struct RenderPipelineState
    {
        eMaterialAlphaMode AlphaMode = eMaterialAlphaMode::None;
        eMaterialAlphaBlendMode AlphaBlendMode = eMaterialAlphaBlendMode::None;
        bool bWireframe = false;
        bool bBackfaceCulling = true;
    };

    RMaterialAsset()
    {
        for (int32_t i = 0; i < static_cast<int32_t>(eShaderStage::Last); ++i)
        {
            hShaders[i] = ObjectHandle<RShaderAsset>::sNULL;
        }
        for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
        {
            hTextures[i] = ObjectHandle<RTextureAsset>::sNULL;
            UVChannels[i] = 0;
            UVTransforms[i] = Matrix3x3::sIdentity;
        }
    }

    RMaterialAsset(const RMaterialAsset&) = delete;
    RMaterialAsset& operator=(const RMaterialAsset&) = delete;
    ~RMaterialAsset() override = default;

    RMaterialAsset(RMaterialAsset&&) noexcept = default;
    RMaterialAsset& operator=(RMaterialAsset&&) noexcept = default;

    int32_t GetByteSize() const override;

    eMaterialAssetType Type = eMaterialAssetType::None;

    RenderPipelineState PipelineState;

    ObjectHandle<RShaderAsset> hShaders[static_cast<int32_t>(eShaderStage::Last)] = {};

    // Legacy / Phong Attributes
    Color128 Ambient{0.f, 0.f, 0.f, 1.f};
    Color128 Diffuse{1.f, 1.f, 1.f, 1.f};
    Color128 Specular{1.f, 1.f, 1.f, 1.f};
    float Shininess = 32.0f;

    // PBR Core Attributes
    Color128 Albedo{1.f, 1.f, 1.f, 1.f};
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    float IndexOfRefraction = 1.45f;

    // Global / Transparency Attributes
    float Opacity = 1.0f;
    float AlphaThreshold = 0.5f;
    Color128 Emissive{0.f, 0.f, 0.f};
    float EmissiveIntensity = 1.0f;
    float NormalScale = 1.0f;
    float OcclusionStrength = 1.0f;

    StringHandle hTextureNames[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    ObjectHandle<RTextureAsset> hTextures[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    int32_t UVChannels[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    Matrix3x3 UVTransforms[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};

    GpuMaterialHandle hRenderProxy;
};

} // namespace ho