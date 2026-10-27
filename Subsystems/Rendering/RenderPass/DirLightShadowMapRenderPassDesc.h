#pragma once

#include "Core/Math/Matrix4x4.h"

namespace ho
{
struct DirLightShadowMapRenderPassDesc
{
    int32_t ShadowDataIndex = 0;
    Vector3 LightWorldPos;
    int32_t CascadeCount = 0;
    Matrix4x4 LightProjViewMats[6] = {};
};
} // namespace ho