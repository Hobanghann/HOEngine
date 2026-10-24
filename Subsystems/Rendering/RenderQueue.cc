#include "RenderQueue.h"

#include <imgui.h>

#include "RenderBackend/GpuBufferLayouts.h"
#include "RenderBackend/IRenderingBackend.h"
#include "RenderBackend/RenderResourceContext.h"
#include "RenderData/DrawCommand.h"
#include "RenderPass/IRenderPass.h"

namespace ho
{
RenderQueue::RenderQueue(IRenderingBackend* pBackend,
                         int32_t initialDrawCommandCount,
                         int32_t initialInstanceDataCount,
                         int32_t initialMorphControlDataCount)
  : mInstanceDataBuffer(FixedArray<InstanceData>(initialInstanceDataCount))
  , mMorphControlDataBuffer(FixedArray<MorphControlData>(initialMorphControlDataCount))
  , mpBackend(pBackend)
{
    HO_ASSERT(mpBackend, "Backend is null.");
    for (auto& drawCommands : mDrawCommands)
    {
        drawCommands = FixedArray<DrawCommand>(initialDrawCommandCount);
    }
}

RenderQueue::RenderQueue(const RenderQueue& other)
  : mbSetup(false)
  , mbPrepared(false)
  , mRenderPassCount(0)
  , mpRenderPasses(FixedArray<IRenderPass*>(other.mpRenderPasses.GetSize()))
  , mInstanceDataCount(0)
  , mInstanceDataBuffer(FixedArray<InstanceData>(other.mInstanceDataBuffer.GetSize()))
  , mMorphControlDataCount(0)
  , mMorphControlDataBuffer(FixedArray<MorphControlData>(other.mMorphControlDataBuffer.GetSize()))
  , mUIViewportCount(0)
  , mUIViewports(FixedArray<UIViewportData>(other.mUIViewports.GetSize()))
  , mpBackend(other.mpBackend)
{
    HO_ASSERT(other.mpBackend, "Source render queue has invalid backend.");
    HO_ASSERT(!other.mbSetup && !other.mbPrepared, "Source render queue must not be setup or prepared for copying.");
    HO_ASSERT(other.mRenderPassCount == 0, "Source render queue must have no render passes for copying.");
    HO_ASSERT(other.mInstanceDataCount == 0, "Source render queue must have no instance data for copying.");
    HO_ASSERT(other.mMorphControlDataCount == 0, "Source render queue must have no morph control data for copying.");
    HO_ASSERT(other.mUIViewportCount == 0, "Source render queue must have no UI viewports for copying.");
    for (int32_t i = 0; i < static_cast<int32_t>(eRenderStageType::Last); ++i)
    {
        mDrawCommands[i] = FixedArray<DrawCommand>(other.mDrawCommands[i].GetSize());
    }
}

RenderQueue& RenderQueue::operator=(const RenderQueue& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }

    HO_ASSERT(rhs.mpBackend, "RHS render queue has invalid backend.");
    HO_ASSERT(!rhs.mbSetup && !rhs.mbPrepared, "RHS render queue must not be setup or prepared for copying.");
    HO_ASSERT(rhs.mRenderPassCount == 0, "RHS render queue must have no render passes for copying.");
    HO_ASSERT(rhs.mInstanceDataCount == 0, "RHS render queue must have no instance data for copying.");
    HO_ASSERT(rhs.mMorphControlDataCount == 0, "RHS render queue must have no morph control data for copying.");
    HO_ASSERT(rhs.mUIViewportCount == 0, "RHS render queue must have no UI viewports for copying.");

    HO_ASSERT(!mbSetup && !mbPrepared, "LHS render queue must not be setup or prepared for copying.");
    HO_ASSERT(mRenderPassCount == 0, "LHS render queue must have no render passes for copying.");
    HO_ASSERT(mInstanceDataCount == 0, "LHS render queue must have no instance data for copying.");
    HO_ASSERT(mMorphControlDataCount == 0, "LHS render queue must have no morph control data for copying.");
    HO_ASSERT(mUIViewportCount == 0, "LHS render queue must have no UI viewports for copying.");

    mbSetup = false;
    mbPrepared = false;
    mRenderPassCount = 0;
    mpRenderPasses = FixedArray<IRenderPass*>(rhs.mpRenderPasses.GetSize());
    for (int32_t i = 0; i < static_cast<int32_t>(eRenderStageType::Last); ++i)
    {
        mDrawCommands[i] = FixedArray<DrawCommand>(rhs.mDrawCommands[i].GetSize());
    }
    mInstanceDataCount = 0;
    mInstanceDataBuffer = FixedArray<InstanceData>(rhs.mInstanceDataBuffer.GetSize());
    mMorphControlDataCount = 0;
    mMorphControlDataBuffer = FixedArray<MorphControlData>(rhs.mMorphControlDataBuffer.GetSize());
    mUIViewportCount = 0;
    mUIViewports = FixedArray<UIViewportData>(rhs.mUIViewports.GetSize());
    mpBackend = rhs.mpBackend;

    return *this;
}

void RenderQueue::Setup()
{
    HO_ASSERT(!mbSetup, "Render queue has already been set up.");
    HO_ASSERT(!mbPrepared, "Cannot set up a render queue while it is in a prepared state.");

    mbSetup = true;
}

void RenderQueue::PrepareExecute(const RenderResourceContext& resourceContext)
{
    HO_ASSERT(mbSetup, "Render queue must be set up before preparing execution.");
    HO_ASSERT(!mbRecordBegun, "Recording hasn't end. Recording must be ended before preparing execution.");
    HO_ASSERT(!mbPrepared, "Render queue is already prepared for execution.");

    mpBackend->UploadBufferData(resourceContext.hInstanceDataBuffer,
                                mInstanceDataBuffer.Data(),
                                static_cast<int32_t>(sizeof(InstanceData)) * mInstanceDataCount);
    mpBackend->UploadBufferData(resourceContext.hMorphControlDataBuffer,
                                mMorphControlDataBuffer.Data(),
                                static_cast<int32_t>(sizeof(MorphControlData)) * mMorphControlDataCount);

    mpBackend->BindBuffer(resourceContext.hInstanceDataBuffer, eBufferBindingIndex::InstanceData);
    mpBackend->BindBuffer(resourceContext.hMorphControlDataBuffer, eBufferBindingIndex::MorphControlData);
    mbPrepared = true;
}

void RenderQueue::Execute(const RenderResourceContext& resourceContext)
{
    HO_ASSERT(mbSetup && mbPrepared, "Render queue must be setup and prepared before execute.");

    // execute all render passes
    for (int32_t i = 0; i < mRenderPassCount; ++i)
    {
        if (mpRenderPasses[i])
        {
            mpRenderPasses[i]->PrepareExecute(resourceContext);
            mpRenderPasses[i]->Execute(resourceContext);
            mpRenderPasses[i]->FinishExecute(resourceContext);
        }
    }

    // render all UI viewports
    for (int32_t i = 0; i < mUIViewportCount; ++i)
    {
        mpBackend->RenderUI(mUIViewports[i]);
    }
}

void RenderQueue::FinishExecute(const RenderResourceContext& resourceContext)
{
    (void)resourceContext;
    HO_ASSERT(mbPrepared, "Cannot finish execution on a pass that is not prepared.");

    mbPrepared = false;
}

void RenderQueue::ResizeBuffers(RenderResourceContext& resourceContext)
{
    HO_ASSERT(!mbPrepared, "Must be called after 'FinishExecute' to ensure all resource usage has finished.");

    if (mbNeedToResizeRenderPassBuffer)
    {
        FixedArray<IRenderPass*> pNewRenderPasses = FixedArray<IRenderPass*>(mpRenderPasses.GetSize() * 2);
        for (int32_t i = 0; i < mRenderPassCount; ++i)
        {
            pNewRenderPasses[i] = mpRenderPasses[i];
        }

        mpRenderPasses = std::move(pNewRenderPasses);
        mbNeedToResizeRenderPassBuffer = false;
    }

    for (int32_t i = 0; i < static_cast<int32_t>(eRenderStageType::Last); ++i)
    {
        if (mbNeedToResizeDrawCommandsBuffer[i])
        {
            mDrawCommands[i] = FixedArray<DrawCommand>(mDrawCommands[i].GetSize() * 2);
            mbNeedToResizeDrawCommandsBuffer[i] = false;
        }
    }

    if (mbNeedToResizeInstanceDataBuffer)
    {
        mInstanceDataBuffer = FixedArray<InstanceData>(mInstanceDataBuffer.GetSize() * 2);

        mpBackend->UnloadBuffer(resourceContext.hInstanceDataBuffer);
        resourceContext.hInstanceDataBuffer =
            mpBackend->PrepareCreateBuffer(static_cast<int32_t>(sizeof(InstanceData)) * mInstanceDataBuffer.GetSize(),
                                           eGpuBufferUsage::Storage,
                                           eGpuBufferAccess::Dynamic,
                                           true);
        mpBackend->CreateBuffer(resourceContext.hInstanceDataBuffer);
        HO_ASSERT(resourceContext.hInstanceDataBuffer.IsValid(), "Failed to resize GPU instance data buffer.");

        mbNeedToResizeInstanceDataBuffer = false;
    }

    if (mbNeedToResizeMorphControlDataBuffer)
    {
        mMorphControlDataBuffer = FixedArray<MorphControlData>(mMorphControlDataBuffer.GetSize() * 2);

        mpBackend->UnloadBuffer(resourceContext.hMorphControlDataBuffer);
        resourceContext.hMorphControlDataBuffer = mpBackend->PrepareCreateBuffer(
            static_cast<int32_t>(sizeof(MorphControlData)) * mMorphControlDataBuffer.GetSize(),
            eGpuBufferUsage::Storage,
            eGpuBufferAccess::Dynamic,
            true);
        mpBackend->CreateBuffer(resourceContext.hMorphControlDataBuffer);
        HO_ASSERT(resourceContext.hMorphControlDataBuffer.IsValid(), "Failed to resize GPU morph control data buffer.");

        mbNeedToResizeMorphControlDataBuffer = false;
    }

    if (mbNeedToResizeUIViewportBuffer)
    {
        FixedArray<UIViewportData> newUIViewports = FixedArray<UIViewportData>(mUIViewports.GetSize() * 2);
        for (int32_t i = 0; i < mRenderPassCount; ++i)
        {
            newUIViewports[i] = std::move(mUIViewports[i]);
        }

        mUIViewports = std::move(newUIViewports);
        mbNeedToResizeRenderPassBuffer = false;
    }
}

void RenderQueue::TearDown()
{
    HO_ASSERT(mbSetup, "Cannot tear down a pass that has not been set up.");
    HO_ASSERT(!mbPrepared, "Cannot tear down a pass while execution is still prepared.");

    mbSetup = false;

    for (int32_t i = 0; i < mRenderPassCount; ++i)
    {
        mpRenderPasses[i]->TearDown();
        mpRenderPasses[i]->SetBound(false);
    }

    mRenderPassCount = 0;
    for (int& drawCommandCount : mDrawCommandCount)
    {
        drawCommandCount = 0;
    }
    mInstanceDataCount = 0;
    mMorphControlDataCount = 0;
    mUIViewportCount = 0;
}

bool RenderQueue::IsSetup() const
{
    return mbSetup;
}

bool RenderQueue::IsPrepared() const
{
    return mbPrepared;
}

bool RenderQueue::PushRenderPass(IRenderPass* pRenderPass)
{
    if (!pRenderPass)
    {
        HO_ASSERT(false, "RenderPass is null");
        return false;
    }
    if (pRenderPass->IsBound())
    {
        HO_ASSERT(false, "Render pass is already bound.");
        return false;
    }

    if (mRenderPassCount == mpRenderPasses.GetSize())
    {
        mbNeedToResizeRenderPassBuffer = true;
        return false;
    }

    mpRenderPasses[mRenderPassCount] = pRenderPass;
    ++mRenderPassCount;
    pRenderPass->SetBound(true);
    return true;
}

int32_t RenderQueue::GetDrawCommandCount(eRenderStageType stageType) const
{
    return mDrawCommandCount[static_cast<int32_t>(stageType)];
}

DrawCommand& RenderQueue::GetDrawCommand(eRenderStageType stageType, int32_t idx)
{
    HO_ASSERT(idx > 0 && idx < mDrawCommandCount[static_cast<int32_t>(stageType)], "Out of bounds.");
    return mDrawCommands[static_cast<int32_t>(stageType)][idx];
}

void RenderQueue::BeginRecordDrawCommand()
{
    HO_ASSERT(mRenderPassCount > 0, "There is no pushed pass.");
    HO_ASSERT(!mbRecordBegun, "Recording has begun already.");
    mbRecordBegun = true;
    mpRecordingPass = mpRenderPasses[mRenderPassCount - 1];
    for (int32_t i = 0; i < static_cast<int32_t>(eRenderStageType::Last); ++i)
    {
        mDrawCommandRecordOffset[i] = mDrawCommandCount[i];
    }
}

bool RenderQueue::PushDrawCommand(eRenderStageType stageType, const DrawCommand& cmd)
{
    HO_ASSERT(mbRecordBegun, "Recording hasn't begun.");
    HO_ASSERT(mpRecordingPass->IsStageAttached(stageType), "Stage not attached on pushed pass.");

    const int32_t stageIdx = static_cast<int32_t>(stageType);
    if (mDrawCommandCount[stageIdx] == mDrawCommands[stageIdx].GetSize())
    {
        mbNeedToResizeDrawCommandsBuffer[stageIdx] = true;
        return false;
    }
    mDrawCommands[stageIdx][mDrawCommandCount[stageIdx]] = cmd;
    ++mDrawCommandCount[stageIdx];
    return true;
}

void RenderQueue::EndRecordDrawCommand()
{
    HO_ASSERT(mbRecordBegun, "Recording hasn't begun.");

    for (int32_t i = 0; i < static_cast<int32_t>(eRenderStageType::Last); ++i)
    {
        const eRenderStageType stageType = static_cast<eRenderStageType>(i);
        if (mpRecordingPass->IsStageAttached(stageType))
        {
            mpRecordingPass->GetStage(stageType)->SetDrawCommands(mDrawCommands[i].Data() + mDrawCommandRecordOffset[i],
                                                                  mDrawCommandCount[i] - mDrawCommandRecordOffset[i]);
        }
        mDrawCommandRecordOffset[i] = 0;
    }
    mbRecordBegun = false;
    mpRecordingPass = nullptr;
}

bool RenderQueue::IsInstanceDataBufferFull() const
{
    return mInstanceDataCount == mInstanceDataBuffer.GetSize();
}

int32_t RenderQueue::GetInstanceDataCount() const
{
    return mInstanceDataCount;
}

InstanceData* RenderQueue::AllocateInstanceDataBuffer(int32_t dataCount)
{
    HO_ASSERT(dataCount > 0, "Data count must be greater than zero.");
    if (mInstanceDataCount + dataCount > mInstanceDataBuffer.GetSize())
    {
        mbNeedToResizeInstanceDataBuffer = true;
        return nullptr;
    }
    InstanceData* pAllocBegin = mInstanceDataBuffer.Data() + mInstanceDataCount; // NOLINT(misc-const-correctness)
    mInstanceDataCount += dataCount;
    return pAllocBegin;
}

bool RenderQueue::IsMorphControlDataBufferFull() const
{
    return mMorphControlDataCount == mMorphControlDataBuffer.GetSize();
}

int32_t RenderQueue::GetMorphControlDataCount() const
{
    return mMorphControlDataCount;
}

MorphControlData* RenderQueue::AllocateMorphControlDataBuffer(int32_t dataCount)
{
    HO_ASSERT(dataCount > 0, "Data count must be greater than zero.");
    if (mMorphControlDataCount + dataCount > mMorphControlDataBuffer.GetSize())
    {
        mbNeedToResizeMorphControlDataBuffer = true;
        return nullptr;
    }
    MorphControlData* pAllocBegin = // NOLINT(misc-const-correctness)
        mMorphControlDataBuffer.Data() + mMorphControlDataCount;
    mMorphControlDataCount += dataCount;
    return pAllocBegin;
}

bool RenderQueue::PushUIViewportData(const IPlatformWindow* pTargetWindow, const ImDrawData& srcDrawData)
{
    if (mUIViewportCount == mUIViewports.GetSize())
    {
        mbNeedToResizeUIViewportBuffer = true;
        return false;
    }

    if (!srcDrawData.Valid)
    {
        return false;
    }

    UIViewportData& dstData = mUIViewports[mUIViewportCount];

    dstData.pTargetWindow = pTargetWindow;

    dstData.pDrawData->Valid = srcDrawData.Valid;
    dstData.pDrawData->CmdListsCount = srcDrawData.CmdListsCount;
    dstData.pDrawData->TotalIdxCount = srcDrawData.TotalIdxCount;
    dstData.pDrawData->TotalVtxCount = srcDrawData.TotalVtxCount;

    // deep copy ImDrawData::CmdLists
    const int32_t oldSize = dstData.pDrawData->CmdLists.Size;
    const int32_t newSize = srcDrawData.CmdLists.Size;
    if (oldSize > newSize)
    {
        for (int32_t i = newSize; i < oldSize; ++i)
        {
            IM_DELETE(dstData.pDrawData->CmdLists[i]);
            dstData.pDrawData->CmdLists[i] = nullptr;
        }
        dstData.pDrawData->CmdLists.resize(newSize);
    }
    else if (oldSize < newSize)
    {
        dstData.pDrawData->CmdLists.resize(srcDrawData.CmdLists.Size);
        for (int32_t i = oldSize; i < newSize; ++i)
        {
            dstData.pDrawData->CmdLists[i] = nullptr;
        }
    }

    for (int32_t i = 0; i < srcDrawData.CmdLists.Size; ++i)
    {
        if (dstData.pDrawData->CmdLists[i] == nullptr)
        {
            dstData.pDrawData->CmdLists[i] = IM_NEW(ImDrawList)(nullptr);
        }

        ImDrawList* pDstList = dstData.pDrawData->CmdLists[i];
        const ImDrawList* pSrcList = srcDrawData.CmdLists[i];

        pDstList->Flags = pSrcList->Flags;

        pDstList->CmdBuffer = pSrcList->CmdBuffer;
        pDstList->IdxBuffer = pSrcList->IdxBuffer;
        pDstList->VtxBuffer = pSrcList->VtxBuffer;
    }

    dstData.pDrawData->DisplayPos = srcDrawData.DisplayPos;
    dstData.pDrawData->DisplaySize = srcDrawData.DisplaySize;
    dstData.pDrawData->FramebufferScale = srcDrawData.FramebufferScale;
    dstData.pDrawData->OwnerViewport = srcDrawData.OwnerViewport;
    dstData.pDrawData->Textures = srcDrawData.Textures;

    ++mUIViewportCount;
    return true;
}
} // namespace ho