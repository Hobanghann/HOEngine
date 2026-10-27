#include "OutlineRenderStage.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderData/DrawCommand.h"
#include "../RenderingBuiltInAssets.h"

namespace ho
{
void OutlineRenderStage::Setup(const OutlineRenderStageDesc& desc)
{
    IRenderStage::Setup();

    mStageDesc = desc;
}

void OutlineRenderStage::PrepareExecute(const RenderResourceContext& resourceContext)
{
    if (!mbSetup)
    {
        return;
    }

    IRenderStage::PrepareExecute(resourceContext);

    mpBackend->SetCullMode(IRenderingBackend::eCullMode::Back);

    mpBackend->SetDepthWrite(false);
    mpBackend->SetDepthTest(false);

    mpBackend->SetAlphaBlendMode(IRenderingBackend::eAlphaBlendMode::None);

    mpBackend->SetStencilTest(true);
    mpBackend->SetStencilState(IRenderingBackend::eComparisonFunc::NotEqual, 0x01);
    mpBackend->SetStencilReference(0x01);
    mpBackend->SetStencilOperator(IRenderingBackend::eStencilOperator::Keep,
                                  IRenderingBackend::eStencilOperator::Keep,
                                  IRenderingBackend::eStencilOperator::Keep);

    mpBackend->BindShader(mpBuiltInAssets->hOutlineVS, eShaderStage::VertexShader);
    mpBackend->BindShader(mpBuiltInAssets->hSingleColorFS, eShaderStage::FragmentShader);
}

void OutlineRenderStage::Execute(const RenderResourceContext& resourceContext)
{
    HO_ASSERT(isExecutable(), "Stage is not executable");

    for (int32_t i = 0; i < mDrawCommandCount; ++i)
    {
        if (mpDrawCommandsBegin[i].StencilWriteBitIndex != -1)
        {
            HO_ASSERT(false, "Draw commands in this stage must not perform stencil writes.");
            continue;
        }

        // Bind static mesh
        GpuStaticMeshHandle hStaticMesh = mpDrawCommandsBegin[i].hGpuStaticMesh;

        if (hStaticMesh.IsNULL())
        {
            hStaticMesh = mpBuiltInAssets->hOctahedronStaticMesh;
        }
        else if (hStaticMesh->State == eGpuResourceState::PendingUpload)
        {
            hStaticMesh = mpBuiltInAssets->hIcosaedroStaticMesh;
        }

        mpBackend->BindStaticMesh(hStaticMesh);

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

void OutlineRenderStage::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderStage::FinishExecute(resourceContext);
}

void OutlineRenderStage::TearDown()
{
    IRenderStage::TearDown();
}

void OutlineRenderStage::SortInstanceData(InstanceData* pInstanceDataBufferBegin,
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

void OutlineRenderStage::sortDrawCommands()
{
    if (!mpDrawCommandsBegin)
    {
        return;
    }

    std::sort(mpDrawCommandsBegin,
              mpDrawCommandsBegin + mDrawCommandCount,
              [](const DrawCommand& a, const DrawCommand& b) { return a.SortDepth < b.SortDepth; });
}

} // namespace ho