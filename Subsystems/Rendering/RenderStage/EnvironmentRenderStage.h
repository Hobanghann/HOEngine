#pragma once

#include "EnvironmentRenderStageDesc.h"
#include "IRenderStage.h"

namespace ho
{
// TODO: EnvironmentRenderStage just ignore draw command currently.
class EnvironmentRenderStage final : public IRenderStage
{
  public:
    EnvironmentRenderStage() = default;

    EnvironmentRenderStage(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderStage(pBackend, pBuiltInAssets)
    {
        mType = eRenderStageType::Environment;
    }

    EnvironmentRenderStage(const EnvironmentRenderStage& other)
      : IRenderStage(other)
    {
    }

    EnvironmentRenderStage& operator=(const EnvironmentRenderStage& rhs)
    {
        if (this != &rhs)
        {
            IRenderStage::operator=(rhs);
        }
        return *this;
    }

    ~EnvironmentRenderStage() override = default;

    void Setup(const EnvironmentRenderStageDesc& desc);
    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

    void SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                          int32_t instanceCount,
                          const Vector3& worldCameraPos) override;

  private:
    void sortDrawCommands() override;

    EnvironmentRenderStageDesc mStageDesc;
};
} // namespace ho