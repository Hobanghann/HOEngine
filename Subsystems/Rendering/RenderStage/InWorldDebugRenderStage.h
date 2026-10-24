#pragma once

#include "IRenderStage.h"
#include "InWorldDebugRenderStageDesc.h"

namespace ho
{
class InWorldDebugRenderStage final : public IRenderStage
{
  public:
    InWorldDebugRenderStage() = default;

    InWorldDebugRenderStage(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderStage(pBackend, pBuiltInAssets)
    {
        mType = eRenderStageType::InWorldDebug;
    }

    InWorldDebugRenderStage(const InWorldDebugRenderStage& other)
      : IRenderStage(other)
    {
    }

    InWorldDebugRenderStage& operator=(const InWorldDebugRenderStage& rhs)
    {
        if (this != &rhs)
        {
            IRenderStage::operator=(rhs);
        }
        return *this;
    }

    ~InWorldDebugRenderStage() override = default;

    void Setup(const InWorldDebugRenderStageDesc& desc);
    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

    void SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                          int32_t instanceCount,
                          const Vector3& worldCameraPos) override;

  private:
    void sortDrawCommands() override;

    InWorldDebugRenderStageDesc mStageDesc;
};
} // namespace ho