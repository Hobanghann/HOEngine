#pragma once

#include "../RenderData/GpuResources.h"
#include "../RenderData/LightData.h"
#include "../RenderData/LightShadowData.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Rect.h"

namespace ho
{
struct ForwardRenderPassDesc
{
    StringHandle hFrameBufferName;

    bool bClearColorTarget = true;
    Color128 ClearColor = Color128(1.0f, 1.0f, 1.0f, 1.0f);

    bool bClearDepthTarget = true;
    bool bClearStencilTarget = true;
    float ClearDepth = 1.0f;
    int32_t ClearStencil = 0;

    Rect Viewport;

    Vector3 WorldCameraPos = Vector3::sZero;
    Matrix4x4 ViewMat = Matrix4x4::sIdentity;
    Matrix4x4 ProjMat = Matrix4x4::sIdentity;

    DirectLightData DirectLightsData;
    DirectLightShadowData DirectLightsShadowData;

    GpuTextureHandle hIrradianceMap;
    GpuTextureHandle hPrefilteredEnvMap;
    IndirectLightData IndirectLights;
};
} // namespace ho