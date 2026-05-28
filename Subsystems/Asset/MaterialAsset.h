#pragma once

#include "AssetDefs.h"
#include "Core/Math/Color128.h"
#include "Core/Math/Matrix3x3.h"
#include "Core/Templates/GlobalPoolIndex.h"
#include "Core/Templates/WeakLocalPoolIndex.h"
#include "ShaderAsset.h"
#include "TextureAsset.h"

namespace ho
{
struct GpuMaterial;
using GpuMaterialHandle = WeakLocalPoolIndex<GpuMaterial, ObjectPool>;

struct MaterialAsset
{
    struct RenderPipelineState
    {
        eMaterialAlphaMode AlphaMode = eMaterialAlphaMode::None;
        eMaterialAlphaBlendMode AlphaBlendMode = eMaterialAlphaBlendMode::None;
        float AlphaThreshold = 0.5f;
        bool bWireframe = false;
        bool bBackfaceCulling = true;
    };

    MaterialAsset()
    {
        for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
        {
            hTextures[i] = TextureHandle::sNULL;
            UVChannels[i] = 0;
            UVTransforms[i] = Matrix3x3::sIdentity;
        }
    }

    StringHandle hName = StringHandle::sNULL;

    eMaterialAssetType Type = eMaterialAssetType::None;

    RenderPipelineState PipelineState;

    ShaderHandle hShaders[static_cast<int32_t>(eShaderStage::Last)] = {};

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
    Color128 Emissive{0.f, 0.f, 0.f};
    float EmissiveIntensity = 1.0f;
    float NormalScale = 1.0f;
    float OcclusionStrength = 1.0f;

    StringHandle hTextureNames[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    TextureHandle hTextures[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    int32_t UVChannels[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    Matrix3x3 UVTransforms[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};

    GpuMaterialHandle hRenderProxy = GpuMaterialHandle::sNULL;
};

using MaterialHandle = WeakLocalPoolIndex<MaterialAsset, ObjectPool>;
} // namespace ho