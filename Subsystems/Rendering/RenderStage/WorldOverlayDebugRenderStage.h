#pragma once

#include "IRenderStage.h"
#include "WorldOverlayDebugRenderStageDesc.h"

namespace ho
{
// TODO: WorldOverlayDebugRenderStage just ignore draw command currently.
class WorldOverlayDebugRenderStage final : public IRenderStage
{
  public:
    WorldOverlayDebugRenderStage() = default;

    WorldOverlayDebugRenderStage(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets)
      : IRenderStage(pBackend, pBuiltInAssets)
    {
        mType = eRenderStageType::WorldOverlayDebug;
    }

    WorldOverlayDebugRenderStage(const WorldOverlayDebugRenderStage& other)
      : IRenderStage(other)
    {
    }

    WorldOverlayDebugRenderStage& operator=(const WorldOverlayDebugRenderStage& rhs)
    {
        if (this != &rhs)
        {
            IRenderStage::operator=(rhs);
        }
        return *this;
    }

    ~WorldOverlayDebugRenderStage() override = default;

    void Setup(const WorldOverlayDebugRenderStageDesc& desc);
    void PrepareExecute(const RenderResourceContext& resourceContext) override;
    void Execute(const RenderResourceContext& resourceContext) override;
    void FinishExecute(const RenderResourceContext& resourceContext) override;
    void TearDown() override;

    void SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                          int32_t instanceCount,
                          const Vector3& worldCameraPos) override;

  private:
    void sortDrawCommands() override;

    WorldOverlayDebugRenderStageDesc mStageDesc;
};
} // namespace ho