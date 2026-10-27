#pragma once

#include "../RenderData/GpuResources.h"

namespace ho
{
struct RenderResourceContext
{
    GpuBufferHandle hMatrixBuffer = GpuBufferHandle::sNULL;
    GpuBufferHandle hDirectLightBuffer = GpuBufferHandle::sNULL;
    GpuBufferHandle hDirectLightShadowDataBuffer = GpuBufferHandle::sNULL;
    GpuBufferHandle hIndirectLightBuffer = GpuBufferHandle::sNULL;
    GpuBufferHandle hGeneralPurposeBuffer = GpuBufferHandle::sNULL;
    GpuBufferHandle hWorldGizmoDataBuffer = GpuBufferHandle::sNULL;
    GpuBufferHandle hShadowMapRenderDataBuffer = GpuBufferHandle::sNULL;
    GpuBufferHandle hEnvironmentBakeDataBuffer = GpuBufferHandle::sNULL;

    GpuBufferHandle hInstanceDataBuffer = GpuBufferHandle::sNULL;
    // Dummy morph control data has 1 MorphControlData that morph target count is 0.
    GpuBufferHandle hDummyMorphControlDataBuffer = GpuBufferHandle::sNULL;
    GpuBufferHandle hMorphControlDataBuffer = GpuBufferHandle::sNULL;

    FrameBufferHandle hDirLightShadowMapFrameBuffer = FrameBufferHandle::sNULL;
    GpuTextureHandle hDirLightDynamicShadowMap = GpuTextureHandle::sNULL;

    FrameBufferHandle hPointLightShadowMapFrameBuffer = FrameBufferHandle::sNULL;
    GpuTextureHandle hPointLightDynamicShadowMap = GpuTextureHandle::sNULL;
    GpuTextureHandle hPointLightShadowMapDummyDepthBuffer = GpuTextureHandle::sNULL;

    FrameBufferHandle hSpotLightShadowMapFrameBuffer = FrameBufferHandle::sNULL;
    GpuTextureHandle hSpotLightDynamicShadowMap = GpuTextureHandle::sNULL;
    GpuTextureHandle hSpotLightShadowMapDummyDepthBuffer = GpuTextureHandle::sNULL;
};
} // namespace ho