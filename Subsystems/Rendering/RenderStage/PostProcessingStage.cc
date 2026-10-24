#include "PostProcessingStage.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderBindingIndex.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderData/DrawCommand.h"
#include "../RenderPass/ForwardRenderPass.h"
#include "../RenderPass/IRenderPass.h"
#include "../RenderingBuiltInAssets.h"
#include "PostProcessingStageDesc.h"

namespace ho
{
void PostProcessingStage::Setup(const PostProcessingStageDesc& desc)
{
    IRenderStage::Setup();

    mStageDesc = desc;
}

void PostProcessingStage::PrepareExecute(const RenderResourceContext& resourceContext)
{
    if (!mbSetup)
    {
        return;
    }

    IRenderStage::PrepareExecute(resourceContext);

    mpBackend->SetPolygonMode(IRenderingBackend::ePolygonMode::Triangle);
    mpBackend->SetCullMode(IRenderingBackend::eCullMode::Back);

    mpBackend->SetDepthTest(false);
    mpBackend->SetDepthWrite(false);

    mpBackend->SetStencilTest(false);

    mpBackend->SetAlphaBlendMode(IRenderingBackend::eAlphaBlendMode::None);
}

void PostProcessingStage::Execute(const RenderResourceContext& resourceContext)
{
    HO_ASSERT(isExecutable(), "Stage is not executable");

    // Tone Mapping & Gamma Correction
    mpBackend->BindShader(mpBuiltInAssets->hToneMappingGammaCorrectionVS, eShaderStage::VertexShader);
    mpBackend->BindShader(mpBuiltInAssets->hToneMappingGammaCorrectionFS, eShaderStage::FragmentShader);

    const FrameBuffer::TargetBindingState accBindingState =
        mhProcessingFB->ColorTargetStates[static_cast<int32_t>(eRenderTargetBindingIndex::Accumulation)];

    mpBackend->ResourceBarrier(accBindingState.hTarget,
                               IRenderingBackend::eResourceState::RenderTarget,
                               IRenderingBackend::eResourceState::ShaderResource);

    mpBackend->DetachColorTarget(mhProcessingFB, eRenderTargetBindingIndex::Accumulation);
    mpBackend->BindTexture(accBindingState.hTarget, eTextureBindingIndex::Accumulation);

    mpBackend->DrawProcedural(6);

    mpBackend->AttachColorTargetRange(mhProcessingFB,
                                      eRenderTargetBindingIndex::Accumulation,
                                      accBindingState.hTarget,
                                      accBindingState.BaseSlice,
                                      accBindingState.SliceCount,
                                      accBindingState.MipLevel);
}

void PostProcessingStage::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderStage::FinishExecute(resourceContext);

    mStageDesc = PostProcessingStageDesc();
}

void PostProcessingStage::TearDown()
{
    IRenderStage::TearDown();

    mhProcessingFB = FrameBufferHandle::sNULL;
}

void PostProcessingStage::OnAttached(const IRenderPass& owner)
{
    IRenderStage::OnAttached(owner);

    switch (owner.GetType())
    {
        case eRenderPassType::ForwardRenderPass:
        {
            mhProcessingFB = static_cast<const ForwardRenderPass&>(owner).GetFrameBuffer();
        }
        break;
        case eRenderPassType::DeferedRenderPass:
        {
        }
        break;
        default:
            HO_ASSERT(false, "Post processing stage can be attached forward/defered render pass.");
    }
}

void PostProcessingStage::SortInstanceData(InstanceData* pInstanceDataBufferBegin,
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

void PostProcessingStage::sortDrawCommands()
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