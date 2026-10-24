#pragma once

#include <cstdint>

namespace ho
{
enum class eMaterialTextureUsage : uint16_t;

enum class eRenderTargetBindingIndex
{
    Final = 0,
    Accumulation = 1,
    DeferedAlbedoAO = 2,        // RGB: Albedo, A: AO
    DeferedNormalRoughness = 3, // RGB: World Normal, A: Roughness
    DeferedMaterial = 4,        // R: Metallic, G: Specular, B: MaterialID / Mask, A: Emissive
    Luminance = 5,
    Last,
};

enum class eBufferBindingIndex
{
    Matrices = 0,
    MaterialAttributes = 1,
    DirectLights = 2,
    DirectLightShadowData = 3,
    IndirectLights = 4,
    GeneralPurpose = 5,
    ShadowMapRenderData = 6,
    EnvironmentBakeData = 7,
    WorldGizmoData = 8,

    InstanceData = 9,
    MorphControlData = 10,

    MorphTargets = 11,
    MorphTargetPositions = 12,
    MorphTargetNormals = 13,
    MorphTargetTangents = 14,

    Last,
};

enum class eTextureBindingIndex
{
    // Material Textures
    Diffuse = 1,
    Specular = 2,
    Opacity = 3,
    Normal = 4,
    Albedo = 5,
    MetallicRoughness = 6,
    Emissive = 7,
    AmbientOcclusion = 8,

    // IBL Textures
    RadianceMap = 9,
    IrradianceMap = 10,
    PrefilteredEnvironmentMap = 11,
    BRDFIntegrationMap = 12,

    // Render Target Textures
    Accumulation = 13,
    DeferedAlbedoAO = 14,
    DeferedNormalRoughness = 15,
    DeferedMaterial = 16,
    Luminance = 17,

    // Shadow Maps
    DirLightDynamicShadowMap = 18,
    PointLightDynamicShadowMap = 19,
    SpotLightDynamicShadowMap = 20,

    DirLightStaticShadowMap0 = 21,

    PointLightStaticShadowMap0 = 22,
    PointLightStaticShadowMap1 = PointLightStaticShadowMap0 + 1,
    PointLightStaticShadowMap2 = PointLightStaticShadowMap0 + 2,
    PointLightStaticShadowMap3 = PointLightStaticShadowMap0 + 3,

    SpotLightStaticShadowMap0 = 26,
    SpotLightStaticShadowMap1 = SpotLightStaticShadowMap0 + 1,
    SpotLightStaticShadowMap2 = SpotLightStaticShadowMap0 + 2,
    SpotLightStaticShadowMap3 = SpotLightStaticShadowMap0 + 3,
    SpotLightStaticShadowMap4 = SpotLightStaticShadowMap0 + 4,
    SpotLightStaticShadowMap5 = SpotLightStaticShadowMap0 + 5,
    SpotLightStaticShadowMap6 = SpotLightStaticShadowMap0 + 6,
    SpotLightStaticShadowMap7 = SpotLightStaticShadowMap0 + 7,

    Last,
};

enum class eStorageTextureBindingIndex
{
    IrradianceMap = 0,
    PrefilteredEnvironmentMap = 1,
    BRDFIntegrationMap = 2,
    Last,
};

eTextureBindingIndex getTextureBindingIndex(eMaterialTextureUsage usage);
} // namespace ho