#include "ForwardRenderPass.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderingBuiltInAssets.h"

namespace ho
{
void ForwardRenderPass::Setup(FrameBufferHandle hFrameBuffer, const ForwardRenderPassDesc& desc)
{
    IRenderPass::Setup(desc.WorldCameraPos);

    HO_ASSERT(hFrameBuffer.IsValid(), "Invalid frame buffer.");
    HO_ASSERT(
        hFrameBuffer->ColorTargetStates[static_cast<int32_t>(eRenderTargetBindingIndex::Final)].hTarget.IsValid() &&
            hFrameBuffer->ColorTargetStates[static_cast<int32_t>(eRenderTargetBindingIndex::Accumulation)]
                .hTarget.IsValid(),
        "Frame buffer doesn't have valid render targets for forward rendering.");
    mhFrameBuffer = hFrameBuffer;

    HO_ASSERT(desc.Viewport.Width > 0.0f && desc.Viewport.Height > 0.0f, "Invalid viewport");

    mPassDesc = desc;
}

void ForwardRenderPass::PrepareExecute(const RenderResourceContext& resourceContext)
{
    if (!mbSetup)
    {
        return;
    }

    IRenderPass::PrepareExecute(resourceContext);

    mpBackend->SetDepthTest(true);
    mpBackend->SetDepthWrite(true);

    mpBackend->SetStencilTest(true);
    mpBackend->SetStencilWriteMask(0xFF);

    mpBackend->BindFrameBuffer(mhFrameBuffer);
    mpBackend->SetViewport(mPassDesc.Viewport);

    mpBackend->ClearColorTarget(mhFrameBuffer, eRenderTargetBindingIndex::Final, &mPassDesc.ClearColor.R);

    if (mPassDesc.bClearColorTarget)
    {
        mpBackend->ClearColorTarget(mhFrameBuffer, eRenderTargetBindingIndex::Accumulation, &mPassDesc.ClearColor.R);
    }

    if (mPassDesc.bClearDepthTarget && mPassDesc.bClearStencilTarget)
    {
        mpBackend->ClearDepthStencilTarget(mhFrameBuffer, mPassDesc.ClearDepth, mPassDesc.ClearStencil);
    }
    else if (mPassDesc.bClearDepthTarget)
    {
        mpBackend->ClearDepthTarget(mhFrameBuffer, mPassDesc.ClearDepth);
    }
    else if (mPassDesc.bClearStencilTarget)
    {
        mpBackend->ClearStencilTarget(mhFrameBuffer, mPassDesc.ClearStencil);
    }

    MatrixLayout matLayout;
    matLayout.ViewMat = mPassDesc.ViewMat;
    matLayout.ProjMat = mPassDesc.ProjMat;
    mpBackend->UploadBufferData(resourceContext.hMatrixBuffer, &matLayout, static_cast<int32_t>(sizeof(MatrixLayout)));
    mpBackend->BindBuffer(resourceContext.hMatrixBuffer, eBufferBindingIndex::Matrices);

    DirectLightLayout directLightLayout;

    for (int32_t i = 0; i < mPassDesc.DirectLightsData.DirLightCount; ++i)
    {
        directLightLayout.DirLights[i].Color[0] = mPassDesc.DirectLightsData.DirLights[i].Color.R;
        directLightLayout.DirLights[i].Color[1] = mPassDesc.DirectLightsData.DirLights[i].Color.G;
        directLightLayout.DirLights[i].Color[2] = mPassDesc.DirectLightsData.DirLights[i].Color.B;

        directLightLayout.DirLights[i].Direction[0] = mPassDesc.DirectLightsData.DirLights[i].Direction.X;
        directLightLayout.DirLights[i].Direction[1] = mPassDesc.DirectLightsData.DirLights[i].Direction.Y;
        directLightLayout.DirLights[i].Direction[2] = mPassDesc.DirectLightsData.DirLights[i].Direction.Z;

        directLightLayout.DirLights[i].Intensity = mPassDesc.DirectLightsData.DirLights[i].Intensity;
        directLightLayout.DirLights[i].ShadowDataIndex = mPassDesc.DirectLightsData.DirLights[i].ShadowDataIndex;
    }
    directLightLayout.DirLightCount = mPassDesc.DirectLightsData.DirLightCount;
    for (int32_t i = 0; i < mPassDesc.DirectLightsData.PointLightCount; ++i)
    {
        directLightLayout.PointLights[i].Color[0] = mPassDesc.DirectLightsData.PointLights[i].Color.R;
        directLightLayout.PointLights[i].Color[1] = mPassDesc.DirectLightsData.PointLights[i].Color.G;
        directLightLayout.PointLights[i].Color[2] = mPassDesc.DirectLightsData.PointLights[i].Color.B;

        directLightLayout.PointLights[i].Position[0] = mPassDesc.DirectLightsData.PointLights[i].Position.X;
        directLightLayout.PointLights[i].Position[1] = mPassDesc.DirectLightsData.PointLights[i].Position.Y;
        directLightLayout.PointLights[i].Position[2] = mPassDesc.DirectLightsData.PointLights[i].Position.Z;

        directLightLayout.PointLights[i].Intensity = mPassDesc.DirectLightsData.PointLights[i].Intensity;

        directLightLayout.PointLights[i].Constant = mPassDesc.DirectLightsData.PointLights[i].Constant;
        directLightLayout.PointLights[i].Linear = mPassDesc.DirectLightsData.PointLights[i].Linear;
        directLightLayout.PointLights[i].Quadratic = mPassDesc.DirectLightsData.PointLights[i].Quadratic;

        directLightLayout.PointLights[i].FarDist = mPassDesc.DirectLightsData.PointLights[i].FarDist;
        directLightLayout.PointLights[i].ShadowDataIndex = mPassDesc.DirectLightsData.PointLights[i].ShadowDataIndex;
    }
    directLightLayout.PointLightCount = mPassDesc.DirectLightsData.PointLightCount;
    for (int32_t i = 0; i < mPassDesc.DirectLightsData.SpotLightCount; ++i)
    {
        directLightLayout.SpotLights[i].Color[0] = mPassDesc.DirectLightsData.SpotLights[i].Color.R;
        directLightLayout.SpotLights[i].Color[1] = mPassDesc.DirectLightsData.SpotLights[i].Color.G;
        directLightLayout.SpotLights[i].Color[2] = mPassDesc.DirectLightsData.SpotLights[i].Color.B;

        directLightLayout.SpotLights[i].Position[0] = mPassDesc.DirectLightsData.SpotLights[i].Position.X;
        directLightLayout.SpotLights[i].Position[1] = mPassDesc.DirectLightsData.SpotLights[i].Position.Y;
        directLightLayout.SpotLights[i].Position[2] = mPassDesc.DirectLightsData.SpotLights[i].Position.Z;

        directLightLayout.SpotLights[i].Direction[0] = mPassDesc.DirectLightsData.SpotLights[i].Direction.X;
        directLightLayout.SpotLights[i].Direction[1] = mPassDesc.DirectLightsData.SpotLights[i].Direction.Y;
        directLightLayout.SpotLights[i].Direction[2] = mPassDesc.DirectLightsData.SpotLights[i].Direction.Z;

        directLightLayout.SpotLights[i].Intensity = mPassDesc.DirectLightsData.SpotLights[i].Intensity;

        directLightLayout.SpotLights[i].InnerCutoff = mPassDesc.DirectLightsData.SpotLights[i].InnerCutoff;
        directLightLayout.SpotLights[i].OuterCutoff = mPassDesc.DirectLightsData.SpotLights[i].OuterCutoff;

        directLightLayout.SpotLights[i].Constant = mPassDesc.DirectLightsData.SpotLights[i].Constant;
        directLightLayout.SpotLights[i].Linear = mPassDesc.DirectLightsData.SpotLights[i].Linear;
        directLightLayout.SpotLights[i].Quadratic = mPassDesc.DirectLightsData.SpotLights[i].Quadratic;

        directLightLayout.SpotLights[i].FarDist = mPassDesc.DirectLightsData.SpotLights[i].FarDist;
        directLightLayout.SpotLights[i].ShadowDataIndex = mPassDesc.DirectLightsData.SpotLights[i].ShadowDataIndex;
    }
    directLightLayout.SpotLightCount = mPassDesc.DirectLightsData.SpotLightCount;

    mpBackend->UploadBufferData(
        resourceContext.hDirectLightBuffer, &directLightLayout, static_cast<int32_t>(sizeof(DirectLightLayout)));
    mpBackend->BindBuffer(resourceContext.hDirectLightBuffer, eBufferBindingIndex::DirectLights);

    DirectLightShadowDataLayout shadowDataLayout;
    for (int32_t i = 0; i < mPassDesc.DirectLightsShadowData.DirLightShadowDataCount; ++i)
    {
        shadowDataLayout.DirLightsShadowData[i].CascadeCount =
            mPassDesc.DirectLightsShadowData.DirLightsShadowData[i].CascadeCount;
        for (int32_t j = 0; j < 6; ++j)
        {
            shadowDataLayout.DirLightsShadowData[i].CascadeNormalOffsetScales[j] =
                mPassDesc.DirectLightsShadowData.DirLightsShadowData[i].CascadeNormalOffsetScales[j];
            shadowDataLayout.DirLightsShadowData[i].CascadeDepthOffsetScales[j] =
                mPassDesc.DirectLightsShadowData.DirLightsShadowData[i].CascadeDepthOffsetScales[j];
            shadowDataLayout.DirLightsShadowData[i].CascadeFarDist[j] =
                mPassDesc.DirectLightsShadowData.DirLightsShadowData[i].CascadeFarDist[j];
            shadowDataLayout.DirLightsShadowData[i].ProjViewMats[j] =
                mPassDesc.DirectLightsShadowData.DirLightsShadowData[i].ProjViewMats[j];
        }
    }
    shadowDataLayout.DirLightShadowDataCount = mPassDesc.DirectLightsShadowData.DirLightShadowDataCount;
    for (int32_t i = 0; i < mPassDesc.DirectLightsShadowData.PointLightShadowDataCount; ++i)
    {
        shadowDataLayout.PointLightsShadowData[i].LightWorldPos[0] =
            mPassDesc.DirectLightsShadowData.PointLightsShadowData[i].LightWorldPos.X;
        shadowDataLayout.PointLightsShadowData[i].LightWorldPos[1] =
            mPassDesc.DirectLightsShadowData.PointLightsShadowData[i].LightWorldPos.Y;
        shadowDataLayout.PointLightsShadowData[i].LightWorldPos[2] =
            mPassDesc.DirectLightsShadowData.PointLightsShadowData[i].LightWorldPos.Z;
        shadowDataLayout.PointLightsShadowData[i].FarDist =
            mPassDesc.DirectLightsShadowData.PointLightsShadowData[i].FarDist;
        for (int32_t j = 0; j < 6; ++j)
        {
            shadowDataLayout.PointLightsShadowData[i].ProjViewMats[j] =
                mPassDesc.DirectLightsShadowData.PointLightsShadowData[i].ProjViewMats[j];
        }
    }
    shadowDataLayout.PointLightShadowDataCount = mPassDesc.DirectLightsShadowData.PointLightShadowDataCount;
    for (int32_t i = 0; i < mPassDesc.DirectLightsShadowData.SpotLightShadowDataCount; ++i)
    {
        shadowDataLayout.SpotLightsShadowData[i].LightWorldPos[0] =
            mPassDesc.DirectLightsShadowData.SpotLightsShadowData[i].LightWorldPos.X;
        shadowDataLayout.SpotLightsShadowData[i].LightWorldPos[1] =
            mPassDesc.DirectLightsShadowData.SpotLightsShadowData[i].LightWorldPos.Y;
        shadowDataLayout.SpotLightsShadowData[i].LightWorldPos[2] =
            mPassDesc.DirectLightsShadowData.SpotLightsShadowData[i].LightWorldPos.Z;
        shadowDataLayout.SpotLightsShadowData[i].FarDist =
            mPassDesc.DirectLightsShadowData.SpotLightsShadowData[i].FarDist;
        shadowDataLayout.SpotLightsShadowData[i].ProjViewMat =
            mPassDesc.DirectLightsShadowData.SpotLightsShadowData[i].ProjViewMat;
    }
    shadowDataLayout.SpotLightShadowDataCount = mPassDesc.DirectLightsShadowData.SpotLightShadowDataCount;

    mpBackend->UploadBufferData(resourceContext.hDirectLightShadowDataBuffer,
                                &shadowDataLayout,
                                static_cast<int32_t>(sizeof(DirectLightShadowDataLayout)));
    mpBackend->BindBuffer(resourceContext.hDirectLightShadowDataBuffer, eBufferBindingIndex::DirectLightShadowData);

    mpBackend->ResourceBarrier(resourceContext.hDirLightDynamicShadowMap,
                               IRenderingBackend::eResourceState::DepthStencilWrite,
                               IRenderingBackend::eResourceState::ShaderResource);
    mpBackend->BindTexture(resourceContext.hDirLightDynamicShadowMap, eTextureBindingIndex::DirLightDynamicShadowMap);
    mpBackend->ResourceBarrier(resourceContext.hPointLightDynamicShadowMap,
                               IRenderingBackend::eResourceState::DepthStencilWrite,
                               IRenderingBackend::eResourceState::ShaderResource);
    mpBackend->BindTexture(resourceContext.hPointLightDynamicShadowMap,
                           eTextureBindingIndex::PointLightDynamicShadowMap);
    mpBackend->ResourceBarrier(resourceContext.hSpotLightDynamicShadowMap,
                               IRenderingBackend::eResourceState::DepthStencilWrite,
                               IRenderingBackend::eResourceState::ShaderResource);
    mpBackend->BindTexture(resourceContext.hSpotLightDynamicShadowMap, eTextureBindingIndex::SpotLightDynamicShadowMap);

    const int32_t dirLightStaticShadowMap0 = static_cast<int32_t>(eTextureBindingIndex::DirLightStaticShadowMap0);
    for (int32_t i = 0; i < mPassDesc.DirectLightsShadowData.DirLightShadowDataCount; ++i)
    {
        mpBackend->BindTexture(mPassDesc.DirectLightsShadowData.DirLightsShadowData[i].hStaticShadowMap,
                               static_cast<eTextureBindingIndex>(dirLightStaticShadowMap0 + i));
    }
    const int32_t pointLightStaticShadowMap0 = static_cast<int32_t>(eTextureBindingIndex::PointLightStaticShadowMap0);
    for (int32_t i = 0; i < mPassDesc.DirectLightsShadowData.PointLightShadowDataCount; ++i)
    {
        mpBackend->BindTexture(mPassDesc.DirectLightsShadowData.PointLightsShadowData[i].hStaticShadowMap,
                               static_cast<eTextureBindingIndex>(pointLightStaticShadowMap0 + i));
    }
    const int32_t spotLightStaticShadowMap0 = static_cast<int32_t>(eTextureBindingIndex::SpotLightStaticShadowMap0);
    for (int32_t i = 0; i < mPassDesc.DirectLightsShadowData.SpotLightShadowDataCount; ++i)
    {
        mpBackend->BindTexture(mPassDesc.DirectLightsShadowData.SpotLightsShadowData[i].hStaticShadowMap,
                               static_cast<eTextureBindingIndex>(spotLightStaticShadowMap0 + i));
    }

    GpuTextureHandle hIrrMap = mPassDesc.hIrradianceMap;
    if (hIrrMap.IsNULL())
    {
        hIrrMap = mpBuiltInAssets->hFallbackCubeMap;
    }
    else if (hIrrMap->State == eGpuResourceState::PendingUpload)
    {
        hIrrMap = mpBuiltInAssets->hCheckerBoardCubeMap;
    }
    mpBackend->BindTexture(hIrrMap, eTextureBindingIndex::IrradianceMap);

    GpuTextureHandle hEnvMap = mPassDesc.hPrefilteredEnvMap;
    if (hEnvMap.IsNULL())
    {
        hEnvMap = mpBuiltInAssets->hFallbackCubeMap;
    }
    else if (hEnvMap->State == eGpuResourceState::PendingUpload)
    {
        hEnvMap = mpBuiltInAssets->hCheckerBoardCubeMap;
    }
    mpBackend->BindTexture(hEnvMap, eTextureBindingIndex::PrefilteredEnvironmentMap);
    mpBackend->BindTexture(mpBuiltInAssets->hBRDFIntegrationMap, eTextureBindingIndex::BRDFIntegrationMap);

    IndirectLightLayout indirectLightLayout;
    indirectLightLayout.Intensity = mPassDesc.IndirectLights.Intensity;
    mpBackend->UploadBufferData(
        resourceContext.hIndirectLightBuffer, &indirectLightLayout, static_cast<int32_t>(sizeof(IndirectLightLayout)));
    mpBackend->BindBuffer(resourceContext.hIndirectLightBuffer, eBufferBindingIndex::IndirectLights);

    GeneralPurposeLayout genLayout;
    genLayout.WorldCameraPos[0] = mCameraWorldPos.X;
    genLayout.WorldCameraPos[1] = mCameraWorldPos.Y;
    genLayout.WorldCameraPos[2] = mCameraWorldPos.Z;
    const Vector3 cameraForward(mPassDesc.ViewMat.GetCol2().Normalized());
    genLayout.CameraForward[0] = cameraForward.X;
    genLayout.CameraForward[1] = cameraForward.Y;
    genLayout.CameraForward[2] = cameraForward.Z;
    mpBackend->UploadBufferData(
        resourceContext.hGeneralPurposeBuffer, &genLayout, static_cast<int32_t>(sizeof(GeneralPurposeLayout)));
    mpBackend->BindBuffer(resourceContext.hGeneralPurposeBuffer, eBufferBindingIndex::GeneralPurpose);
}

void ForwardRenderPass::Execute(const RenderResourceContext& resourceContext)
{
    if (mhFrameBuffer.IsNULL())
    {
        return;
    }

    IRenderPass::Execute(resourceContext);
}

void ForwardRenderPass::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderPass::FinishExecute(resourceContext);
    mpBackend->SetDepthTest(false);
    mpBackend->SetStencilTest(false);
    mpBackend->Flush();
}

void ForwardRenderPass::TearDown()
{
    IRenderPass::TearDown();
}
} // namespace ho