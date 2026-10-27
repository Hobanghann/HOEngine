#pragma once

#include "Core/Math/Color128.h"
#include "Core/Math/Vector3.h"
#include "Core/Memory/WeakTypedHandle.h"

namespace ho
{
struct DirLightData
{
    Vector3 Direction;

    Color128 Color;
    float Intensity = 0.0f;
    // Index of DirectLightShadowData::DirLightsShadowData. -1 means no shadow.
    int32_t ShadowDataIndex = -1;
};

struct PointLightData
{
    Vector3 Position;

    Color128 Color;
    float Intensity = 0.0f;

    float Constant = 0.0f;
    float Linear = 0.0f;
    float Quadratic = 0.0f;

    float FarDist = 0.0f;
    // Index of DirectLightShadowData::PointLightsShadowData. -1 means no shadow.
    int32_t ShadowDataIndex = -1;
};

struct SpotLightData
{
    Vector3 Position;
    Vector3 Direction;

    Color128 Color;
    float Intensity = 0.0f;

    // Cosine of the inner half-angle representing the spotlight's coverage area
    float InnerCutoff = 0.0f;
    // Cosine of the outer half-angle representing the spotlight's coverage area
    float OuterCutoff = 0.0f;

    float Constant = 1.0f;
    float Linear = 0.045f;
    float Quadratic = 0.0075f;

    float FarDist = 0.0f;
    // Index of DirectLightShadowData::SpotLightsShadowData. -1 means no shadow.
    int32_t ShadowDataIndex = -1;
};

struct DirectLightData
{
    static const int32_t sMaxDirLightCount = 4;
    static const int32_t sMaxPointLightCount = 32;
    static const int32_t sMaxSpotLightCount = 16;

    int32_t DirLightCount = 0;
    DirLightData DirLights[sMaxDirLightCount] = {};

    int32_t PointLightCount = 0;
    PointLightData PointLights[sMaxPointLightCount] = {};

    int32_t SpotLightCount = 0;
    SpotLightData SpotLights[sMaxSpotLightCount] = {};
};

struct IndirectLightData
{
    float Intensity = 0.05f;
};
} // namespace ho