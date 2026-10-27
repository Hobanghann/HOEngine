#include "WorldOverlayDebugRenderStage.h"

#include "../RenderBackend/GpuBufferLayouts.h"
#include "../RenderBackend/IRenderingBackend.h"
#include "../RenderBackend/RenderResourceContext.h"
#include "../RenderData/DrawCommand.h"
#include "WorldOverlayDebugRenderStageDesc.h"

namespace ho
{
void WorldOverlayDebugRenderStage::Setup(const WorldOverlayDebugRenderStageDesc& desc)
{
    IRenderStage::Setup();

    mStageDesc = desc;
}

void WorldOverlayDebugRenderStage::PrepareExecute(const RenderResourceContext& resourceContext)
{
    if (!mbSetup)
    {
        return;
    }

    IRenderStage::PrepareExecute(resourceContext);
}

void WorldOverlayDebugRenderStage::Execute(const RenderResourceContext& resourceContext)
{
    HO_ASSERT(isExecutable(), "Stage is not executable");
}

void WorldOverlayDebugRenderStage::FinishExecute(const RenderResourceContext& resourceContext)
{
    IRenderStage::FinishExecute(resourceContext);
}

void WorldOverlayDebugRenderStage::TearDown()
{
    IRenderStage::TearDown();
}

void WorldOverlayDebugRenderStage::SortInstanceData(InstanceData* pInstanceDataBufferBegin,
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

void WorldOverlayDebugRenderStage::sortDrawCommands()
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