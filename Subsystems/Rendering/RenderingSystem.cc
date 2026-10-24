#include "RenderingSystem.h"

#include "Core/IO/Path.h"
#include "Core/IO/ResourceParseFuncs.h"
#include "Platforms/IPlatformApplication.h"
#include "RenderBackend/GL/RenderingBackendGL.h"
#include "RenderBackend/GpuBufferLayouts.h"
#include "RenderBackend/RenderBindingIndex.h"
#include "RenderPass/RenderPasses.h"
#include "RenderStage/RenderStages.h"
#include "Subsystems/Asset/AssetImportFuncs.h"
#include "Subsystems/Asset/AssetSystem.h"
#include "Subsystems/Asset/Assets.h"

namespace ho
{
bool RenderingSystem::CreateFramebuffer(const FrameBufferDesc& frameBufferDesc)
{
    if (mFrameBufferToCreateQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return false;
    }

    mFrameBufferToCreateQueueLock.Lock();
    mFrameBufferToCreateQueue.Push(frameBufferDesc);
    mFrameBufferToCreateQueueLock.Unlock();
    return true;
}

bool RenderingSystem::DestroyFramebuffer(StringHandle hFrameBufferName)
{
    if (mhFrameBufferToDestroyQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return false;
    }

    mFrameBufferToDestroyQueueLock.Lock();
    mhFrameBufferToDestroyQueue.Push(hFrameBufferName);
    mFrameBufferToDestroyQueueLock.Unlock();
    return true;
}

void* RenderingSystem::GetRenderResultNativeHandle(StringHandle hFrameBufferName) const
{
    auto fbit = mNameToFrameBufferMap.find(hFrameBufferName);

    if (fbit == mNameToFrameBufferMap.end())
    {
        // HO_ASSERT(false, "There is no frame buffer has sush name.");
        return nullptr;
    }

    const FrameBufferHandle hFrontFB = fbit->second.hFrameBuffers[mFrontRenderQueueIndex];

    return mpBackend->GetColorTargetNativeHandle(hFrontFB, eRenderTargetBindingIndex::Final);
}

Vector2 RenderingSystem::GetFrameBufferSize(StringHandle hFrameBufferName) const
{
    auto fboItr = mNameToFrameBufferMap.find(hFrameBufferName);
    if (fboItr == mNameToFrameBufferMap.end())
    {
        HO_ASSERT(false, "There is no frame buffer has sush name.");
        return Vector2::sZero;
    }

    return Vector2(static_cast<float>(fboItr->second.hFrameBuffers[mFrontRenderQueueIndex]->Width),
                   static_cast<float>(fboItr->second.hFrameBuffers[mFrontRenderQueueIndex]->Height));
}

bool RenderingSystem::HasFrameBuffer(StringHandle hFrameBufferName) const
{
    auto fboItr = mNameToFrameBufferMap.find(hFrameBufferName);
    return fboItr != mNameToFrameBufferMap.end();
}

bool RenderingSystem::BeginFrame()
{
    return true;
}

bool RenderingSystem::BeginEnvironmentBakePass(const EnvironmentBakePassDesc& desc)
{
    if (!mRenderQueues[mFrontRenderQueueIndex].IsSetup())
    {
        HO_ASSERT(false, "Render queue is not setup yet. You must call 'BeginFrame' before beginning a render pass.");
        return false;
    }
    if (mpRecordingPass)
    {
        HO_ASSERT(false, "Render pass was already begun. You must call 'EndRenderPass' before beginning a new one.");
        return false;
    }

    EnvironmentBakePass* pRecordingPass = nullptr;
    for (auto& pass : mEnvironmentBakePassPool)
    {
        if (!pass.IsBound())
        {
            pRecordingPass = &pass;
            break;
        }
    }

    if (!pRecordingPass)
    {
        HO_ASSERT(false, "Every pass are bound already. Check mRenderPassPoolSize.");
        return false;
    }

    pRecordingPass->Setup(desc);

    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    if (!queue.PushRenderPass(pRecordingPass))
    {
        return false;
    }

    mpRecordingPass = pRecordingPass;

    return true;
}

bool RenderingSystem::BeginShadowMapBakePass(const ShadowMapBakePassDesc& desc)
{
    if (!mRenderQueues[mFrontRenderQueueIndex].IsSetup())
    {
        HO_ASSERT(false, "Render queue is not setup yet. You must call 'BeginFrame' before beginning a render pass.");
        return false;
    }
    if (mpRecordingPass)
    {
        HO_ASSERT(false, "Render pass was already begun. You must call 'EndRenderPass' before beginning a new one.");
        return false;
    }

    ShadowMapBakePass* pRecordingPass = nullptr;
    for (auto& pass : mShadowMapBakePassPool)
    {
        if (!pass.IsBound())
        {
            pRecordingPass = &pass;
            break;
        }
    }

    if (!pRecordingPass)
    {
        HO_ASSERT(false, "Every pass are bound already. Check mRenderPassPoolSize.");
        return false;
    }

    pRecordingPass->Setup(desc);

    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    if (!queue.PushRenderPass(pRecordingPass))
    {
        return false;
    }

    mpRecordingPass = pRecordingPass;

    return true;
}

bool RenderingSystem::BeginDirLightShadowMapRenderPass(const DirLightShadowMapRenderPassDesc& desc)
{
    if (!mRenderQueues[mFrontRenderQueueIndex].IsSetup())
    {
        HO_ASSERT(false, "Render queue is not setup yet. You must call 'BeginFrame' before beginning a render pass.");
        return false;
    }
    if (mpRecordingPass)
    {
        HO_ASSERT(false, "Render pass was already begun. You must call 'EndRenderPass' before beginning a new one.");
        return false;
    }

    DirLightShadowMapRenderPass* pRecordingPass = nullptr;
    for (auto& pass : mDirLightShadowMapRenderPassPool)
    {
        if (!pass.IsBound())
        {
            pRecordingPass = &pass;
            break;
        }
    }

    if (!pRecordingPass)
    {
        HO_ASSERT(false, "Every pass are bound already. Check mRenderPassPoolSize.");
        return false;
    }

    pRecordingPass->Setup(desc);

    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    if (!queue.PushRenderPass(pRecordingPass))
    {
        return false;
    }

    mpRecordingPass = pRecordingPass;

    return true;
}

bool RenderingSystem::BeginPointLightShadowMapRenderPass(const PointLightShadowMapRenderPassDesc& desc)
{
    if (!mRenderQueues[mFrontRenderQueueIndex].IsSetup())
    {
        HO_ASSERT(false, "Render queue is not setup yet. You must call 'BeginFrame' before beginning a render pass.");
        return false;
    }
    if (mpRecordingPass)
    {
        HO_ASSERT(false, "Render pass was already begun. You must call 'EndRenderPass' before beginning a new one.");
        return false;
    }

    PointLightShadowMapRenderPass* pRecordingPass = nullptr;
    for (auto& pass : mPointLightShadowMapRenderPassPool)
    {
        if (!pass.IsBound())
        {
            pRecordingPass = &pass;
            break;
        }
    }

    if (!pRecordingPass)
    {
        HO_ASSERT(false, "Every pass are bound already. Check mRenderPassPoolSize.");
        return false;
    }

    pRecordingPass->Setup(desc);

    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    if (!queue.PushRenderPass(pRecordingPass))
    {
        return false;
    }

    mpRecordingPass = pRecordingPass;

    return true;
}

bool RenderingSystem::BeginSpotLightShadowMapRenderPass(const SpotLightShadowMapRenderPassDesc& desc)
{
    if (!mRenderQueues[mFrontRenderQueueIndex].IsSetup())
    {
        HO_ASSERT(false, "Render queue is not setup yet. You must call 'BeginFrame' before beginning a render pass.");
        return false;
    }
    if (mpRecordingPass)
    {
        HO_ASSERT(false, "Render pass was already begun. You must call 'EndRenderPass' before beginning a new one.");
        return false;
    }

    SpotLightShadowMapRenderPass* pRecordingPass = nullptr;
    for (auto& pass : mSpotLightShadowMapRenderPassPool)
    {
        if (!pass.IsBound())
        {
            pRecordingPass = &pass;
            break;
        }
    }

    if (!pRecordingPass)
    {
        HO_ASSERT(false, "Every pass are bound already. Check mRenderPassPoolSize.");
        return false;
    }

    pRecordingPass->Setup(desc);

    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    if (!queue.PushRenderPass(pRecordingPass))
    {
        return false;
    }

    mpRecordingPass = pRecordingPass;

    return true;
}

bool RenderingSystem::BeginForwardRenderPass(const ForwardRenderPassDesc& desc)
{
    if (!mRenderQueues[mFrontRenderQueueIndex].IsSetup())
    {
        HO_ASSERT(false, "Render queue is not setup yet. You must call 'BeginFrame' before beginning a render pass.");
        return false;
    }
    if (mpRecordingPass)
    {
        HO_ASSERT(false, "Render pass was already begun. You must call 'EndRenderPass' before beginning a new one.");
        return false;
    }

    ForwardRenderPass* pRecordingPass = nullptr;
    for (auto& pass : mForwardRenderPassPool)
    {
        if (!pass.IsBound())
        {
            pRecordingPass = &pass;
            break;
        }
    }

    if (!pRecordingPass)
    {
        HO_ASSERT(false, "Every pass are bound already. Check mRenderPassPoolSize.");
        return false;
    }

    auto it = mNameToFrameBufferMap.find(desc.hFrameBufferName);
    if (it == mNameToFrameBufferMap.end())
    {
        return false;
    }

    pRecordingPass->Setup(it->second.hFrameBuffers[mFrontRenderQueueIndex], desc);

    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    if (!queue.PushRenderPass(pRecordingPass))
    {
        return false;
    }

    mpRecordingPass = pRecordingPass;

    return true;
}

void RenderingSystem::AttachOpaqueRenderStage(const OpaqueRenderStageDesc& desc)
{
    if (!mpRecordingPass)
    {
        HO_ASSERT(false, "Pass is not begun. You must begin Pass before beginning stage.");
        return;
    }
    if (mpRecordingPass->IsStageAttached(eRenderStageType::Opaque))
    {
        HO_ASSERT(false, "Stage is already attached.");
        return;
    }

    static_cast<OpaqueRenderStage*>(mpRecordingPass->GetStage(eRenderStageType::Opaque))->Setup(desc);

    mpRecordingPass->AttachStage(eRenderStageType::Opaque);
}

void RenderingSystem::AttachMaskedRenderStage(const MaskedRenderStageDesc& desc)
{
    if (!mpRecordingPass)
    {
        HO_ASSERT(false, "Pass is not begun. You must begin Pass before beginning stage.");
        return;
    }
    if (mpRecordingPass->IsStageAttached(eRenderStageType::Masked))
    {
        HO_ASSERT(false, "Stage is already attached.");
        return;
    }

    static_cast<MaskedRenderStage*>(mpRecordingPass->GetStage(eRenderStageType::Masked))->Setup(desc);

    mpRecordingPass->AttachStage(eRenderStageType::Masked);
}

void RenderingSystem::AttachInWorldDebugRenderStage(const InWorldDebugRenderStageDesc& desc)
{
    if (!mpRecordingPass)
    {
        HO_ASSERT(false, "Pass is not begun. You must begin Pass before beginning stage.");
        return;
    }
    if (mpRecordingPass->IsStageAttached(eRenderStageType::InWorldDebug))
    {
        HO_ASSERT(false, "Stage is already attached.");
        return;
    }

    static_cast<InWorldDebugRenderStage*>(mpRecordingPass->GetStage(eRenderStageType::InWorldDebug))->Setup(desc);

    mpRecordingPass->AttachStage(eRenderStageType::InWorldDebug);
}

void RenderingSystem::AttachEnvironmentRenderStage(const EnvironmentRenderStageDesc& desc)
{
    if (!mpRecordingPass)
    {
        HO_ASSERT(false, "Pass is not begun. You must begin Pass before beginning stage.");
        return;
    }
    if (mpRecordingPass->IsStageAttached(eRenderStageType::Environment))
    {
        HO_ASSERT(false, "Stage is already attached.");
        return;
    }

    static_cast<EnvironmentRenderStage*>(mpRecordingPass->GetStage(eRenderStageType::Environment))->Setup(desc);

    mpRecordingPass->AttachStage(eRenderStageType::Environment);
}

void RenderingSystem::AttachBlendedRenderStage(const BlendedRenderStageDesc& desc)
{
    if (!mpRecordingPass)
    {
        HO_ASSERT(false, "Pass is not begun. You must begin Pass before beginning stage.");
        return;
    }
    if (mpRecordingPass->IsStageAttached(eRenderStageType::Blended))
    {
        HO_ASSERT(false, "Stage is already attached.");
        return;
    }

    static_cast<BlendedRenderStage*>(mpRecordingPass->GetStage(eRenderStageType::Blended))->Setup(desc);

    mpRecordingPass->AttachStage(eRenderStageType::Blended);
}

void RenderingSystem::AttachOutlineRenderStage(const OutlineRenderStageDesc& desc)
{
    if (!mpRecordingPass)
    {
        HO_ASSERT(false, "Pass is not begun. You must begin Pass before beginning stage.");
        return;
    }
    if (mpRecordingPass->IsStageAttached(eRenderStageType::Outline))
    {
        HO_ASSERT(false, "Stage is already attached.");
        return;
    }

    static_cast<OutlineRenderStage*>(mpRecordingPass->GetStage(eRenderStageType::Outline))->Setup(desc);

    mpRecordingPass->AttachStage(eRenderStageType::Outline);
}

void RenderingSystem::AttachPostProcessingStage(const PostProcessingStageDesc& desc)
{
    if (!mpRecordingPass)
    {
        HO_ASSERT(false, "Pass is not begun. You must begin Pass before beginning stage.");
        return;
    }
    if (mpRecordingPass->IsStageAttached(eRenderStageType::PostProcessing))
    {
        HO_ASSERT(false, "Stage is already attached.");
        return;
    }

    static_cast<PostProcessingStage*>(mpRecordingPass->GetStage(eRenderStageType::PostProcessing))->Setup(desc);

    mpRecordingPass->AttachStage(eRenderStageType::PostProcessing);
}

void RenderingSystem::AttachWorldOverlayDebugRenderStage(const WorldOverlayDebugRenderStageDesc& desc)
{
    if (!mpRecordingPass)
    {
        HO_ASSERT(false, "Pass is not begun. You must begin Pass before beginning stage.");
        return;
    }
    if (mpRecordingPass->IsStageAttached(eRenderStageType::WorldOverlayDebug))
    {
        HO_ASSERT(false, "Stage is already attached.");
        return;
    }

    static_cast<WorldOverlayDebugRenderStage*>(mpRecordingPass->GetStage(eRenderStageType::WorldOverlayDebug))
        ->Setup(desc);

    mpRecordingPass->AttachStage(eRenderStageType::WorldOverlayDebug);
}

void RenderingSystem::EndFrame()
{
    HO_ASSERT(!mpRecordingPass, "There is recording pass. You must call 'EndRenderPass' before end frame.");
}

void RenderingSystem::EndRenderPass()
{
    HO_ASSERT(mpRecordingPass, "Render pass was already ended. You must call 'BeginRenderPass' before ending.");

    mpRecordingPass = nullptr;
}

void RenderingSystem::BeginSubmitDrawCommands()
{
    HO_ASSERT(mpRecordingPass, "Pass is not begun. You must begin Pass before submitting draw command.");
    mRenderQueues[mFrontRenderQueueIndex].BeginRecordDrawCommand();
}

bool RenderingSystem::SubmitDrawCommandsToStage(eRenderStageType stage,
                                                const DrawCommandDesc& commandDesc,
                                                int32_t instanceCount,
                                                const InstanceData* pInstanceData)
{
    if (!mpRecordingPass)
    {
        HO_ASSERT(false, "Render pass is not begun. You must call 'BeginRenderPass' before submit draw command.");
        return false;
    }
    if (!mpRecordingPass->IsStageAttached(stage))
    {
        HO_ASSERT(false, "Stage is not attached.");
        return false;
    }

    HO_ASSERT(pInstanceData, "Instance data is null.");
    HO_ASSERT(instanceCount > 0, "Instance count must be greater than zero.");
    HO_ASSERT(commandDesc.hStaticMesh.IsValid(), "Static mesh is not uploaded at all.");
    HO_ASSERT(commandDesc.hMaterial.IsValid(), "Material is not uploaded at all.");

    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    HO_ASSERT(!queue.IsPrepared(), "Draw command must be pushed before execution.");

    if (commandDesc.hStaticMesh->State == eGpuResourceState::PendingUpload ||
        commandDesc.hMaterial->State == eGpuResourceState::PendingUpload)
    {
        return false;
    }

    const int32_t instanceOffset = queue.GetInstanceDataCount();

    InstanceData* pAllocBegin = queue.AllocateInstanceDataBuffer(instanceCount);
    if (!pAllocBegin)
    {
        return false;
    }

    std::memcpy(pAllocBegin, pInstanceData, sizeof(InstanceData) * instanceCount);

    DrawCommand newCommand;
    newCommand.hGpuStaticMesh = commandDesc.hStaticMesh;
    newCommand.hGpuMaterial = commandDesc.hMaterial;
    newCommand.VertexOffset = commandDesc.VertexOffset;
    newCommand.IndexOffset = commandDesc.IndexOffset;
    newCommand.IndexCount = commandDesc.IndexCount;
    newCommand.InstanceOffset = instanceOffset;
    newCommand.InstanceCount = instanceCount;

    const Vector3 worldCameraPos = mpRecordingPass->GetWorldCameraPos();
    mpRecordingPass->GetStage(stage)->SortInstanceData(pAllocBegin, instanceCount, worldCameraPos);
    newCommand.SortDepth = (worldCameraPos - Vector3(pAllocBegin[0].WorldMat.GetCol3())).SqrdMagnitude();

    newCommand.StencilWriteBitIndex = commandDesc.StencilWriteBitIndex;
    if (!mRenderQueues[mFrontRenderQueueIndex].PushDrawCommand(stage, newCommand))
    {
        return false;
    }

    return true;
}

bool RenderingSystem::SubmitDrawCommandWithMorphToStage(eRenderStageType stage,
                                                        const DrawCommandDesc& commandDesc,
                                                        GpuMorphTargetsHandle hMorphTargets,
                                                        const MorphControlData& morphControlData,
                                                        int32_t instanceCount,
                                                        const InstanceData* pInstanceData)
{
    HO_ASSERT(hMorphTargets.IsValid(), "Invalid morph target. It might not be uploaded yet.");
    HO_ASSERT(morphControlData.SubMeshIndex >= 0, "Submesh index cannot be negative.");
    HO_ASSERT(morphControlData.TargetCount > 0, "Morph control data must contain at least one active target.");

    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    if (queue.IsMorphControlDataBufferFull())
    {
        HO_ASSERT(false, "Morph control data buffer is full.");
        return false;
    }

    if (hMorphTargets->State == eGpuResourceState::PendingUpload)
    {
        return false;
    }

    if (!SubmitDrawCommandsToStage(stage, commandDesc, instanceCount, pInstanceData))
    {
        return false;
    }

    const int32_t morphOffset = queue.GetMorphControlDataCount();

    MorphControlData* pAllocBegin = queue.AllocateMorphControlDataBuffer(1);
    HO_ASSERT(pAllocBegin, "Failed to allocate morph control data buffer.");
    std::memcpy(pAllocBegin, &morphControlData, sizeof(MorphControlData));
    pAllocBegin->SubMeshIndex = hMorphTargets->SubMeshBaseTargetOffset[morphControlData.SubMeshIndex];

    DrawCommand& newCommand = mRenderQueues[mFrontRenderQueueIndex].GetDrawCommand(
        stage, mRenderQueues[mFrontRenderQueueIndex].GetDrawCommandCount(stage) - 1);
    newCommand.hGpuMorphTargets = hMorphTargets;
    newCommand.MorphControlDataOffset = morphOffset;

    return true;
}

void RenderingSystem::EndSubmitDrawCommands()
{
    mRenderQueues[mFrontRenderQueueIndex].EndRecordDrawCommand();
}

bool RenderingSystem::SubmitUIViewportData(const IPlatformWindow* pTargetWindow, const ImDrawData& srcDrawData)
{
    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    if (!queue.PushUIViewportData(pTargetWindow, srcDrawData))
    {
        HO_ASSERT(false, "Failed to push ui viewports. Check 'sMaxUIViewportCount' in render queue.");
        return false;
    }
    return true;
}

const RenderingBuiltInAssets& RenderingSystem::GetBuiltInAssets() const
{
    HO_ASSERT(mbBuiltInAssetLoaded, "Built-in assets are not loaded yet.");
    return mBuiltInAssets;
}

bool RenderingSystem::UploadStaticMesh(ObjectHandle<RStaticMeshAsset> hStaticMesh, bool bPersistent)
{
    if (hStaticMesh.IsNULL() || !AssetSystem::GetInstance().IsUploaded(hStaticMesh))
    {
        return false;
    }

    if (hStaticMesh->hRenderProxy.IsValid() && hStaticMesh->hRenderProxy->State != eGpuResourceState::Obsolete)
    {
        return false;
    }

    if (mhPendingStaticMeshQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return false;
    }

    const GpuStaticMeshHandle hGpuStaticMesh = mpBackend->PrepareUploadStaticMesh(hStaticMesh, bPersistent);
    if (hGpuStaticMesh.IsNULL())
    {
        HO_ASSERT(false, "Failed to prepare static mesh upload. Asset might be invalid.");
        return false;
    }

    mPendingStaticMeshQueueLock.Lock();
    mhPendingStaticMeshQueue.Push(hGpuStaticMesh);
    mPendingStaticMeshQueueLock.Unlock();

    return true;
}

bool RenderingSystem::UploadMaterial(ObjectHandle<RMaterialAsset> hMaterial, bool bPersistent)
{
    if (hMaterial.IsNULL() || !AssetSystem::GetInstance().IsUploaded(hMaterial))
    {
        return false;
    }

    if (hMaterial->hRenderProxy.IsValid() && hMaterial->hRenderProxy->State != eGpuResourceState::Obsolete)
    {
        return false;
    }

    if (mhPendingMaterialQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return false;
    }
    const GpuMaterialHandle hGpuMaterial = mpBackend->PrepareUploadMaterial(hMaterial, bPersistent);
    if (hGpuMaterial.IsNULL())
    {
        HO_ASSERT(false, "Preparing upload was failed. Asset might be invalid.");
        return false;
    }

    mPendingMaterialQueueLock.Lock();
    mhPendingMaterialQueue.Push(hGpuMaterial);
    mPendingMaterialQueueLock.Unlock();

    return true;
}

bool RenderingSystem::UploadTexture(ObjectHandle<RTextureAsset> hTexture, bool bPersistent)
{
    if (hTexture.IsNULL() || !AssetSystem::GetInstance().IsUploaded(hTexture))
    {
        return false;
    }

    if (hTexture->hRenderProxy.IsValid() && hTexture->hRenderProxy->State != eGpuResourceState::Obsolete)
    {
        return false;
    }

    if (mhPendingTextureQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return false;
    }
    const GpuTextureHandle hGpuTexture = mpBackend->PrepareUploadTexture(hTexture, bPersistent);
    if (hGpuTexture.IsNULL())
    {
        HO_ASSERT(false, "Preparing upload was failed. Asset might be invalid.");
        return false;
    }

    mPendingTextureQueueLock.Lock();
    mhPendingTextureQueue.Push(hGpuTexture);
    mPendingTextureQueueLock.Unlock();
    return true;
}

bool RenderingSystem::UploadShader(ObjectHandle<RShaderAsset> hShader, bool bPersistent)
{
    if (hShader.IsNULL() || !AssetSystem::GetInstance().IsUploaded(hShader))
    {
        return false;
    }

    if (hShader->hRenderProxy.IsValid() && hShader->hRenderProxy->State != eGpuResourceState::Obsolete)
    {
        return false;
    }

    if (mhPendingShaderQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return false;
    }
    const GpuShaderHandle hGpuShader = mpBackend->PrepareUploadShader(hShader, bPersistent);
    if (hGpuShader.IsNULL())
    {
        HO_ASSERT(false, "Preparing upload was failed. Asset might be invalid.");
        return false;
    }

    mPendingShaderQueueLock.Lock();
    mhPendingShaderQueue.Push(hGpuShader);
    mPendingShaderQueueLock.Unlock();
    return true;
}

bool RenderingSystem::UploadMorphTargets(ObjectHandle<RMorphTargetsAsset> hMorphTargets, bool bPersistent)
{
    if (hMorphTargets.IsNULL() || !AssetSystem::GetInstance().IsUploaded(hMorphTargets))
    {
        return false;
    }

    if (hMorphTargets->hRenderProxy.IsValid() && hMorphTargets->hRenderProxy->State != eGpuResourceState::Obsolete)
    {
        return false;
    }

    if (mhPendingMorphTargetsQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return false;
    }
    const GpuMorphTargetsHandle hGpuMorphTargets = mpBackend->PrepareUploadMorphTargets(hMorphTargets, bPersistent);
    if (hMorphTargets.IsNULL())
    {
        HO_ASSERT(false, "Preparing upload was failed. Asset might be invalid.");
        return false;
    }

    mPendingMorphTargetsQueueLock.Lock();
    mhPendingMorphTargetsQueue.Push(hGpuMorphTargets);
    mPendingMorphTargetsQueueLock.Unlock();
    return true;
}

bool RenderingSystem::UploadMaterialTexture(ObjectHandle<RMaterialAsset> hMaterial,
                                            eMaterialTextureUsage usage,
                                            bool bPersistent)
{
    HO_ASSERT(hMaterial.IsValid(), "Invalid material asset.");
    HO_ASSERT(AssetSystem::GetInstance().IsUploaded(hMaterial) && hMaterial->hRenderProxy.IsValid(),
              "Material is not uploaded yet.");

    bool bSuccess = true;
    if (hMaterial->hTextures[static_cast<int32_t>(usage)].IsValid())
    {
        if (!AssetSystem::GetInstance().IsUploaded(hMaterial->hTextures[static_cast<int32_t>(usage)]))
        {
            return false;
        }
        else if (!IsUploaded(hMaterial->hTextures[static_cast<int32_t>(usage)]))
        {
            bSuccess = UploadTexture(hMaterial->hTextures[static_cast<int32_t>(usage)], bPersistent);
        }
        hMaterial->hRenderProxy->hGpuTextures[static_cast<int32_t>(usage)] =
            hMaterial->hTextures[static_cast<int32_t>(usage)]->hRenderProxy;
    }
    return bSuccess;
}

bool RenderingSystem::UploadMaterialTextures(ObjectHandle<RMaterialAsset> hMaterial, bool bPersistent)
{
    HO_ASSERT(hMaterial.IsValid(), "Invalid material asset.");
    HO_ASSERT(AssetSystem::GetInstance().IsUploaded(hMaterial) && hMaterial->hRenderProxy.IsValid(),
              "Material is not uploaded yet.");

    bool bSuccess = true;
    for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
    {
        if (hMaterial->hTextures[i].IsValid())
        {
            if (!AssetSystem::GetInstance().IsUploaded(hMaterial->hTextures[i]))
            {
                bSuccess = false;
                continue;
            }
            else if (!IsUploaded(hMaterial->hTextures[i]))
            {
                bSuccess &= UploadTexture(hMaterial->hTextures[i], bPersistent);
            }
            hMaterial->hRenderProxy->hGpuTextures[i] = hMaterial->hTextures[i]->hRenderProxy;
        }
    }
    return bSuccess;
}

bool RenderingSystem::UploadMaterialShaders(ObjectHandle<RMaterialAsset> hMaterial, bool bPersistent)
{
    HO_ASSERT(hMaterial.IsValid(), "Invalid material asset.");
    HO_ASSERT(hMaterial->hRenderProxy.IsValid(), "Material is not uploaded yet.");
    bool bSuccess = true;
    for (int32_t i = 0; i < static_cast<int32_t>(eShaderStage::Last); ++i)
    {
        if (hMaterial->hShaders[i].IsValid())
        {
            if (!AssetSystem::GetInstance().IsUploaded(hMaterial->hShaders[i]))
            {
                bSuccess = false;
                continue;
            }
            if (!IsUploaded(hMaterial->hShaders[i]))
            {
                bSuccess &= UploadShader(hMaterial->hShaders[i], bPersistent);
            }
            hMaterial->hRenderProxy->hGpuShaders[i] = hMaterial->hShaders[i]->hRenderProxy;
        }
    }
    return bSuccess;
}

bool RenderingSystem::IsVSyncEnabled() const
{
    return mbVSyncEnabled.Get();
}

void RenderingSystem::SetVSync(bool bEnabled)
{
    mbVSyncEnabled.Set(bEnabled);
}

RenderingSystem::RenderingSystem(eGraphicsAPI api)
  : mFrameBufferToCreateQueue(sPendingFrameBufferQueueSize)
  , mhFrameBufferToDestroyQueue(sPendingFrameBufferQueueSize)
  , mhPendingStaticMeshQueue(sPendingResourceQueueSize)
  , mhPendingMaterialQueue(sPendingResourceQueueSize)
  , mhPendingTextureQueue(sPendingResourceQueueSize)
  , mhPendingShaderQueue(sPendingResourceQueueSize)
  , mhPendingMorphTargetsQueue(sPendingResourceQueueSize)
  , mbVSyncEnabled(true)
{
    HO_ASSERT(!spInstance, "Rendering system muse be created by 'createInstance' function.");

    switch (api)
    {
        case GL:
            mpBackend = new RenderingBackendGL();
            break;
        default:
            HO_ASSERT(false, "Unsupported API.");
    }

    mEnvironmentBakePassPool = FixedArray<EnvironmentBakePass>(sRenderPassPoolSize * sSwapChainSize,
                                                               EnvironmentBakePass(mpBackend, &mBuiltInAssets));
    mShadowMapBakePassPool = FixedArray<ShadowMapBakePass>(sRenderPassPoolSize * sSwapChainSize,
                                                           ShadowMapBakePass(mpBackend, &mBuiltInAssets));
    mDirLightShadowMapRenderPassPool =
        FixedArray<DirLightShadowMapRenderPass>(DirectLightShadowData::sMaxDirLightShadowDataCount * sSwapChainSize * 4,
                                                DirLightShadowMapRenderPass(mpBackend, &mBuiltInAssets));
    mPointLightShadowMapRenderPassPool = FixedArray<PointLightShadowMapRenderPass>(
        DirectLightShadowData::sMaxPointLightShadowDataCount * sSwapChainSize * 4,
        PointLightShadowMapRenderPass(mpBackend, &mBuiltInAssets));
    mSpotLightShadowMapRenderPassPool = FixedArray<SpotLightShadowMapRenderPass>(
        DirectLightShadowData::sMaxSpotLightShadowDataCount * sSwapChainSize * 4,
        SpotLightShadowMapRenderPass(mpBackend, &mBuiltInAssets));
    mForwardRenderPassPool = FixedArray<ForwardRenderPass>(sRenderPassPoolSize * sSwapChainSize,
                                                           ForwardRenderPass(mpBackend, &mBuiltInAssets));

    mRenderQueues = FixedArray<RenderQueue>(
        sSwapChainSize,
        RenderQueue(mpBackend, sInitialDrawCommandCount, sInitialInstanceDataCount, sInitialMorphControlDataCount));
}

RenderingSystem::~RenderingSystem()
{
    delete mpBackend;
}

void RenderingSystem::createInstance(eGraphicsAPI api)
{
    if (spInstance)
    {
        HO_ASSERT(false, "Rendering system is already created.");
    }
    else
    {
        // Rendering Backend created before engering constructor.

        spInstance = new RenderingSystem(api);
    }
}

void RenderingSystem::deleteInstance()
{
    if (!spInstance)
    {
        HO_ASSERT(false, "Rendering system is not created.");
    }
    else
    {
        delete spInstance;
        spInstance = nullptr;
    }
}

bool RenderingSystem::init()
{
    // Its to enture 'swapRenderQueues' works correctly.
    mRenderQueues[mFrontRenderQueueIndex].Setup();
    mRenderQueues[mBackRenderQueueIndex].Setup();

    mpBackend->BeginFrame(0);

    if (!mpBackend->Init())
    {
        HO_ASSERT(false, "Failed to initialize rendering backend.");
        return false;
    }

    if (!uploadResourceContext())
    {
        HO_ASSERT(false, "Failed to upload resource context.");
        mpBackend->UnloadAllResources();
        return false;
    }

    if (!uploadBuiltInAssets())
    {
        HO_ASSERT(false, "Failed to upload built-in assets.");
        mpBackend->UnloadAllResources();
        return false;
    }

    mpBackend->EndFrame();

    return true;
}

void RenderingSystem::run()
{
    HO_ASSERT(spInstance, "Rendering system is not created.");

    // This deactivation is needed for render thread to activate context.
    IPlatformApplication::GetInstance().GetMainWindow()->DeactivateContext();
    mpRenderThread = std::make_unique<Thread>(reinterpret_cast<Thread::Entry>(runInternal));
    mpRenderThread->SetName("Render Thread");
}

void RenderingSystem::runInternal(void* unused)
{
    (void)unused;
    IPlatformApplication::GetInstance().GetMainWindow()->ActivateContext();

    while (spInstance->mbRunning)
    {
        spInstance->mRenderSync.WaitUntilEngineSwapComplete();

        if (!spInstance->mbRunning)
        {
            break;
        }

        spInstance->mpBackend->BeginFrame(spInstance->mCurrentFrame);

        spInstance->mpBackend->SetVSync(spInstance->mbVSyncEnabled.Get());

        spInstance->destroyPendingFrameBuffers();
        spInstance->createPendingFrameBuffers();

        spInstance->uploadPendingResources();

        if (spInstance->mRenderQueues[spInstance->mBackRenderQueueIndex].IsSetup())
        {
            RenderQueue& queue = spInstance->mRenderQueues[spInstance->mBackRenderQueueIndex];

            queue.PrepareExecute(spInstance->mResourceContext);
            queue.Execute(spInstance->mResourceContext);
            queue.FinishExecute(spInstance->mResourceContext);
        }

        spInstance->mRenderQueues[spInstance->mBackRenderQueueIndex].ResizeBuffers(spInstance->mResourceContext);

        spInstance->mpBackend->Flush(); // TODO: CPU might do something while waiting GPU completes own jobs.

        spInstance->mpBackend->EvictStaleResources(1000);

        spInstance->mpBackend->EndFrame();

        spInstance->mRenderSync.NotifyRenderDone();
    }

    spInstance->mpBackend->BeginFrame(spInstance->mCurrentFrame);
    spInstance->mpBackend->Shutdown();
    spInstance->mpBackend->EndFrame();

    IPlatformApplication::GetInstance().GetMainWindow()->DeactivateContext();
}

void RenderingSystem::shutdown()
{
    // Every resource created in 'init' function will be deleted
    // through 'UnloadAllResources' and RenderingBackend's 'Shutdown' function.
    mRenderSync.WaitUntilRenderDone();

    mbRunning = false;

    mRenderSync.NotifyEngineSwapComplete();
    mpRenderThread->Join();
}

bool RenderingSystem::uploadBuiltInAssets()
{
    HO_ASSERT(!mbBuiltInAssetLoaded, "Built-in assets are loaded already.");
    // Shaders
    Path shaderDirPath(std::string("Subsystems/Rendering/BuiltInAssets/Shaders/GL"));
    shaderDirPath.ResolveProjectPath();

    auto loadShader = [&](const std::string& shaderFileName,
                          GpuShaderHandle& hDstHandle,
                          eShaderStage stage,
                          eMaterialAssetType compatibleMaterialTypes) -> bool
    {
        const std::unique_ptr<const parser::ShaderIR> pShaderIR =
            parser::parseShaderFile(shaderDirPath / Path(shaderFileName), shaderFileName);
        if (!pShaderIR)
        {
            return false;
        }

        const ObjectHandle<RShaderAsset> hShader =
            AssetSystem::GetInstance().ImportShaderImmediate(nullptr, *pShaderIR, stage, compatibleMaterialTypes);
        if (hShader.IsNULL())
        {
            return false;
        }

        hDstHandle = mpBackend->PrepareUploadShader(hShader, true);
        if (hDstHandle.IsNULL())
        {
            return false;
        }
        mpBackend->UploadShader(hDstHandle);

        AssetSystem::GetInstance().DeleteAssetImmediate(hShader);
        return true;
    };

    if (!loadShader("Default.vert", mBuiltInAssets.hDefaultVS, eShaderStage::VertexShader, eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("ForwardUnlitOpaque.frag",
                    mBuiltInAssets.hForwardUnlitOpaqueFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::Unlit))
    {
        return false;
    }

    if (!loadShader("ForwardUnlitMasked.frag",
                    mBuiltInAssets.hForwardUnlitMaskedFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::Unlit))
    {
        return false;
    }

    if (!loadShader("ForwardUnlitBlended.frag",
                    mBuiltInAssets.hForwardUnlitBlendedFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::Unlit))
    {
        return false;
    }

    if (!loadShader("ForwardPhongOpaque.frag",
                    mBuiltInAssets.hForwardPhongOpaqueFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::Legacy))
    {
        return false;
    }

    if (!loadShader("ForwardPhongMasked.frag",
                    mBuiltInAssets.hForwardPhongMaskedFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::Legacy))
    {
        return false;
    }

    if (!loadShader("ForwardPhongBlended.frag",
                    mBuiltInAssets.hForwardPhongBlendedFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::Legacy))
    {
        return false;
    }

    if (!loadShader("ForwardPbrOpaque.frag",
                    mBuiltInAssets.hForwardPbrOpaqueFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::StandardLit))
    {
        return false;
    }

    if (!loadShader("ForwardPbrMasked.frag",
                    mBuiltInAssets.hForwardPbrMaskedFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::StandardLit))
    {
        return false;
    }

    if (!loadShader("ForwardPbrBlended.frag",
                    mBuiltInAssets.hForwardPbrBlendedFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::StandardLit))
    {
        return false;
    }

    if (!loadShader(
            "RadianceMap.vert", mBuiltInAssets.hRadianceMapVS, eShaderStage::VertexShader, eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader(
            "RadianceMap.frag", mBuiltInAssets.hRadianceMapFS, eShaderStage::FragmentShader, eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("ToneMappingGammaCorrection.vert",
                    mBuiltInAssets.hToneMappingGammaCorrectionVS,
                    eShaderStage::VertexShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("ToneMappingGammaCorrection.frag",
                    mBuiltInAssets.hToneMappingGammaCorrectionFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader(
            "WorldGizmo.vert", mBuiltInAssets.hWorldGizmoVS, eShaderStage::VertexShader, eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("PerspectiveWorldGizmo.frag",
                    mBuiltInAssets.hPerspectiveWorldGizmoFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("OrthographicWorldGizmo.frag",
                    mBuiltInAssets.hOrthographicWorldGizmoFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("DirLightShadowMap.vert",
                    mBuiltInAssets.hDirLightShadowMapVS,
                    eShaderStage::VertexShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("DirLightShadowMap.geom",
                    mBuiltInAssets.hDirLightShadowMapGS,
                    eShaderStage::GeometryShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("DirLightShadowMap.frag",
                    mBuiltInAssets.hDirLightShadowMapFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("DirLightShadowMapMasked.frag",
                    mBuiltInAssets.hDirLightShadowMapMaskedFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("PointLightShadowMap.vert",
                    mBuiltInAssets.hPointLightShadowMapVS,
                    eShaderStage::VertexShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("PointLightShadowMap.geom",
                    mBuiltInAssets.hPointLightShadowMapGS,
                    eShaderStage::GeometryShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("PointLightShadowMap.frag",
                    mBuiltInAssets.hPointLightShadowMapFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("PointLightShadowMapMasked.frag",
                    mBuiltInAssets.hPointLightShadowMapMaskedFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("SpotLightShadowMap.vert",
                    mBuiltInAssets.hSpotLightShadowMapVS,
                    eShaderStage::VertexShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("SpotLightShadowMap.frag",
                    mBuiltInAssets.hSpotLightShadowMapFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("SpotLightShadowMapMasked.frag",
                    mBuiltInAssets.hSpotLightShadowMapMaskedFS,
                    eShaderStage::FragmentShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("Outline.vert", mBuiltInAssets.hOutlineVS, eShaderStage::VertexShader, eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader(
            "SingleColor.frag", mBuiltInAssets.hSingleColorFS, eShaderStage::FragmentShader, eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("IrradianceConvolution.comp",
                    mBuiltInAssets.hIrradianceConvolutionCS,
                    eShaderStage::ComputeShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    if (!loadShader("PrefilteredEnvironmentConvolution.comp",
                    mBuiltInAssets.hPrefilteredEnvironmentConvolutionCS,
                    eShaderStage::ComputeShader,
                    eMaterialAssetType::None))
    {
        return false;
    }

    // Create and Render BRDF Integration Map

    GpuShaderHandle hBRDFIntegrationCS;
    if (!loadShader("BRDFIntegration.comp", hBRDFIntegrationCS, eShaderStage::ComputeShader, eMaterialAssetType::None))
    {
        return false;
    }

    mpBackend->BeginDrawSequence();

    mBuiltInAssets.hBRDFIntegrationMap =
        mpBackend->PrepareCreateTexture2D(eTextureFormat::R16G16_FLOAT, 512, 512, 1, 1, true);
    mpBackend->CreateTexture(mBuiltInAssets.hBRDFIntegrationMap);

    mpBackend->BindShader(hBRDFIntegrationCS, eShaderStage::ComputeShader);
    mpBackend->BindStorageTexture(mBuiltInAssets.hBRDFIntegrationMap,
                                  eStorageTextureBindingIndex::BRDFIntegrationMap,
                                  0,
                                  0,
                                  IRenderingBackend::eResourceAccessType::WriteOnly);
    mpBackend->Dispatch(
        static_cast<uint32_t>(math::Ceil(static_cast<float>(mBuiltInAssets.hBRDFIntegrationMap->Width) / 16.0f)),
        static_cast<uint32_t>(math::Ceil(static_cast<float>(mBuiltInAssets.hBRDFIntegrationMap->Height) / 16.0f)),
        1);

    mpBackend->BindShader(GpuShaderHandle::sNULL, eShaderStage::ComputeShader);
    mpBackend->BindStorageTexture(GpuTextureHandle::sNULL,
                                  eStorageTextureBindingIndex::BRDFIntegrationMap,
                                  0,
                                  0,
                                  IRenderingBackend::eResourceAccessType::WriteOnly);

    mpBackend->UnloadShader(hBRDFIntegrationCS);

    mpBackend->EndDrawSequence();

    if (mBuiltInAssets.hBRDFIntegrationMap.IsNULL())
    {
        return false;
    }

    // Textures
    Path textureDirPath(std::string("Subsystems/Rendering/BuiltInAssets/Textures"));
    textureDirPath.ResolveProjectPath();

    auto loadTexture2D = [&](const std::unique_ptr<const parser::TextureIR>& pTexIR,
                             GpuTextureHandle& hDstHandle,
                             bool bIsLinear) -> bool
    {
        const ObjectHandle<RTextureAsset> hTex = AssetSystem::GetInstance().ImportTexture2DImmediate(
            *pTexIR, bIsLinear ? eTextureFormat::BC7_UNORM : eTextureFormat::BC7_SRGB, true);

        if (hTex.IsNULL())
        {
            return false;
        }

        hDstHandle = mpBackend->PrepareUploadTexture(hTex, true);
        if (hDstHandle.IsNULL())
        {
            return false;
        }
        mpBackend->UploadTexture(hDstHandle);

        AssetSystem::GetInstance().DeleteAssetImmediate(hTex);
        return true;
    };

    auto loadTextureCubeMap = [&](const std::unique_ptr<const parser::TextureIR>& pTexIR,
                                  const std::string& assetName,
                                  GpuTextureHandle& hDstHandle,
                                  bool bIsLinear) -> bool
    {
        const ObjectHandle<RTextureAsset> hTex = AssetSystem::GetInstance().ImportTextureCubemapImmediate(
            assetName,
            *pTexIR,
            *pTexIR,
            *pTexIR,
            *pTexIR,
            *pTexIR,
            *pTexIR,
            bIsLinear ? eTextureFormat::BC7_UNORM : eTextureFormat::BC7_SRGB,
            true);

        if (hTex.IsNULL())
        {
            return false;
        }

        hDstHandle = mpBackend->PrepareUploadTexture(hTex, true);
        if (hDstHandle.IsNULL())
        {
            return false;
        }
        mpBackend->UploadTexture(hDstHandle);

        AssetSystem::GetInstance().DeleteAssetImmediate(hTex);

        return true;
    };

    const std::unique_ptr<const parser::TextureIR> p2x2Black = parser::parseTextureFile(
        Path(std::string("Subsystems/Rendering/BuiltInAssets/Textures/2x2Black.png")).ResolvedProjectPath(),
        "2x2Black",
        false);

    if (!loadTextureCubeMap(p2x2Black, "DefaultCubeMap", mBuiltInAssets.hBlackCubeMap, false))
    {
        return false;
    }

    const std::unique_ptr<const parser::TextureIR> pFallback = parser::parseTextureFile(
        Path(std::string("Subsystems/Rendering/BuiltInAssets/Textures/fallback.png")).ResolvedProjectPath(),
        "Fallback",
        false);

    if (!loadTexture2D(pFallback, mBuiltInAssets.hFallbackTexture, false))
    {
        return false;
    }

    if (!loadTextureCubeMap(pFallback, "FallbackCubeMap", mBuiltInAssets.hFallbackCubeMap, false))
    {
        return false;
    }

    const std::unique_ptr<const parser::TextureIR> pCheckerBoard = parser::parseTextureFile(
        Path(std::string("Subsystems/Rendering/BuiltInAssets/Textures/checkerboard.jpg")).ResolvedProjectPath(),
        "CheckerBoard",
        false);

    if (!loadTexture2D(p2x2Black, mBuiltInAssets.hCheckerBoardTexture, false))
    {
        return false;
    }

    if (!loadTextureCubeMap(p2x2Black, "CheckerBoardCubeMap", mBuiltInAssets.hCheckerBoardCubeMap, false))
    {
        return false;
    }

    const std::unique_ptr<const parser::TextureIR> p2x2Gray = parser::parseTextureFile(
        Path(std::string("Subsystems/Rendering/BuiltInAssets/Textures/2x2Gray.png")).ResolvedProjectPath(),
        "2x2Gray",
        false);

    if (!loadTexture2D(p2x2Gray, mBuiltInAssets.hGrayTexture, false))
    {
        return false;
    }

    if (!loadTextureCubeMap(p2x2Gray, "GrayCubeMap", mBuiltInAssets.hGrayCubeMap, false))
    {
        return false;
    }

    const std::unique_ptr<const parser::TextureIR> pMetallic1Roughness0 = parser::parseTextureFile(
        Path(std::string("Subsystems/Rendering/BuiltInAssets/Textures/metallic1roughness0.png")).ResolvedProjectPath(),
        "Metallic1Roughness0",
        true);

    if (!loadTexture2D(pMetallic1Roughness0, mBuiltInAssets.hMetallic1Roughness0Texture, true))
    {
        return false;
    }

    FrameBufferDesc fbDesc;
    fbDesc.hName = StringHandle("FallbackShadowMapRenderFrameBuffer");
    fbDesc.Width = 1;
    fbDesc.Height = 1;
    const FrameBufferHandle hFallbackShadowMapFB = mpBackend->CreateFrameBuffer(fbDesc);
    if (hFallbackShadowMapFB.IsNULL())
    {
        return false;
    }
    mpBackend->SetDepthWrite(true);
    mpBackend->BindFrameBuffer(hFallbackShadowMapFB);

    // NOTE: mResourceContext must be uploaded first.
    mBuiltInAssets.hFallbackDirLightStaticShadowMap = mpBackend->PrepareCreateTexture2DArray(
        eTextureFormat::D32_FLOAT, 1, 1, DirLightShadowData::sMaxCascadeCount, 1, 1, true);
    mpBackend->CreateTexture(mBuiltInAssets.hFallbackDirLightStaticShadowMap);
    if (mBuiltInAssets.hFallbackDirLightStaticShadowMap.IsNULL())
    {
        return false;
    }

    mpBackend->AttachDepthTargetRange(hFallbackShadowMapFB,
                                      mBuiltInAssets.hFallbackDirLightStaticShadowMap,
                                      0,
                                      DirLightShadowData::sMaxCascadeCount,
                                      0);
    mpBackend->ClearDepthTarget(hFallbackShadowMapFB, 1.0f);
    mpBackend->DetachDepthTarget(hFallbackShadowMapFB);

    mBuiltInAssets.hFallbackPointLightStaticShadowMap =
        mpBackend->PrepareCreateTextureCubeMap(eTextureFormat::R32_FLOAT, 1, 1, 1, true);
    mpBackend->CreateTexture(mBuiltInAssets.hFallbackPointLightStaticShadowMap);
    if (mBuiltInAssets.hFallbackPointLightStaticShadowMap.IsNULL())
    {
        return false;
    }
    mpBackend->AttachColorTargetRange(hFallbackShadowMapFB,
                                      eRenderTargetBindingIndex::Final,
                                      mBuiltInAssets.hFallbackPointLightStaticShadowMap,
                                      0,
                                      6,
                                      0);
    float clearColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    mpBackend->ClearColorTarget(hFallbackShadowMapFB, eRenderTargetBindingIndex::Final, clearColor);
    mpBackend->DetachColorTarget(hFallbackShadowMapFB, eRenderTargetBindingIndex::Final);

    mBuiltInAssets.hFallbackSpotLightStaticShadowMap =
        mpBackend->PrepareCreateTexture2D(eTextureFormat::R32_FLOAT, 1, 1, 1, 1, true);
    mpBackend->CreateTexture(mBuiltInAssets.hFallbackSpotLightStaticShadowMap);
    if (mBuiltInAssets.hFallbackSpotLightStaticShadowMap.IsNULL())
    {
        return false;
    }
    mpBackend->AttachColorTarget(
        hFallbackShadowMapFB, eRenderTargetBindingIndex::Final, mBuiltInAssets.hFallbackSpotLightStaticShadowMap);
    mpBackend->ClearColorTarget(hFallbackShadowMapFB, eRenderTargetBindingIndex::Final, clearColor);
    mpBackend->DetachColorTarget(hFallbackShadowMapFB, eRenderTargetBindingIndex::Final);

    mpBackend->BindFrameBuffer(FrameBufferHandle::sNULL);
    mpBackend->DestroyFrameBuffer(hFallbackShadowMapFB);

    // Material
    parser::MaterialIR fallbackMatIR;
    fallbackMatIR.NameStr = std::string("FallbackMaterial");
    fallbackMatIR.AlphaMode = parser::MaterialIR::eAlphaMode::Opaque;
    fallbackMatIR.BlendMode = parser::MaterialIR::eAlphaBlendMode::Default;
    fallbackMatIR.bWireframe = false;
    fallbackMatIR.bBackfaceCulling = true;
    const ObjectHandle<RMaterialAsset> hFallbackMat =
        AssetSystem::GetInstance().ImportMaterialImmediate(fallbackMatIR, eMaterialAssetType::StandardLit);
    if (hFallbackMat.IsNULL())
    {
        return false;
    }

    mBuiltInAssets.hDefaultMaterial = mpBackend->PrepareUploadMaterial(hFallbackMat, true);
    if (mBuiltInAssets.hDefaultMaterial.IsNULL())
    {
        return false;
    }
    mpBackend->UploadMaterial(mBuiltInAssets.hDefaultMaterial);
    AssetSystem::GetInstance().DeleteAssetImmediate(hFallbackMat);

    parser::MaterialIR metallic1Roughness0MatIR;
    metallic1Roughness0MatIR.NameStr = std::string("Metallic1Roughness0Material");
    metallic1Roughness0MatIR.AlphaMode = parser::MaterialIR::eAlphaMode::Opaque;
    metallic1Roughness0MatIR.BlendMode = parser::MaterialIR::eAlphaBlendMode::Default;
    metallic1Roughness0MatIR.bWireframe = false;
    metallic1Roughness0MatIR.bBackfaceCulling = true;
    metallic1Roughness0MatIR.Metallic = 1.0f;
    metallic1Roughness0MatIR.Roughness = 0.0f;
    const ObjectHandle<RMaterialAsset> hMetallic1Roughness0Mat =
        AssetSystem::GetInstance().ImportMaterialImmediate(metallic1Roughness0MatIR, eMaterialAssetType::StandardLit);
    if (hMetallic1Roughness0Mat.IsNULL())
    {
        return false;
    }

    mBuiltInAssets.hMetallic1Roughness0Material = mpBackend->PrepareUploadMaterial(hMetallic1Roughness0Mat, true);
    if (mBuiltInAssets.hMetallic1Roughness0Material.IsNULL())
    {
        return false;
    }
    mpBackend->UploadMaterial(mBuiltInAssets.hMetallic1Roughness0Material);
    AssetSystem::GetInstance().DeleteAssetImmediate(hMetallic1Roughness0Mat);

    // Static Meshes
    // Load default shape meshes
    Path staticMeshDirPath = Path(std::string("Subsystems/Rendering/BuiltInAssets/StaticMeshes"));
    staticMeshDirPath.ResolveProjectPath();

    auto loadStaticMesh =
        [&](GpuStaticMeshHandle* phDst, const std::string& relPathStr, const std::string& nameStr) -> bool
    {
        const Path path = staticMeshDirPath / Path(relPathStr);
        const std::unique_ptr<const parser::ModelIR> pIR = parser::parseModelFile(path, nameStr, true);
        if (!pIR)
        {
            return false;
        }

        const ObjectHandle<RStaticMeshAsset> hMesh = AssetSystem::GetInstance().ImportStaticMeshImmediate(
            *pIR->pMeshIR, pIR->pMaterialIRs, *pIR->pSkeletonIR, *pIR->pSkinIR);
        if (hMesh.IsNULL())
        {
            return false;
        }

        *phDst = mpBackend->PrepareUploadStaticMesh(hMesh, true);
        if (phDst->IsNULL())
        {
            return false;
        }
        mpBackend->UploadStaticMesh(*phDst);

        AssetSystem::GetInstance().DeleteAssetImmediate(hMesh);

        return true;
    };

    GpuStaticMeshHandle* const phStaticMeshDst[] = {
        &mBuiltInAssets.hIcosaedroStaticMesh,
        &mBuiltInAssets.hOctahedronStaticMesh,
    };
    const char* const staticMeshRelPath[] = {
        "icosaedro/icosaedro.glb",
        "octahedron/scene.gltf",
    };
    const char* const staticMeshNames[] = {
        "Icosaedro",
        "Octahedron",
    };

    for (int32_t i = 0; i < 2; ++i)
    {
        if (!loadStaticMesh(phStaticMeshDst[i], staticMeshRelPath[i], staticMeshNames[i]))
        {
            for (int j = 0; j < i; ++j)
            {
                mpBackend->UnloadStaticMesh(*phStaticMeshDst[i]);
            }
            return false;
        }
    }

    mbBuiltInAssetLoaded = true;
    return true;
}

bool RenderingSystem::uploadResourceContext()
{
    mResourceContext.hMatrixBuffer = mpBackend->PrepareCreateBuffer(
        static_cast<int32_t>(sizeof(MatrixLayout)), eGpuBufferUsage::Uniform, eGpuBufferAccess::Dynamic, true);
    mpBackend->CreateBuffer(mResourceContext.hMatrixBuffer);
    if (mResourceContext.hMatrixBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hDirectLightBuffer = mpBackend->PrepareCreateBuffer(
        static_cast<int32_t>(sizeof(DirectLightLayout)), eGpuBufferUsage::Uniform, eGpuBufferAccess::Dynamic, true);
    mpBackend->CreateBuffer(mResourceContext.hDirectLightBuffer);
    if (mResourceContext.hDirectLightBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hDirectLightShadowDataBuffer =
        mpBackend->PrepareCreateBuffer(static_cast<int32_t>(sizeof(DirectLightShadowDataLayout)),
                                       eGpuBufferUsage::Uniform,
                                       eGpuBufferAccess::Dynamic,
                                       true);
    mpBackend->CreateBuffer(mResourceContext.hDirectLightShadowDataBuffer);
    if (mResourceContext.hDirectLightShadowDataBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hIndirectLightBuffer = mpBackend->PrepareCreateBuffer(
        static_cast<int32_t>(sizeof(IndirectLightLayout)), eGpuBufferUsage::Uniform, eGpuBufferAccess::Dynamic, true);
    mpBackend->CreateBuffer(mResourceContext.hIndirectLightBuffer);
    if (mResourceContext.hIndirectLightBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hGeneralPurposeBuffer = mpBackend->PrepareCreateBuffer(
        static_cast<int32_t>(sizeof(GeneralPurposeLayout)), eGpuBufferUsage::Uniform, eGpuBufferAccess::Dynamic, true);
    mpBackend->CreateBuffer(mResourceContext.hGeneralPurposeBuffer);
    if (mResourceContext.hGeneralPurposeBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hWorldGizmoDataBuffer = mpBackend->PrepareCreateBuffer(
        static_cast<int32_t>(sizeof(WorldGizmoDataLayout)), eGpuBufferUsage::Uniform, eGpuBufferAccess::Dynamic, true);
    mpBackend->CreateBuffer(mResourceContext.hWorldGizmoDataBuffer);
    if (mResourceContext.hWorldGizmoDataBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hShadowMapRenderDataBuffer =
        mpBackend->PrepareCreateBuffer(static_cast<int32_t>(sizeof(ShadowMapRenderDataLayout)),
                                       eGpuBufferUsage::Uniform,
                                       eGpuBufferAccess::Dynamic,
                                       true);
    mpBackend->CreateBuffer(mResourceContext.hShadowMapRenderDataBuffer);
    if (mResourceContext.hGeneralPurposeBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hEnvironmentBakeDataBuffer =
        mpBackend->PrepareCreateBuffer(static_cast<int32_t>(sizeof(EnvironmentBakeDataLayout)),
                                       eGpuBufferUsage::Uniform,
                                       eGpuBufferAccess::Dynamic,
                                       true);
    mpBackend->CreateBuffer(mResourceContext.hEnvironmentBakeDataBuffer);
    if (mResourceContext.hEnvironmentBakeDataBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hInstanceDataBuffer =
        mpBackend->PrepareCreateBuffer(static_cast<int32_t>(sizeof(InstanceData)) * sInitialInstanceDataCount,
                                       eGpuBufferUsage::Storage,
                                       eGpuBufferAccess::Dynamic,
                                       true);
    mpBackend->CreateBuffer(mResourceContext.hInstanceDataBuffer);
    if (mResourceContext.hInstanceDataBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hDummyMorphControlDataBuffer = mpBackend->PrepareCreateBuffer(
        static_cast<int32_t>(sizeof(MorphControlData)), eGpuBufferUsage::Storage, eGpuBufferAccess::Static, true);
    MorphControlData dummyData;
    dummyData.TargetCount = 0;
    mpBackend->CreateBuffer(mResourceContext.hDummyMorphControlDataBuffer);
    if (mResourceContext.hDummyMorphControlDataBuffer.IsNULL())
    {
        return false;
    }
    mpBackend->UploadBufferData(
        mResourceContext.hDummyMorphControlDataBuffer, &dummyData, static_cast<int32_t>(sizeof(MorphControlData)));

    mResourceContext.hMorphControlDataBuffer =
        mpBackend->PrepareCreateBuffer(static_cast<int32_t>(sizeof(MorphControlData)) * sInitialMorphControlDataCount,
                                       eGpuBufferUsage::Storage,
                                       eGpuBufferAccess::Dynamic,
                                       true);
    mpBackend->CreateBuffer(mResourceContext.hMorphControlDataBuffer);
    if (mResourceContext.hMorphControlDataBuffer.IsNULL())
    {
        return false;
    }

    // TODO: Avoid hardcoding shadow map resolutions.
    // Move these values to render configuration/settings.
    FrameBufferDesc fbDesc;
    fbDesc.hName = StringHandle("DirLightShadowMapRenderFrameBuffer");
    fbDesc.Width = sDirLightShadowMapSize;
    fbDesc.Height = sDirLightShadowMapSize;
    mResourceContext.hDirLightShadowMapFrameBuffer = mpBackend->CreateFrameBuffer(fbDesc);
    if (mResourceContext.hDirLightShadowMapFrameBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hDirLightDynamicShadowMap = mpBackend->PrepareCreateTexture2DArray(
        eTextureFormat::D32_FLOAT,
        sDirLightShadowMapSize,
        sDirLightShadowMapSize,
        DirectLightShadowData::sMaxDirLightShadowDataCount * DirLightShadowData::sMaxCascadeCount,
        1,
        1,
        true);
    mpBackend->CreateTexture(mResourceContext.hDirLightDynamicShadowMap);
    if (mResourceContext.hDirLightDynamicShadowMap.IsNULL())
    {
        return false;
    }

    fbDesc.hName = StringHandle("PointLightShadowMapRenderFrameBuffer");
    fbDesc.Width = sPointLightShadowMapSize;
    fbDesc.Height = sPointLightShadowMapSize;
    mResourceContext.hPointLightShadowMapFrameBuffer = mpBackend->CreateFrameBuffer(fbDesc);
    if (mResourceContext.hPointLightShadowMapFrameBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hPointLightDynamicShadowMap =
        mpBackend->PrepareCreateTextureCubeMapArray(eTextureFormat::R32_FLOAT,
                                                    sPointLightShadowMapSize,
                                                    sPointLightShadowMapSize,
                                                    DirectLightShadowData::sMaxPointLightShadowDataCount,
                                                    1,
                                                    true);
    mpBackend->CreateTexture(mResourceContext.hPointLightDynamicShadowMap);
    if (mResourceContext.hPointLightDynamicShadowMap.IsNULL())
    {
        return false;
    }

    mResourceContext.hPointLightShadowMapDummyDepthBuffer =
        mpBackend->PrepareCreateTextureCubeMapArray(eTextureFormat::D32_FLOAT,
                                                    sPointLightShadowMapSize,
                                                    sPointLightShadowMapSize,
                                                    DirectLightShadowData::sMaxPointLightShadowDataCount,
                                                    1,
                                                    true);
    mpBackend->CreateTexture(mResourceContext.hPointLightShadowMapDummyDepthBuffer);
    if (mResourceContext.hPointLightShadowMapDummyDepthBuffer.IsNULL())
    {
        return false;
    }

    fbDesc.hName = StringHandle("SpotLightShadowMapRenderFrameBuffer");
    fbDesc.Width = sSpotLightShadowMapSize;
    fbDesc.Height = sSpotLightShadowMapSize;
    mResourceContext.hSpotLightShadowMapFrameBuffer = mpBackend->CreateFrameBuffer(fbDesc);
    if (mResourceContext.hSpotLightShadowMapFrameBuffer.IsNULL())
    {
        return false;
    }

    mResourceContext.hSpotLightDynamicShadowMap =
        mpBackend->PrepareCreateTexture2DArray(eTextureFormat::R32_FLOAT,
                                               sSpotLightShadowMapSize,
                                               sSpotLightShadowMapSize,
                                               DirectLightShadowData::sMaxSpotLightShadowDataCount,
                                               1,
                                               1,
                                               true);
    mpBackend->CreateTexture(mResourceContext.hSpotLightDynamicShadowMap);
    if (mResourceContext.hSpotLightDynamicShadowMap.IsNULL())
    {
        return false;
    }

    mResourceContext.hSpotLightShadowMapDummyDepthBuffer =
        mpBackend->PrepareCreateTexture2DArray(eTextureFormat::D32_FLOAT,
                                               sSpotLightShadowMapSize,
                                               sSpotLightShadowMapSize,
                                               DirectLightShadowData::sMaxSpotLightShadowDataCount,
                                               1,
                                               1,
                                               true);
    mpBackend->CreateTexture(mResourceContext.hSpotLightShadowMapDummyDepthBuffer);
    if (mResourceContext.hSpotLightShadowMapDummyDepthBuffer.IsNULL())
    {
        return false;
    }

    return true;
}

// Sets up the front render queue and tears down the back render queue simultaneously.
// This ensures that both operations are performed on the same thread.
// Tearing down the back queue modifies internal pass and stage states; executing this
// concurrently across the main and render threads could cause race conditions.
void RenderingSystem::swapRenderQueues(uint64_t currentFrame)
{
    mRenderSync.WaitUntilRenderDone();

    mRenderQueues[mBackRenderQueueIndex].TearDown();
    mBackRenderQueueIndex = (mBackRenderQueueIndex + 1) % sSwapChainSize;

    mFrontRenderQueueIndex = (mFrontRenderQueueIndex + 1) % sSwapChainSize;
    mRenderQueues[mFrontRenderQueueIndex].Setup();

    mCurrentFrame = currentFrame;

    mRenderSync.NotifyEngineSwapComplete();
}

bool RenderingSystem::trySwapRenderQueues(uint64_t currentFrame)
{
    if (!mRenderSync.IsRenderDone())
    {
        return false;
    }

    swapRenderQueues(currentFrame);
    return true;
}

void RenderingSystem::createPendingFrameBuffers()
{
    while (true)
    {
        FrameBufferDesc desc;
        bool bEmpty = false;

        spInstance->mFrameBufferToCreateQueueLock.Lock();
        if (!spInstance->mFrameBufferToCreateQueue.IsEmpty())
        {
            desc = spInstance->mFrameBufferToCreateQueue.Front();
            spInstance->mFrameBufferToCreateQueue.Pop();
            bEmpty = false;
        }
        else
        {
            bEmpty = true;
        }
        spInstance->mFrameBufferToCreateQueueLock.Unlock();

        if (bEmpty)
        {
            break;
        }

        if (mNameToFrameBufferMap.find(desc.hName) != mNameToFrameBufferMap.end())
        {
            HO_ASSERT(false, "There is frame buffer has same name.");
            continue;
        }

        FrameBufferChain fbChain;
        for (auto& hFrameBuffer : fbChain.hFrameBuffers)
        {
            hFrameBuffer = spInstance->mpBackend->CreateFrameBuffer(desc);
        }

        mNameToFrameBufferMap.insert({desc.hName, fbChain});
    }
}

void RenderingSystem::destroyPendingFrameBuffers()
{
    while (true)
    {
        StringHandle hFrameBufferName;
        bool bEmpty = false;

        spInstance->mFrameBufferToDestroyQueueLock.Lock();
        if (!spInstance->mhFrameBufferToDestroyQueue.IsEmpty())
        {
            hFrameBufferName = spInstance->mhFrameBufferToDestroyQueue.Front();
            spInstance->mhFrameBufferToDestroyQueue.Pop();
            bEmpty = false;
        }
        else
        {
            bEmpty = true;
        }
        spInstance->mFrameBufferToDestroyQueueLock.Unlock();

        if (bEmpty)
        {
            break;
        }

        auto fbit = mNameToFrameBufferMap.find(hFrameBufferName);
        if (fbit == mNameToFrameBufferMap.end())
        {
            HO_ASSERT(false, "There is no framebuffer has sush name.");
            continue;
        }

        const FrameBufferChain fbChain = fbit->second;

        for (const auto& hFrameBuffer : fbChain.hFrameBuffers)
        {
            spInstance->mpBackend->DestroyFrameBuffer(hFrameBuffer);
        }

        mNameToFrameBufferMap.erase(hFrameBufferName);
    }
}

void RenderingSystem::updateGpuMaterialTextureShader(ObjectHandle<RMaterialAsset> hMaterial)
{
    HO_ASSERT(hMaterial.IsValid(), "Invalid material asset.");

    if (!hMaterial->hRenderProxy.IsValid())
    {
        return;
    }

    GpuMaterial& gpuMat = *hMaterial->hRenderProxy;

    for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
    {
        if (hMaterial->hTextures[i].IsValid())
        {
            if (hMaterial->hTextures[i]->hRenderProxy.IsValid())
            {
                gpuMat.hGpuTextures[i] = hMaterial->hTextures[i]->hRenderProxy;
            }
        }
        else
        {
            gpuMat.hGpuTextures[i] = GpuTextureHandle::sNULL;
        }
    }

    for (int32_t i = 0; i < static_cast<int32_t>(eShaderStage::Last); ++i)
    {
        if (hMaterial->hShaders[i].IsValid())
        {
            if (hMaterial->hShaders[i]->hRenderProxy.IsValid())
            {
                gpuMat.hGpuShaders[i] = hMaterial->hShaders[i]->hRenderProxy;
            }
        }
        else
        {
            gpuMat.hGpuShaders[i] = GpuShaderHandle::sNULL;
        }
    }
}

// TODO: Currently, always upload all pending resources.
//       Need to apply time slicing method later.
void RenderingSystem::uploadPendingResources()
{
    mPendingStaticMeshQueueLock.Lock();
    int32_t pendingStaticMeshCount = mhPendingStaticMeshQueue.GetCount();
    mPendingStaticMeshQueueLock.Unlock();

    while (pendingStaticMeshCount > 0)
    {
        mPendingStaticMeshQueueLock.Lock();
        const GpuStaticMeshHandle hPending = mhPendingStaticMeshQueue.Front();
        mhPendingStaticMeshQueue.Pop();
        mPendingStaticMeshQueueLock.Unlock();

        switch (mpBackend->UploadStaticMesh(hPending))
        {
            case IRenderingBackend::eResourceTransferResult::Success:
            case IRenderingBackend::eResourceTransferResult::Failed:
                break;
            case IRenderingBackend::eResourceTransferResult::NeedsRetry:
                mPendingStaticMeshQueueLock.Lock();
                mhPendingStaticMeshQueue.Push(hPending);
                mPendingStaticMeshQueueLock.Unlock();
                break;
            default:
                HO_ASSERT(false, "Invalid transfer result.");
        }

        --pendingStaticMeshCount;
    }

    mPendingMaterialQueueLock.Lock();
    int32_t pendingMaterialCount = mhPendingMaterialQueue.GetCount();
    mPendingMaterialQueueLock.Unlock();

    while (pendingMaterialCount > 0)
    {
        mPendingMaterialQueueLock.Lock();
        const GpuMaterialHandle hPending = mhPendingMaterialQueue.Front();
        mhPendingMaterialQueue.Pop();
        mPendingMaterialQueueLock.Unlock();

        switch (mpBackend->UploadMaterial(hPending))
        {
            case IRenderingBackend::eResourceTransferResult::Success:
            case IRenderingBackend::eResourceTransferResult::Failed:
                break;
            case IRenderingBackend::eResourceTransferResult::NeedsRetry:
                mPendingMaterialQueueLock.Lock();
                mhPendingMaterialQueue.Push(hPending);
                mPendingMaterialQueueLock.Unlock();
                break;
            default:
                HO_ASSERT(false, "Invalid transfer result.");
        }

        --pendingMaterialCount;
    }

    mPendingTextureQueueLock.Lock();
    int32_t pendingTextureCount = mhPendingTextureQueue.GetCount();
    mPendingTextureQueueLock.Unlock();

    while (pendingTextureCount > 0)
    {
        mPendingTextureQueueLock.Lock();
        const GpuTextureHandle hPending = mhPendingTextureQueue.Front();
        mhPendingTextureQueue.Pop();
        mPendingTextureQueueLock.Unlock();

        switch (mpBackend->UploadTexture(hPending))
        {
            case IRenderingBackend::eResourceTransferResult::Success:
            case IRenderingBackend::eResourceTransferResult::Failed:
                break;
            case IRenderingBackend::eResourceTransferResult::NeedsRetry:
                mPendingTextureQueueLock.Lock();
                mhPendingTextureQueue.Push(hPending);
                mPendingTextureQueueLock.Unlock();
                break;
            default:
                HO_ASSERT(false, "Invalid transfer result.");
        }

        --pendingTextureCount;
    }

    mPendingShaderQueueLock.Lock();
    int32_t pendingShaderCount = mhPendingShaderQueue.GetCount();
    mPendingShaderQueueLock.Unlock();

    while (pendingShaderCount > 0)
    {
        mPendingShaderQueueLock.Lock();
        const GpuShaderHandle hPending = mhPendingShaderQueue.Front();
        mhPendingShaderQueue.Pop();
        mPendingShaderQueueLock.Unlock();

        switch (mpBackend->UploadShader(hPending))
        {
            case IRenderingBackend::eResourceTransferResult::Success:
            case IRenderingBackend::eResourceTransferResult::Failed:
                break;
            case IRenderingBackend::eResourceTransferResult::NeedsRetry:
                mPendingShaderQueueLock.Lock();
                mhPendingShaderQueue.Push(hPending);
                mPendingShaderQueueLock.Unlock();
                break;
            default:
                HO_ASSERT(false, "Invalid transfer result.");
        }

        --pendingShaderCount;
    }

    mPendingMorphTargetsQueueLock.Lock();
    int32_t pendingMorphTargetsCount = mhPendingMorphTargetsQueue.GetCount();
    mPendingMorphTargetsQueueLock.Unlock();

    while (pendingMorphTargetsCount > 0)
    {
        mPendingMorphTargetsQueueLock.Lock();
        const GpuMorphTargetsHandle hPending = mhPendingMorphTargetsQueue.Front();
        mhPendingMorphTargetsQueue.Pop();
        mPendingMorphTargetsQueueLock.Unlock();

        switch (mpBackend->UploadMorphTargets(hPending))
        {
            case IRenderingBackend::eResourceTransferResult::Success:
            case IRenderingBackend::eResourceTransferResult::Failed:
                break;
            case IRenderingBackend::eResourceTransferResult::NeedsRetry:
                mPendingMorphTargetsQueueLock.Lock();
                mhPendingMorphTargetsQueue.Push(hPending);
                mPendingMorphTargetsQueueLock.Unlock();
                break;
            default:
                HO_ASSERT(false, "Invalid transfer result.");
        }

        --pendingMorphTargetsCount;
    }
}

RenderingSystem* RenderingSystem::spInstance = nullptr;
;
} // namespace ho