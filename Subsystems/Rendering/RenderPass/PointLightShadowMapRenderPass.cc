#include "PointLightShadowMapRenderPass.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderingBuiltInAssets.h"
#include "Core/Math/Projection.h"
#include "Core/Math/Transform3D.h"

namespace ho
{
void PointLightShadowMapRenderPass::Setup(const PointLightShadowMapRenderPassDesc& desc)
{
    IRenderPass::Setup(desc.LightWorldPos);

    mPassDesc = desc;
}

void PointLightShadowMapRenderPass::PrepareExecute(const RenderResourceContext& resourceContext)
{
    if (!mbSetup)
    {
        return;
    }

    IRenderPass::PrepareExecute(resourceContext);

    mpBackend->SetDepthTest(true);
    mpBackend->SetDepthWrite(true);

    mpBackend->SetStencilTest(false);

    mpBackend->BindFrameBuffer(resourceContext.hPointLightShadowMapFrameBuffer);
    const int32_t fbWidth = resourceContext.hPointLightShadowMapFrameBuffer->Width;
    const int32_t fbHeight = resourceContext.hPointLightShadowMapFrameBuffer->Height;
    mpBackend->SetViewport(Rect(0.0f, 0.0f, static_cast<float>(fbWidth), static_cast<float>(fbHeight)));

    float clearColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    mpBackend->AttachColorTargetRange(resourceContext.hPointLightShadowMapFrameBuffer,
                                      eRenderTargetBindingIndex::Final,
                                      resourceContext.hPointLightDynamicShadowMap,
                                      6 * mPassDesc.ShadowDataIndex,
                                      6,
                                      0);
    mpBackend->ClearColorTarget(
        resourceContext.hPointLightShadowMapFrameBuffer, eRenderTargetBindingIndex::Final, clearColor);
    mpBackend->AttachDepthTargetRange(resourceContext.hPointLightShadowMapFrameBuffer,
                                      resourceContext.hPointLightShadowMapDummyDepthBuffer,
                                      6 * mPassDesc.ShadowDataIndex,
                                      6,
                                      0);
    mpBackend->ClearDepthTarget(resourceContext.hPointLightShadowMapFrameBuffer, 1.0f);

    mpBackend->BindShader(mpBuiltInAssets->hPointLightShadowMapGS, eShaderStage::GeometryShader);

    ShadowMapRenderDataLayout shadowDataLayout;
    shadowDataLayout.LightWorldPos[0] = mPassDesc.LightWorldPos.X;
    shadowDataLayout.LightWorldPos[1] = mPassDesc.LightWorldPos.Y;
    shadowDataLayout.LightWorldPos[2] = mPassDesc.LightWorldPos.Z;
    shadowDataLayout.FarDist = mPassDesc.FarDist;

    for (int32_t i = 0; i < 6; ++i)
    {
        shadowDataLayout.ProjViewMats[i] = mPassDesc.LightProjViewMats[i];
    }

    mpBackend->UploadBufferData(resourceContext.hShadowMapRenderDataBuffer,
                                &shadowDataLayout,
                                static_cast<int32_t>(sizeof(ShadowMapRenderDataLayout)));
    mpBackend->BindBuffer(resourceContext.hShadowMapRenderDataBuffer, eBufferBindingIndex::ShadowMapRenderData);
}

void PointLightShadowMapRenderPass::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderPass::FinishExecute(resourceContext);
    mpBackend->BindShader(GpuShaderHandle::sNULL, eShaderStage::GeometryShader);
    mpBackend->Flush();
}

void PointLightShadowMapRenderPass::TearDown()
{
    IRenderPass::TearDown();
}
} // namespace ho