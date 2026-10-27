#pragma once

#include "Core/Memory/WeakTypedHandle.h"
#include "Core/Templates/FixedArray.h"
#include "RenderData/UIViewportData.h"
#include "RenderStage/IRenderStage.h"

namespace ho
{
class IRenderPass;
class IRenderingBackend;
struct RenderResourceContext;
struct InstanceData;
struct MorphControlData;
struct DrawCommand;

struct RMorphTargetsAsset;
using MorphTargetsHandle = WeakTypedHandle<RMorphTargetsAsset, TypedObjectPool>;

class RenderQueue final
{
    static const int32_t sInitialRenderPassCount = 16;
    static const int32_t sInitialUIViewportCount = 16;

  public:
    RenderQueue() = default;
    RenderQueue(IRenderingBackend* pBackend,
                int32_t initialDrawCommandCount,
                int32_t initialInstanceDataCount,
                int32_t initialMorphControlDataCount);
    RenderQueue(const RenderQueue& other);
    RenderQueue& operator=(const RenderQueue& rhs);

    ~RenderQueue() = default;

    // Setup is called by the main thread and constructs the queue.
    void Setup();

    // PrepareExecute through FinishExecute are called by the rendering thread.
    // An asset judged by the main thread to be uploaded to the GPU during Setup
    // may be evicted by the rendering backend at the time the render queue is swapped.
    // Therefore, functions from PrepareExecute to FinishExecute must be implemented
    // considering that recorded GpuResourceHandles may be invalid.
    void PrepareExecute(const RenderResourceContext& resourceContext);
    void Execute(const RenderResourceContext& resourceContext);
    void FinishExecute(const RenderResourceContext& resourceContext);

    // ResizeBuffers is called by the render thread because it calls graphics API.
    void ResizeBuffers(RenderResourceContext& resourceContext);

    // TearDown is called by the main thread and tear down the queue, pass, stage.
    void TearDown();

    bool IsSetup() const;

    bool IsPrepared() const;

    bool PushRenderPass(IRenderPass* pRenderPass);

    int32_t GetDrawCommandCount(eRenderStageType stageType) const;

    DrawCommand& GetDrawCommand(eRenderStageType stageType, int32_t idx);

    // It's for recording to latest pushed render pass.
    void BeginRecordDrawCommand();

    bool PushDrawCommand(eRenderStageType stageType, const DrawCommand& cmd);

    void EndRecordDrawCommand();

    bool IsInstanceDataBufferFull() const;

    int32_t GetInstanceDataCount() const;

    InstanceData* AllocateInstanceDataBuffer(int32_t dataCount);

    bool IsMorphControlDataBufferFull() const;

    int32_t GetMorphControlDataCount() const;

    MorphControlData* AllocateMorphControlDataBuffer(int32_t dataCount);

    bool PushUIViewportData(const IPlatformWindow* pTargetWindow, const ImDrawData& srcDrawData);

  private:
    bool mbSetup = false;
    bool mbPrepared = false;

    int32_t mRenderPassCount = 0;
    FixedArray<IRenderPass*> mpRenderPasses = FixedArray<IRenderPass*>(sInitialRenderPassCount);
    bool mbNeedToResizeRenderPassBuffer = false;

    int32_t mDrawCommandCount[static_cast<int32_t>(eRenderStageType::Last)] = {};
    FixedArray<DrawCommand> mDrawCommands[static_cast<int32_t>(eRenderStageType::Last)] = {};
    bool mbNeedToResizeDrawCommandsBuffer[static_cast<int32_t>(eRenderStageType::Last)] = {};

    bool mbRecordBegun = false;
    IRenderPass* mpRecordingPass = nullptr;
    int32_t mDrawCommandRecordOffset[static_cast<int32_t>(eRenderStageType::Last)] = {};

    int32_t mInstanceDataCount = 0;
    FixedArray<InstanceData> mInstanceDataBuffer;
    bool mbNeedToResizeInstanceDataBuffer = false;

    int32_t mMorphControlDataCount = 0;
    FixedArray<MorphControlData> mMorphControlDataBuffer;
    bool mbNeedToResizeMorphControlDataBuffer = false;

    int32_t mUIViewportCount = 0;
    FixedArray<UIViewportData> mUIViewports = FixedArray<UIViewportData>(sInitialUIViewportCount);
    bool mbNeedToResizeUIViewportBuffer = false;

    IRenderingBackend* mpBackend = nullptr;
};

} // namespace ho