#pragma once

#include "IRenderStage.h"
#include "OpaqueRenderStageDesc.h"

namespace ho
{
class OpaqueRenderStage final : public IRenderStage
{
  public:
    OpaqueRenderStage() = default;

    OpaqueRenderStage(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderStage(pBackend, pBuiltInAssets)
    {
        mType = eRenderStageType::Opaque;
    }

    OpaqueRenderStage(const OpaqueRenderStage& other)
      : IRenderStage(other)
    {
    }

    OpaqueRenderStage& operator=(const OpaqueRenderStage& rhs)
    {
        if (this != &rhs)
        {
            IRenderStage::operator=(rhs);
        }
        return *this;
    }

    ~OpaqueRenderStage() override = default;

    void Setup(const OpaqueRenderStageDesc& desc);
    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

    void SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                          int32_t instanceCount,
                          const Vector3& worldCameraPos) override;

  private:
    void sortDrawCommands() override;

    OpaqueRenderStageDesc mStageDesc;
};
} // namespace ho