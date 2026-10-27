#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Core/Memory/WeakTypedHandle.h"

namespace ho
{
struct GpuTexture;
using GpuTextureHandle = WeakTypedHandle<GpuTexture, TypedObjectPool>;

struct DirLightShadowData
{
    static const int32_t sMaxCascadeCount = 6;

    int32_t CascadeCount = 0;
    float CascadeNormalOffsetScales[sMaxCascadeCount] = {};
    float CascadeDepthOffsetScales[sMaxCascadeCount] = {};
    float CascadeFarDist[sMaxCascadeCount] = {};
    Matrix4x4 ProjViewMats[sMaxCascadeCount] = {};

    GpuTextureHandle hStaticShadowMap;
};

struct PointLightShadowData
{
    Vector3 LightWorldPos;
    float NormalOffsetScale = 0.0f;
    float DepthOffsetScale = 0.0f;
    float FarDist = 0.0f;
    Matrix4x4 ProjViewMats[6] = {};

    GpuTextureHandle hStaticShadowMap;
};

struct SpotLightShadowData
{
    Vector3 LightWorldPos;
    float NormalOffsetScale = 0.0f;
    float DepthOffsetScale = 0.0f;
    float FarDist = 0.0f;
    Matrix4x4 ProjViewMat;

    GpuTextureHandle hStaticShadowMap;
};

struct DirectLightShadowData
{
    static const int32_t sMaxDirLightShadowDataCount = 1;
    static const int32_t sMaxPointLightShadowDataCount = 4;
    static const int32_t sMaxSpotLightShadowDataCount = 8;

    int32_t DirLightShadowDataCount = 0;
    DirLightShadowData DirLightsShadowData[sMaxDirLightShadowDataCount] = {};

    int32_t PointLightShadowDataCount = 0;
    PointLightShadowData PointLightsShadowData[sMaxPointLightShadowDataCount] = {};

    int32_t SpotLightShadowDataCount = 0;
    SpotLightShadowData SpotLightsShadowData[sMaxSpotLightShadowDataCount] = {};
};
} // namespace ho