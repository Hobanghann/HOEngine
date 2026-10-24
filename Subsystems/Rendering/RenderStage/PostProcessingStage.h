#pragma once

#include "../RenderData/GpuResources.h"
#include "IRenderStage.h"
#include "PostProcessingStageDesc.h"

namespace ho
{
class PostProcessingStage final : public IRenderStage
{
  public:
    PostProcessingStage() = default;

    PostProcessingStage(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderStage(pBackend, pBuiltInAssets)
    {
        mType = eRenderStageType::PostProcessing;
    }

    PostProcessingStage(const PostProcessingStage& other)
      : IRenderStage(other)
    {
    }

    PostProcessingStage& operator=(const PostProcessingStage& rhs)
    {
        if (this != &rhs)
        {
            IRenderStage::operator=(rhs);
        }
        return *this;
    }

    ~PostProcessingStage() override = default;

    void Setup(const PostProcessingStageDesc& desc);
    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

    void OnAttached(const IRenderPass& owner) override;

    void SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                          int32_t instanceCount,
                          const Vector3& worldCameraPos) override;

  private:
    void sortDrawCommands() override;

    PostProcessingStageDesc mStageDesc;
    FrameBufferHandle mhProcessingFB;
};
} // namespace ho