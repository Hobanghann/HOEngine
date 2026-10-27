#pragma once

#include "../RenderData/GpuResources.h"
#include "IRenderStage.h"
#include "OutlineRenderStageDesc.h"

namespace ho
{
class OutlineRenderStage final : public IRenderStage
{
  public:
    OutlineRenderStage() = default;

    OutlineRenderStage(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderStage(pBackend, pBuiltInAssets)
    {
        mType = eRenderStageType::Outline;
    }

    OutlineRenderStage(const OutlineRenderStage& other)
      : IRenderStage(other)
    {
    }

    OutlineRenderStage& operator=(const OutlineRenderStage& rhs)
    {
        if (this != &rhs)
        {
            IRenderStage::operator=(rhs);
        }
        return *this;
    }

    ~OutlineRenderStage() override = default;

    void Setup(const OutlineRenderStageDesc& desc);
    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

    void SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                          int32_t instanceCount,
                          const Vector3& worldCameraPos) override;

  private:
    void sortDrawCommands() override;

    OutlineRenderStageDesc mStageDesc;
    FrameBufferHandle mhProcessingFB;
};
} // namespace ho