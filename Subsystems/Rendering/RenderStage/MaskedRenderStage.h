#pragma once

#include "IRenderStage.h"
#include "MaskedRenderStageDesc.h"

namespace ho
{
class MaskedRenderStage final : public IRenderStage
{
  public:
    MaskedRenderStage() = default;

    MaskedRenderStage(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderStage(pBackend, pBuiltInAssets)
    {
        mType = eRenderStageType::Masked;
    }

    MaskedRenderStage(const MaskedRenderStage& other)
      : IRenderStage(other)
    {
    }

    MaskedRenderStage& operator=(const MaskedRenderStage& rhs)
    {
        if (this != &rhs)
        {
            IRenderStage::operator=(rhs);
        }
        return *this;
    }

    ~MaskedRenderStage() override = default;

    void Setup(const MaskedRenderStageDesc& desc);
    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

    void SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                          int32_t instanceCount,
                          const Vector3& worldCameraPos) override;

  private:
    void sortDrawCommands() override;

    MaskedRenderStageDesc mStageDesc;
};
} // namespace ho