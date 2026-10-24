#include "ShadowMapBakePass.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderData/LightShadowData.h"
#include "../RenderingBuiltInAssets.h"
#include "Core/Math/Rect.h"

namespace ho
{
void ShadowMapBakePass::Setup(const ShadowMapBakePassDesc& desc)
{
    IRenderPass::Setup(desc.LightWorldPos);

    mPassDesc = desc;
}

void ShadowMapBakePass::PrepareExecute(const RenderResourceContext& resourceContext)
{
    if (!mbSetup)
    {
        return;
    }

    IRenderPass::PrepareExecute(resourceContext);

    mpBackend->SetDepthTest(true);
    mpBackend->SetDepthWrite(true);

    mpBackend->SetStencilTest(false);

    switch (mPassDesc.ShadowMapType)
    {
        case eShadowMapType::PointLightShadowMap:
        {
            mpBackend->BindFrameBuffer(resourceContext.hPointLightShadowMapFrameBuffer);
            const int32_t fbWidth = resourceContext.hPointLightShadowMapFrameBuffer->Width;
            const int32_t fbHeight = resourceContext.hPointLightShadowMapFrameBuffer->Height;
            mpBackend->SetViewport(Rect(0.0f, 0.0f, static_cast<float>(fbWidth), static_cast<float>(fbHeight)));

            mhBakedTexture =
                mpBackend->PrepareCreateTextureCubeMap(eTextureFormat::R32_FLOAT, fbWidth, fbHeight, 1, false);
            mpBackend->CreateTexture(mhBakedTexture);

            float clearColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            mpBackend->AttachColorTargetRange(resourceContext.hPointLightShadowMapFrameBuffer,
                                              eRenderTargetBindingIndex::Final,
                                              mhBakedTexture,
                                              0,
                                              6,
                                              0);
            mpBackend->ClearColorTarget(
                resourceContext.hPointLightShadowMapFrameBuffer, eRenderTargetBindingIndex::Final, clearColor);

            mpBackend->AttachDepthTargetRange(resourceContext.hPointLightShadowMapFrameBuffer,
                                              resourceContext.hPointLightShadowMapDummyDepthBuffer,
                                              0,
                                              6,
                                              0);
            mpBackend->ClearDepthTarget(resourceContext.hPointLightShadowMapFrameBuffer, 1.0f);

            mpBackend->BindShader(mpBuiltInAssets->hPointLightShadowMapGS, eShaderStage::GeometryShader);
        }
        break;
        case eShadowMapType::SpotLightShadowMap:
        {
            mpBackend->BindFrameBuffer(resourceContext.hSpotLightShadowMapFrameBuffer);
            const int32_t fbWidth = resourceContext.hSpotLightShadowMapFrameBuffer->Width;
            const int32_t fbHeight = resourceContext.hSpotLightShadowMapFrameBuffer->Height;
            mpBackend->SetViewport(Rect(0.0f, 0.0f, static_cast<float>(fbWidth), static_cast<float>(fbHeight)));

            mhBakedTexture =
                mpBackend->PrepareCreateTexture2D(eTextureFormat::R32_FLOAT, fbWidth, fbHeight, 1, 1, false);
            mpBackend->CreateTexture(mhBakedTexture);

            float clearColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            mpBackend->AttachColorTarget(
                resourceContext.hSpotLightShadowMapFrameBuffer, eRenderTargetBindingIndex::Final, mhBakedTexture);
            mpBackend->ClearColorTarget(
                resourceContext.hSpotLightShadowMapFrameBuffer, eRenderTargetBindingIndex::Final, clearColor);

            mpBackend->AttachDepthTargetRange(resourceContext.hSpotLightShadowMapFrameBuffer,
                                              resourceContext.hSpotLightShadowMapDummyDepthBuffer,
                                              0,
                                              1,
                                              0);
            mpBackend->ClearDepthTarget(resourceContext.hSpotLightShadowMapFrameBuffer, 1.0f);
        }
        break;
        default:
            HO_ASSERT(false, "Invalid shadow map type.");
    }

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

void ShadowMapBakePass::Execute(const RenderResourceContext& resourceContext)
{
    IRenderPass::Execute(resourceContext);

    HO_ASSERT(mhBakedTexture.IsValid(), "Baked textrue was created incompletely.");

    mpBackend->DownloadTexture(mhBakedTexture, mPassDesc.hDstTexture);
}

void ShadowMapBakePass::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderPass::FinishExecute(resourceContext);
    mpBackend->BindShader(GpuShaderHandle::sNULL, eShaderStage::GeometryShader);
    mpBackend->UnloadTexture(mhBakedTexture);
    mpBackend->Flush();
}

void ShadowMapBakePass::TearDown()
{
    IRenderPass::TearDown();
}
} // namespace ho