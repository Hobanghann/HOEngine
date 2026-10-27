#pragma once

#include "../RenderData/DrawCommand.h"
#include "Core/Templates/FixedArray.h"

namespace ho
{
class IRenderingBackend;
struct RenderingBuiltInAssets;
struct InstanceData;
struct RenderResourceContext;
struct Vector3;

class IRenderPass;

// NOTE: Stages will execute following order.
enum class eRenderStageType
{
    Opaque,
    Masked,
    Environment,
    InWorldDebug,
    Blended,
    Outline,
    PostProcessing,
    WorldOverlayDebug,
    Last,
};

class IRenderStage
{
  public:
    IRenderStage();

    IRenderStage(IRenderingBackend* pBackend, RenderingBuiltInAssets* pBuiltInAssets);

    IRenderStage(const IRenderStage& other);

    IRenderStage& operator=(const IRenderStage& rhs);

    virtual ~IRenderStage();

    eRenderStageType GetType() const;

    // Setup is called by the main thread and constructs the stage.
    void Setup();

    // PrepareExecute through FinishExecute are called by the rendering thread.
    // An asset judged by the main thread to be uploaded to the GPU during Setup
    // may be evicted by the rendering backend at the time the render queue is swapped.
    // Therefore, functions from PrepareExecute to FinishExecute must be implemented
    // considering that recorded GpuResourceHandles may be invalid.
    virtual void PrepareExecute(const RenderResourceContext& resourceContext);
    virtual void Execute(const RenderResourceContext& resourceContext) = 0;
    virtual void FinishExecute(const RenderResourceContext& resourceContext);

    // TearDown is called by the main thread and tear down the stage.
    virtual void TearDown();

    virtual void OnAttached(const IRenderPass& owner);

    bool IsAttached() const;

    bool IsSetup() const;

    bool IsPrepared() const;

    virtual void SortInstanceData(InstanceData* pInstanceDataBufferBegin,
                                  int32_t instanceCount,
                                  const Vector3& worldCameraPos) = 0;

    void SetDrawCommands(DrawCommand* pDrawCommandsBegin, int32_t drawCommandCount);

  protected:
    bool isExecutable() const;

    // mpDrawCommandsBegin can be null.
    virtual void sortDrawCommands() = 0;

    eRenderStageType mType = eRenderStageType::Last;

    bool mbIsAttached = false;
    bool mbSetup = false;
    bool mbPrepared = false;

    int32_t mDrawCommandCount = 0;
    DrawCommand* mpDrawCommandsBegin = nullptr;

    RenderingBuiltInAssets* mpBuiltInAssets = nullptr;
    IRenderingBackend* mpBackend = nullptr;
};
} // namespace ho