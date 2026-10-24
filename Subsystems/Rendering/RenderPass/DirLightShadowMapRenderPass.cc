#include "DirLightShadowMapRenderPass.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderingBuiltInAssets.h"

namespace ho
{
void DirLightShadowMapRenderPass::Setup(const DirLightShadowMapRenderPassDesc& desc)
{
    IRenderPass::Setup(desc.LightWorldPos);

    mPassDesc = desc;
}

void DirLightShadowMapRenderPass::PrepareExecute(const RenderResourceContext& resourceContext)
{
    if (!mbSetup)
    {
        return;
    }

    IRenderPass::PrepareExecute(resourceContext);

    mpBackend->SetDepthTest(true);
    mpBackend->SetDepthWrite(true);

    mpBackend->SetStencilTest(false);

    mpBackend->BindFrameBuffer(resourceContext.hDirLightShadowMapFrameBuffer);
    const int32_t fbWidth = resourceContext.hDirLightShadowMapFrameBuffer->Width;
    const int32_t fbHeight = resourceContext.hDirLightShadowMapFrameBuffer->Height;
    mpBackend->SetViewport(Rect(0.0f, 0.0f, static_cast<float>(fbWidth), static_cast<float>(fbHeight)));

    mpBackend->AttachDepthTargetRange(resourceContext.hDirLightShadowMapFrameBuffer,
                                      resourceContext.hDirLightDynamicShadowMap,
                                      mPassDesc.ShadowDataIndex * 6,
                                      mPassDesc.CascadeCount,
                                      0);

    mpBackend->ClearDepthTarget(resourceContext.hDirLightShadowMapFrameBuffer, 1.0f);

    mpBackend->BindShader(mpBuiltInAssets->hDirLightShadowMapGS, eShaderStage::GeometryShader);

    ShadowMapRenderDataLayout shadowDataLayout;
    shadowDataLayout.CascadeCount = mPassDesc.CascadeCount;
    for (int32_t i = 0; i < mPassDesc.CascadeCount; ++i)
    {
        shadowDataLayout.ProjViewMats[i] = mPassDesc.LightProjViewMats[i];
    }

    mpBackend->UploadBufferData(resourceContext.hShadowMapRenderDataBuffer,
                                &shadowDataLayout,
                                static_cast<int32_t>(sizeof(ShadowMapRenderDataLayout)));
    mpBackend->BindBuffer(resourceContext.hShadowMapRenderDataBuffer, eBufferBindingIndex::ShadowMapRenderData);
}

void DirLightShadowMapRenderPass::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderPass::FinishExecute(resourceContext);
    mpBackend->BindShader(GpuShaderHandle::sNULL, eShaderStage::GeometryShader);
    mpBackend->Flush();
}

void DirLightShadowMapRenderPass::TearDown()
{
    IRenderPass::TearDown();
}
} // namespace ho