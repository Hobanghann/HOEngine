#pragma once
#include "Core/Memory/WeakTypedHandle.h"

namespace ho
{
struct GpuStaticMesh;
using GpuStaticMeshHandle = WeakTypedHandle<GpuStaticMesh, TypedObjectPool>;
struct GpuMaterial;
using GpuMaterialHandle = WeakTypedHandle<GpuMaterial, TypedObjectPool>;
struct GpuTexture;
using GpuTextureHandle = WeakTypedHandle<GpuTexture, TypedObjectPool>;
struct GpuShader;
using GpuShaderHandle = WeakTypedHandle<GpuShader, TypedObjectPool>;
struct GpuMorphTargets;
using GpuMorphTargetsHandle = WeakTypedHandle<GpuMorphTargets, TypedObjectPool>;

struct RenderingBuiltInAssets
{
    GpuShaderHandle hDefaultVS = GpuShaderHandle::sNULL;

    GpuShaderHandle hForwardUnlitOpaqueFS = GpuShaderHandle::sNULL;
    GpuShaderHandle hForwardUnlitMaskedFS = GpuShaderHandle::sNULL;
    GpuShaderHandle hForwardUnlitBlendedFS = GpuShaderHandle::sNULL;

    GpuShaderHandle hForwardPhongOpaqueFS = GpuShaderHandle::sNULL;
    GpuShaderHandle hForwardPhongMaskedFS = GpuShaderHandle::sNULL;
    GpuShaderHandle hForwardPhongBlendedFS = GpuShaderHandle::sNULL;

    GpuShaderHandle hForwardPbrOpaqueFS = GpuShaderHandle::sNULL;
    GpuShaderHandle hForwardPbrMaskedFS = GpuShaderHandle::sNULL;
    GpuShaderHandle hForwardPbrBlendedFS = GpuShaderHandle::sNULL;

    GpuShaderHandle hRadianceMapVS = GpuShaderHandle::sNULL;
    GpuShaderHandle hRadianceMapFS = GpuShaderHandle::sNULL;

    GpuShaderHandle hToneMappingGammaCorrectionVS = GpuShaderHandle::sNULL;
    GpuShaderHandle hToneMappingGammaCorrectionFS = GpuShaderHandle::sNULL;

    GpuShaderHandle hWorldGizmoVS = GpuShaderHandle::sNULL;
    GpuShaderHandle hPerspectiveWorldGizmoFS = GpuShaderHandle::sNULL;
    GpuShaderHandle hOrthographicWorldGizmoFS = GpuShaderHandle::sNULL;

    GpuShaderHandle hDirLightShadowMapVS = GpuShaderHandle::sNULL;
    GpuShaderHandle hDirLightShadowMapGS = GpuShaderHandle::sNULL;
    GpuShaderHandle hDirLightShadowMapFS = GpuShaderHandle::sNULL;
    GpuShaderHandle hDirLightShadowMapMaskedFS = GpuShaderHandle::sNULL;

    GpuShaderHandle hPointLightShadowMapVS = GpuShaderHandle::sNULL;
    GpuShaderHandle hPointLightShadowMapGS = GpuShaderHandle::sNULL;
    GpuShaderHandle hPointLightShadowMapFS = GpuShaderHandle::sNULL;
    GpuShaderHandle hPointLightShadowMapMaskedFS = GpuShaderHandle::sNULL;

    GpuShaderHandle hSpotLightShadowMapVS = GpuShaderHandle::sNULL;
    GpuShaderHandle hSpotLightShadowMapFS = GpuShaderHandle::sNULL;
    GpuShaderHandle hSpotLightShadowMapMaskedFS = GpuShaderHandle::sNULL;

    GpuShaderHandle hOutlineVS = GpuShaderHandle::sNULL;
    GpuShaderHandle hSingleColorFS = GpuShaderHandle::sNULL;

    GpuShaderHandle hIrradianceConvolutionCS = GpuShaderHandle::sNULL;
    GpuShaderHandle hPrefilteredEnvironmentConvolutionCS = GpuShaderHandle::sNULL;

    GpuTextureHandle hBRDFIntegrationMap = GpuTextureHandle::sNULL;

    GpuTextureHandle hBlackCubeMap = GpuTextureHandle::sNULL;
    GpuTextureHandle hFallbackCubeMap = GpuTextureHandle::sNULL;
    GpuTextureHandle hCheckerBoardCubeMap = GpuTextureHandle::sNULL;
    GpuTextureHandle hGrayCubeMap = GpuTextureHandle::sNULL;

    GpuTextureHandle hFallbackTexture = GpuTextureHandle::sNULL;
    GpuTextureHandle hCheckerBoardTexture = GpuTextureHandle::sNULL;
    GpuTextureHandle hGrayTexture = GpuTextureHandle::sNULL;
    GpuTextureHandle hMetallic1Roughness0Texture = GpuTextureHandle::sNULL;

    GpuTextureHandle hFallbackDirLightStaticShadowMap = GpuTextureHandle::sNULL;
    GpuTextureHandle hFallbackPointLightStaticShadowMap = GpuTextureHandle::sNULL;
    GpuTextureHandle hFallbackSpotLightStaticShadowMap = GpuTextureHandle::sNULL;

    GpuMaterialHandle hDefaultMaterial = GpuMaterialHandle::sNULL;
    GpuMaterialHandle hMetallic1Roughness0Material = GpuMaterialHandle::sNULL;

    GpuStaticMeshHandle hIcosaedroStaticMesh = GpuStaticMeshHandle::sNULL;
    GpuStaticMeshHandle hOctahedronStaticMesh = GpuStaticMeshHandle::sNULL;
};
} // namespace ho