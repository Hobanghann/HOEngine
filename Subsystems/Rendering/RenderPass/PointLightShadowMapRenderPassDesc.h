#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector3.h"

namespace ho
{
struct PointLightShadowMapRenderPassDesc
{
    int32_t ShadowDataIndex = 0;
    Vector3 LightWorldPos;
    float FarDist = 0.0f;
    Matrix4x4 LightProjViewMats[6] = {};
};
} // namespace ho