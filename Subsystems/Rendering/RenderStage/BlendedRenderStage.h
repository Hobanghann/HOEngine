#pragma once

#include "BlendedRenderStageDesc.h"
#include "IRenderStage.h"

namespace ho
{

class BlendedRenderStage final : public IRenderStage
{
  public:
    BlendedRenderStage() = default;

    BlendedRenderStage(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderStage(pBackend, pBuiltInAssets)
    {
        mType = eRenderStageType::Blended;
    }

    BlendedRenderStage(const BlendedRenderStage& other)
      : IRenderStage(other)
    {
    }

    BlendedRenderStage& operator=(const BlendedRenderStage& rhs)
    {
        if (this != &rhs)
        {
            IRenderStage::operator=(rhs);
        }
        return *this;
    }

    ~BlendedRenderStage() override = default;

    void Setup(const BlendedRenderStageDesc& desc);
    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

    void SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                          int32_t instanceCount,
                          const Vector3& worldCameraPos) override;

  private:
    void sortDrawCommands() override;

    BlendedRenderStageDesc mStageDesc;
};
} // namespace ho