#pragma once

#include "../RenderData/LightData.h"
#include "../RenderData/LightShadowData.h"
#include "Core/Math/Matrix4x4.h"
#include "RenderBindingIndex.h"
#include "Subsystems/Asset/AssetDefs.h"

namespace ho
{
struct alignas(16) GpuInt32
{
    GpuInt32() = default;

    GpuInt32(int32_t val)
      : Value(val)
    {
    }

    GpuInt32& operator=(int32_t val)
    {
        Value = val;
        return *this;
    }

    int32_t Value = 0;
    int32_t Padding[3] = {};
};

struct alignas(16) GpuFloat
{
    GpuFloat() = default;

    GpuFloat(float val)
      : Value(val)
    {
    }

    GpuFloat& operator=(float val)
    {
        Value = val;
        return *this;
    }

    float Value = 0;
    float Padding[3] = {};
};

struct MatrixLayout
{
    Matrix4x4 ViewMat;
    Matrix4x4 ProjMat;
};

struct MaterialAttributeLayout
{
    Vector4 Ambient{0.f, 0.f, 0.f, 1.f};
    Vector4 Diffuse{1.f, 1.f, 1.f, 1.f};
    Vector4 Specular{1.f, 1.f, 1.f, 1.f};
    Vector4 Albedo{1.f, 1.f, 1.f, 1.f};
    Vector4 Emissive{0.f, 0.f, 0.f, 1.f};

    float Shininess = 32.0f;
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    float IndexOfRefraction = 1.45f;

    float Opacity = 1.0f;
    float AlphaThreshold = 0.5f;
    float EmissiveIntensity = 1.0f;
    float NormalScale = 1.0f;
    float OcclusionStrength = 1.0f;

    alignas(16) GpuInt32 UVChannels[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};

    alignas(16) Matrix4x4 UVMatrices[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};

    alignas(16) GpuInt32 bHasTexture[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
};

struct DirLightLayout
{
    float Direction[3] = {};
    float Padding = 0.0f;

    float Color[3] = {};
    float Intensity = 0.0f;

    GpuInt32 ShadowDataIndex = -1; // -1 means no shadow.
};

struct alignas(16) PointLightLayout
{
    float Position[3] = {};
    float Padding0 = 0.0f;

    float Color[3] = {};

    float Intensity = 0.0f;
    float Constant = 0.0f;
    float Linear = 0.0f;
    float Quadratic = 0.0f;

    float FarDist = 0.0f;
    int32_t ShadowDataIndex = -1; // -1 means no shadow.
};

struct alignas(16) SpotLightLayout
{
    float Position[3] = {};
    float Padding0 = 0.0f;

    float Direction[3] = {};
    float Padding1 = 0.0f;

    float Color[3] = {};

    float Intensity = 0.0f;
    float InnerCutoff = 0.0f;
    float OuterCutoff = 0.0f;

    float Constant = 1.0f;
    float Linear = 0.045f;
    float Quadratic = 0.0075f;

    float FarDist = 0.0f;
    int32_t ShadowDataIndex = -1; // -1 means no shadow.
};

struct DirectLightLayout
{
    GpuInt32 DirLightCount = 0;
    DirLightLayout DirLights[DirectLightData::sMaxDirLightCount] = {};
    GpuInt32 PointLightCount = 0;
    PointLightLayout PointLights[DirectLightData::sMaxPointLightCount] = {};
    GpuInt32 SpotLightCount = 0;
    SpotLightLayout SpotLights[DirectLightData::sMaxSpotLightCount] = {};
};

struct IndirectLightLayout
{
    float Intensity = 0.0f;
};

struct ShadowMapRenderDataLayout
{
    GpuInt32 CascadeCount = 0;
    float LightWorldPos[3] = {};
    float FarDist = 0.0f;
    Matrix4x4 ProjViewMats[6] = {};
};

struct DirLightShadowDataLayout
{
    GpuInt32 CascadeCount = 0;
    alignas(16) GpuFloat CascadeNormalOffsetScales[6] = {};
    alignas(16) GpuFloat CascadeDepthOffsetScales[6] = {};
    alignas(16) GpuFloat CascadeFarDist[6] = {};
    alignas(16) Matrix4x4 ProjViewMats[6] = {};
};

struct PointLightShadowDataLayout
{
    float LightWorldPos[3] = {};
    float NormalOffsetScale = 0.0f;
    float DepthOffsetScale = 0.0f;
    float FarDist = 0.0f;
    float Padding[2] = {};
    Matrix4x4 ProjViewMats[6] = {};
};

struct SpotLightShadowDataLayout
{
    float LightWorldPos[3] = {};
    float NormalOffsetScale = 0.0f;
    float DepthOffsetScale = 0.0f;
    float FarDist = 0.0f;
    float Padding[2] = {};
    Matrix4x4 ProjViewMat;
};

struct DirectLightShadowDataLayout
{
    GpuInt32 DirLightShadowDataCount = 0;
    alignas(16) DirLightShadowDataLayout DirLightsShadowData[DirectLightShadowData::sMaxDirLightShadowDataCount] = {};
    GpuInt32 PointLightShadowDataCount = 0;
    alignas(16)
        PointLightShadowDataLayout PointLightsShadowData[DirectLightShadowData::sMaxPointLightShadowDataCount] = {};
    GpuInt32 SpotLightShadowDataCount = 0;
    alignas(16)
        SpotLightShadowDataLayout SpotLightsShadowData[DirectLightShadowData::sMaxSpotLightShadowDataCount] = {};
};

struct GeneralPurposeLayout
{
    float WorldCameraPos[3] = {};
    float Padding0 = 0.0f;
    float CameraForward[3] = {};
    float Padding1 = 1.0f;
};

struct EnvironmentBakeDataLayout
{
    float SampleDelta = 0.0f;
    int32_t SampleCount = 0;
    float Roughness = 0.0f;
};

struct WorldGizmoDataLayout
{
    float GizmoGapSize = 10.0f;
    float GizmoLineWidth = 0.2f;
    float GizmoFadeStartDist = 100.0f;
    float GizmoFadeEndDist = 600.0f;
};

struct InstanceData
{
    Matrix4x4 WorldMat;
};

struct MorphControlData
{
    static const int32_t sMaxSubMeshMorphTargetCount = 8;

    // Index of the specific SubMesh within the mesh that this morph control applies to.
    // WARN: This member is used as the base morph target offset within the flattened array inside the shader.
    //       Therefore, this value must be translated to the actual target offset during draw command submission.
    int32_t SubMeshIndex = 0;

    // Total number of active morph targets currently being applied to this SubMesh.
    int32_t TargetCount = 0;

    // Array of (local target index, blending weight) tuples within the SubMesh to be blended.
    int32_t TargetIndex[sMaxSubMeshMorphTargetCount] = {};
    float Weight[sMaxSubMeshMorphTargetCount] = {};
};

} // namespace ho