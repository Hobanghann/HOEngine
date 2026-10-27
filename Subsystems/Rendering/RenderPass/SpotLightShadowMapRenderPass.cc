#include "SpotLightShadowMapRenderPass.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderingBuiltInAssets.h"
#include "Core/Math/Projection.h"
#include "Core/Math/Transform3D.h"
#include "SpotLightShadowMapRenderPassDesc.h"

namespace ho
{
void SpotLightShadowMapRenderPass::Setup(const SpotLightShadowMapRenderPassDesc& desc)
{
    IRenderPass::Setup(desc.LightWorldPos);

    mPassDesc = desc;
}

void SpotLightShadowMapRenderPass::PrepareExecute(const RenderResourceContext& resourceContext)
{
    if (!mbSetup)
    {
        return;
    }

    IRenderPass::PrepareExecute(resourceContext);

    mpBackend->SetDepthTest(true);
    mpBackend->SetDepthWrite(true);

    mpBackend->SetStencilTest(false);

    mpBackend->BindFrameBuffer(resourceContext.hSpotLightShadowMapFrameBuffer);
    const int32_t fbWidth = resourceContext.hSpotLightShadowMapFrameBuffer->Width;
    const int32_t fbHeight = resourceContext.hSpotLightShadowMapFrameBuffer->Height;
    mpBackend->SetViewport(Rect(0.0f, 0.0f, static_cast<float>(fbWidth), static_cast<float>(fbHeight)));

    float clearColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    mpBackend->AttachColorTargetRange(resourceContext.hSpotLightShadowMapFrameBuffer,
                                      eRenderTargetBindingIndex::Final,
                                      resourceContext.hSpotLightDynamicShadowMap,
                                      mPassDesc.ShadowDataIndex,
                                      1,
                                      0);
    mpBackend->ClearColorTarget(
        resourceContext.hSpotLightShadowMapFrameBuffer, eRenderTargetBindingIndex::Final, clearColor);

    mpBackend->AttachDepthTargetRange(resourceContext.hSpotLightShadowMapFrameBuffer,
                                      resourceContext.hSpotLightShadowMapDummyDepthBuffer,
                                      mPassDesc.ShadowDataIndex,
                                      1,
                                      0);

    mpBackend->ClearDepthTarget(resourceContext.hSpotLightShadowMapFrameBuffer, 1.0f);

    ShadowMapRenderDataLayout shadowDataLayout;
    shadowDataLayout.LightWorldPos[0] = mCameraWorldPos.X;
    shadowDataLayout.LightWorldPos[1] = mCameraWorldPos.Y;
    shadowDataLayout.LightWorldPos[2] = mCameraWorldPos.Z;
    shadowDataLayout.FarDist = mPassDesc.FarDist;
    shadowDataLayout.ProjViewMats[0] = mPassDesc.LightProjViewMat;

    mpBackend->UploadBufferData(resourceContext.hShadowMapRenderDataBuffer,
                                &shadowDataLayout,
                                static_cast<int32_t>(sizeof(ShadowMapRenderDataLayout)));
    mpBackend->BindBuffer(resourceContext.hShadowMapRenderDataBuffer, eBufferBindingIndex::ShadowMapRenderData);
}

void SpotLightShadowMapRenderPass::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderPass::FinishExecute(resourceContext);
    mpBackend->Flush();
}

void SpotLightShadowMapRenderPass::TearDown()
{
    IRenderPass::TearDown();
}
} // namespace ho