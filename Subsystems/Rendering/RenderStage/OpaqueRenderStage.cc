#include "OpaqueRenderStage.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderData/DrawCommand.h"
#include "../RenderingBuiltInAssets.h"
#include "Subsystems/Asset/MaterialAsset.h"
#include "Subsystems/Asset/TextureAsset.h"

namespace ho
{
void OpaqueRenderStage::Setup(const OpaqueRenderStageDesc& desc)
{
    IRenderStage::Setup();

    mStageDesc = desc;
}

void OpaqueRenderStage::PrepareExecute(const RenderResourceContext& resourceContext)
{
    if (!mbSetup)
    {
        return;
    }

    IRenderStage::PrepareExecute(resourceContext);

    mpBackend->SetCullMode(IRenderingBackend::eCullMode::Back);

    mpBackend->SetDepthTest(true);
    mpBackend->SetDepthWrite(true);
    mpBackend->SetDepthFunc(IRenderingBackend::eComparisonFunc::Less);

    mpBackend->SetStencilTest(true);
    mpBackend->SetStencilState(IRenderingBackend::eComparisonFunc::Always, 0xFF);
    mpBackend->SetStencilOperator(IRenderingBackend::eStencilOperator::Keep,
                                  IRenderingBackend::eStencilOperator::Replace,
                                  IRenderingBackend::eStencilOperator::Replace);

    mpBackend->SetAlphaBlendMode(IRenderingBackend::eAlphaBlendMode::None);
}

void OpaqueRenderStage::Execute(const RenderResourceContext& resourceContext)
{
    HO_ASSERT(isExecutable(), "Stage is not executable");

    GpuMaterialHandle hPrevRenderedMaterial = GpuMaterialHandle::sNULL;
    for (int32_t i = 0; i < mDrawCommandCount; ++i)
    {
        // Bind static mesh
        GpuStaticMeshHandle hStaticMesh;
        if (mStageDesc.hOverrideStaticMesh.IsValid())
        {
            hStaticMesh = mStageDesc.hOverrideStaticMesh;
        }
        else
        {
            hStaticMesh = mpDrawCommandsBegin[i].hGpuStaticMesh;
        }

        if (hStaticMesh.IsNULL())
        {
            hStaticMesh = mpBuiltInAssets->hOctahedronStaticMesh;
        }
        else if (hStaticMesh->State == eGpuResourceState::PendingUpload)
        {
            hStaticMesh = mpBuiltInAssets->hIcosaedroStaticMesh;
        }

        mpBackend->BindStaticMesh(hStaticMesh);

        if (hPrevRenderedMaterial != mpDrawCommandsBegin[i].hGpuMaterial)
        {
            GpuMaterialHandle hMat = mpDrawCommandsBegin[i].hGpuMaterial;

            if (mStageDesc.hAttributeOverrideMaterial.IsValid())
            {
                hMat = mStageDesc.hAttributeOverrideMaterial;
            }

            if (!hMat.IsValid() || hMat.IsNULL() || hMat->State == eGpuResourceState::PendingUpload)
            {
                hMat = mpBuiltInAssets->hDefaultMaterial;
            }

            // Construct pipeline state
            GpuMaterial::GpuPipelineState pipelineState = hMat->PipelineState;

            if (mStageDesc.bOverrideAlphaBlendMode)
            {
                pipelineState.AlphaMode = mStageDesc.OverridePipelineState.AlphaMode;
                pipelineState.AlphaBlendMode = mStageDesc.OverridePipelineState.AlphaBlendMode;
            }

            if (mStageDesc.bOverridePolygonMode)
            {
                pipelineState.bWireframe = mStageDesc.OverridePipelineState.bWireframe;
            }

            if (mStageDesc.bOverrideCullMode)
            {
                pipelineState.bBackfaceCulling = mStageDesc.OverridePipelineState.bBackfaceCulling;
            }

            // Apply pipeline state
            switch (pipelineState.AlphaMode)
            {
                case eMaterialAlphaMode::Opaque:
                case eMaterialAlphaMode::Mask:
                    mpBackend->SetAlphaBlendMode(IRenderingBackend::eAlphaBlendMode::None);
                    break;
                case eMaterialAlphaMode::Blend:
                    switch (pipelineState.AlphaBlendMode)
                    {
                        case eMaterialAlphaBlendMode::Default:
                            mpBackend->SetAlphaBlendMode(IRenderingBackend::eAlphaBlendMode::Default);
                            break;
                        case eMaterialAlphaBlendMode::Additive:
                            mpBackend->SetAlphaBlendMode(IRenderingBackend::eAlphaBlendMode::Additive);
                            break;
                        default:
                            HO_ASSERT(false, "Invalid alpha blend mode.");
                    }
                    break;
                default:
                    HO_ASSERT(false, "Invalid alpha mode.");
            }

            if (pipelineState.bWireframe)
            {
                mpBackend->SetPolygonMode(IRenderingBackend::ePolygonMode::Line);
            }
            else
            {
                mpBackend->SetPolygonMode(IRenderingBackend::ePolygonMode::Triangle);
            }

            if (pipelineState.bBackfaceCulling)
            {
                mpBackend->SetCullMode(IRenderingBackend::eCullMode::Back);
            }
            else
            {
                mpBackend->SetCullMode(IRenderingBackend::eCullMode::None);
            }

            // Bind material attributes
            mpBackend->BindMaterialAttributes(hMat);

            // Bind textures
            for (int32_t ti = 1; ti < static_cast<int32_t>(eMaterialTextureUsage::Last); ++ti)
            {
                if (!mStageDesc.bBindTexture[ti] || !hMat->bHasTexture[ti])
                {
                    continue;
                }

                GpuTextureHandle hTexture = hMat->hGpuTextures[ti];
                if (mStageDesc.hOverrideTextures[ti].IsValid())
                {
                    hTexture = mStageDesc.hOverrideTextures[ti];
                }

                if (!hTexture.IsValid() || hTexture.IsNULL())
                {
                    hTexture = mpBuiltInAssets->hFallbackTexture;
                }
                else if (hTexture->State == eGpuResourceState::PendingUpload)
                {
                    hTexture = mpBuiltInAssets->hCheckerBoardTexture;
                }

                mpBackend->BindTexture(hTexture, getTextureBindingIndex(static_cast<eMaterialTextureUsage>(ti)));
            }

            // Bind shaders
            GpuShaderHandle hVS = mpBuiltInAssets->hDefaultVS;
            if (mStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)].IsValid())
            {
                hVS = mStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::VertexShader)];
            }
            else if (hMat->hGpuShaders[static_cast<int32_t>(eShaderStage::VertexShader)].IsValid())
            {
                hVS = hMat->hGpuShaders[static_cast<int32_t>(eShaderStage::VertexShader)];
            }

            GpuShaderHandle hFS;
            switch (hMat->Type)
            {
                case eMaterialAssetType::Unlit:
                    hFS = mpBuiltInAssets->hForwardUnlitOpaqueFS;
                    break;
                case eMaterialAssetType::Legacy:
                    hFS = mpBuiltInAssets->hForwardPhongOpaqueFS;
                    break;
                case eMaterialAssetType::StandardLit:
                    hFS = mpBuiltInAssets->hForwardPbrOpaqueFS;
                    break;
                default:
                    HO_ASSERT(false, "Invalid material asset type.");
            }
            if (mStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)].IsValid())
            {
                hFS = mStageDesc.hOverrideShaders[static_cast<int32_t>(eShaderStage::FragmentShader)];
            }
            else if (hMat->hGpuShaders[static_cast<int32_t>(eShaderStage::FragmentShader)].IsValid())
            {
                hFS = hMat->hGpuShaders[static_cast<int32_t>(eShaderStage::FragmentShader)];
            }

            mpBackend->BindShader(hVS, eShaderStage::VertexShader);
            mpBackend->BindShader(hFS, eShaderStage::FragmentShader);

            hPrevRenderedMaterial = mpDrawCommandsBegin[i].hGpuMaterial;
        }

        // Bind morph targets
        if (mpDrawCommandsBegin[i].hGpuMorphTargets.IsValid() &&
            mpDrawCommandsBegin[i].hGpuMorphTargets->State != eGpuResourceState::PendingUpload)
        {
            mpBackend->BindBufferSlice(resourceContext.hMorphControlDataBuffer,
                                       eBufferBindingIndex::MorphControlData,
                                       static_cast<int32_t>(sizeof(MorphControlData)),
                                       static_cast<int32_t>(sizeof(MorphControlData)) *
                                           mpDrawCommandsBegin[i].MorphControlDataOffset);
            mpBackend->BindMorphTargets(mpDrawCommandsBegin[i].hGpuMorphTargets);
        }
        else
        {
            mpBackend->BindBuffer(resourceContext.hDummyMorphControlDataBuffer, eBufferBindingIndex::MorphControlData);
            mpBackend->BindMorphTargets(GpuMorphTargetsHandle::sNULL);
        }

        mpBackend->ExecuteDrawCommand(mpDrawCommandsBegin[i]);
    }
}

void OpaqueRenderStage::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderStage::FinishExecute(resourceContext);
}

void OpaqueRenderStage::TearDown()
{
    IRenderStage::TearDown();
}

void OpaqueRenderStage::SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                                         int32_t instanceCount,
                                         const Vector3& worldCameraPos)
{
    std::sort(pInstanceDataBufferBegin,
              pInstanceDataBufferBegin + instanceCount,
              [&](const InstanceData& a, const InstanceData& b)
              {
                  const float aDist = (worldCameraPos - Vector3(a.WorldMat.GetCol3())).SqrdMagnitude();
                  const float bDist = (worldCameraPos - Vector3(b.WorldMat.GetCol3())).SqrdMagnitude();
                  return aDist < bDist;
              });
}

void OpaqueRenderStage::sortDrawCommands()
{
    if (!mpDrawCommandsBegin)
    {
        return;
    }

    std::sort(mpDrawCommandsBegin,
              mpDrawCommandsBegin + mDrawCommandCount,
              [](const DrawCommand& a, const DrawCommand& b)
              {
                  if (a.hGpuMaterial != b.hGpuMaterial)
                  {
                      return a.hGpuMaterial.GetID() < b.hGpuMaterial.GetID();
                  }
                  if (a.StencilWriteBitIndex != b.StencilWriteBitIndex)
                  {
                      return a.StencilWriteBitIndex < b.StencilWriteBitIndex;
                  }
                  return a.SortDepth < b.SortDepth;
              });
}

} // namespace ho