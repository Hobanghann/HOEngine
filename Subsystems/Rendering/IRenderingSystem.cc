#include "IRenderingSystem.h"

#include <imgui.h>

#include "GL/RenderingSystemGL.h"
#include "Platforms/IPlatformApplication.h"
#include "Subsystems/Asset/Assets.h"

namespace ho
{
bool IRenderingSystem::EnqueueCreateFramebuffer(const FrameBufferDesc& frameBufferDesc)
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

bool IRenderingSystem::EnqueueDestroyFramebuffer(StringHandle hFrameBufferName)
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

bool IRenderingSystem::BeginRenderPass(RenderPassDesc& renderPassDesc)
{
    if (mCurrentRenderPassIndex != -1)
    {
        HO_ASSERT(false, "Render pass was already begun. You must call 'EndRenderPass' before beginning a new one.");
        return false;
    }

    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    if (queue.ActivePassCount >= sMaxRenderPassCount)
    {
        HO_ASSERT(false, "RenderPass count exceeded the maximum limit (sMaxRenderPassCount).");
        return false;
    }

    RenderPass& currentPass = queue.RenderPasses[queue.ActivePassCount];
    auto it = mNameToFrameBufferMap.find(renderPassDesc.hFrameBufferName);
    if (it == mNameToFrameBufferMap.end())
    {
        return false;
    }
    currentPass.pFrameBuffer = &it->second;
    for (int32_t i = 0; i < static_cast<int32_t>(eRenderTargetType::Last); ++i)
    {
        currentPass.bClearTarget[i] = renderPassDesc.bClearTarget[i];
    }
    currentPass.Viewport = renderPassDesc.Viewport;
    currentPass.WorldCameraPos = renderPassDesc.WorldCameraPos;
    currentPass.ViewMat = renderPassDesc.ViewMat;
    currentPass.ProjMat = renderPassDesc.ProjMat;

    HO_ASSERT(renderPassDesc.DirLightCount <= sMaxDirLightCount,
              "Directional light count exceeds the maximum limit. Excess lights will be truncated.");
    currentPass.Lights.DirLightCount = math::Min(renderPassDesc.DirLightCount, sMaxDirLightCount);
    for (int32_t i = 0; i < currentPass.Lights.DirLightCount; ++i)
    {
        currentPass.Lights.DirLights[i].Direction[0] = renderPassDesc.DirLights[i].Direction.X;
        currentPass.Lights.DirLights[i].Direction[1] = renderPassDesc.DirLights[i].Direction.Y;
        currentPass.Lights.DirLights[i].Direction[2] = renderPassDesc.DirLights[i].Direction.Z;
        currentPass.Lights.DirLights[i].Color[0] = renderPassDesc.DirLights[i].Color.R;
        currentPass.Lights.DirLights[i].Color[1] = renderPassDesc.DirLights[i].Color.G;
        currentPass.Lights.DirLights[i].Color[2] = renderPassDesc.DirLights[i].Color.B;
        currentPass.Lights.DirLights[i].Intensity = renderPassDesc.DirLights[i].Intensity;
    }

    HO_ASSERT(renderPassDesc.PointLightCount <= sMaxPointLightCount,
              "Point light count exceeds the maximum limit. Excess lights will be truncated.");
    currentPass.Lights.PointLightCount = math::Min(renderPassDesc.PointLightCount, sMaxPointLightCount);
    for (int32_t i = 0; i < currentPass.Lights.PointLightCount; ++i)
    {
        currentPass.Lights.PointLights[i].Position[0] = renderPassDesc.PointLights[i].Position.X;
        currentPass.Lights.PointLights[i].Position[1] = renderPassDesc.PointLights[i].Position.Y;
        currentPass.Lights.PointLights[i].Position[2] = renderPassDesc.PointLights[i].Position.Z;
        currentPass.Lights.PointLights[i].Color[0] = renderPassDesc.PointLights[i].Color.R;
        currentPass.Lights.PointLights[i].Color[1] = renderPassDesc.PointLights[i].Color.G;
        currentPass.Lights.PointLights[i].Color[2] = renderPassDesc.PointLights[i].Color.B;
        currentPass.Lights.PointLights[i].Intensity = renderPassDesc.PointLights[i].Intensity;
        currentPass.Lights.PointLights[i].Constant = renderPassDesc.PointLights[i].Constant;
        currentPass.Lights.PointLights[i].Linear = renderPassDesc.PointLights[i].Linear;
        currentPass.Lights.PointLights[i].Quadratic = renderPassDesc.PointLights[i].Quadratic;
    }

    HO_ASSERT(renderPassDesc.SpotLightCount <= sMaxSpotLightCount,
              "Spot light count exceeds the maximum limit. Excess lights will be truncated.");
    currentPass.Lights.SpotLightCount = math::Min(renderPassDesc.SpotLightCount, sMaxSpotLightCount);
    for (int32_t i = 0; i < currentPass.Lights.SpotLightCount; ++i)
    {
        currentPass.Lights.SpotLights[i].Position[0] = renderPassDesc.SpotLights[i].Position.X;
        currentPass.Lights.SpotLights[i].Position[1] = renderPassDesc.SpotLights[i].Position.Y;
        currentPass.Lights.SpotLights[i].Position[2] = renderPassDesc.SpotLights[i].Position.Z;
        currentPass.Lights.SpotLights[i].Direction[0] = renderPassDesc.SpotLights[i].Direction.X;
        currentPass.Lights.SpotLights[i].Direction[1] = renderPassDesc.SpotLights[i].Direction.Y;
        currentPass.Lights.SpotLights[i].Direction[2] = renderPassDesc.SpotLights[i].Direction.Z;
        currentPass.Lights.SpotLights[i].Color[0] = renderPassDesc.SpotLights[i].Color.R;
        currentPass.Lights.SpotLights[i].Color[1] = renderPassDesc.SpotLights[i].Color.G;
        currentPass.Lights.SpotLights[i].Color[2] = renderPassDesc.SpotLights[i].Color.B;
        currentPass.Lights.SpotLights[i].Intensity = renderPassDesc.SpotLights[i].Intensity;
        currentPass.Lights.SpotLights[i].InnerCutOff = renderPassDesc.SpotLights[i].InnerCutOff;
        currentPass.Lights.SpotLights[i].OuterCutOff = renderPassDesc.SpotLights[i].OuterCutOff;
        currentPass.Lights.SpotLights[i].Constant = renderPassDesc.SpotLights[i].Constant;
        currentPass.Lights.SpotLights[i].Linear = renderPassDesc.SpotLights[i].Linear;
        currentPass.Lights.SpotLights[i].Quadratic = renderPassDesc.SpotLights[i].Quadratic;
    }

    if (renderPassDesc.hSkyMap.IsValid())
    {
        currentPass.pSkyMap = renderPassDesc.hSkyMap.Get()->bPendingUpload ? nullptr : renderPassDesc.hSkyMap.Get();
    }

    if (renderPassDesc.hIrradianceMap.IsValid())
    {
        currentPass.pIrradianceMap =
            renderPassDesc.hIrradianceMap.Get()->bPendingUpload ? nullptr : renderPassDesc.hIrradianceMap.Get();
    }

    currentPass.OpaqueDrawCommandCount = 0;
    currentPass.MaskedDrawCommandCount = 0;
    currentPass.BlendedDrawCommandCount = 0;

    mCurrentRenderPassIndex = queue.ActivePassCount;
    ++queue.ActivePassCount;

    return true;
}

bool IRenderingSystem::SubmitDrawCommand(const DrawCommandDesc& commandDesc)
{
    HO_ASSERT(commandDesc.hGpuStaticMesh.IsValid(), "Static mesh is not uploaded at all.");
    HO_ASSERT(commandDesc.hGpuMaterial.IsValid(), "Material is not uploaded at all.");

    if (mCurrentRenderPassIndex == -1)
    {
        HO_ASSERT(false, "Render pass is not begun. You must call 'BeginRenderPass' before submit draw command.");
        return false;
    }

    if (commandDesc.hGpuStaticMesh.Get()->bPendingUpload || commandDesc.hGpuMaterial.Get()->bPendingUpload)
    {
        return false;
    }

    DrawCommand newCommand;
    newCommand.hGpuStaticMesh = commandDesc.hGpuStaticMesh;
    newCommand.hGpuMaterial = commandDesc.hGpuMaterial;
    newCommand.VertexOffset = commandDesc.VertexOffset;
    newCommand.IndexOffset = commandDesc.IndexOffset;
    newCommand.IndexCount = commandDesc.IndexCount;
    newCommand.WorldMat = commandDesc.WorldMat;
    newCommand.SqrdDistanceFromCamera =
        (mRenderQueues[mFrontRenderQueueIndex].RenderPasses[mCurrentRenderPassIndex].WorldCameraPos -
         Vector3(newCommand.WorldMat.GetCol3()))
            .SqrdMagnitude();

    RenderPass& pass = mRenderQueues[mFrontRenderQueueIndex].RenderPasses[mCurrentRenderPassIndex];
    switch (commandDesc.AlphaMode)
    {
        case eMaterialAlphaMode::Opaque:
        {
            if (pass.OpaqueDrawCommandCount < sMaxOpaqueDrawCommandCount)
            {
                pass.OpaqueDrawCommands[pass.OpaqueDrawCommandCount] = newCommand;
                ++pass.OpaqueDrawCommandCount;
            }
            else
            {
                HO_ASSERT(false,
                          "Opaque draw command count exceeds the maximum limit. Excess commands will be truncated.");
                return false;
            }
        }
        break;
        case eMaterialAlphaMode::Mask:
        {
            if (pass.MaskedDrawCommandCount < sMaxMaskedDrawCommandCount)
            {
                pass.MaskedDrawCommands[pass.MaskedDrawCommandCount] = newCommand;
                ++pass.MaskedDrawCommandCount;
            }
            else
            {
                HO_ASSERT(false,
                          "Masked draw command count exceeds the maximum limit. Excess commands will be truncated.");
                return false;
            }
        }
        break;
        case eMaterialAlphaMode::Blend:
        {
            if (pass.BlendedDrawCommandCount < sMaxBlendedDrawCommandCount)
            {
                pass.BlendedDrawCommands[pass.BlendedDrawCommandCount] = newCommand;
                ++pass.BlendedDrawCommandCount;
            }
            else
            {
                HO_ASSERT(false,
                          "Blended draw command count exceeds the maximum limit. Excess commands will be truncated.");
                return false;
            }
        }
        break;
        default:
            HO_ASSERT(false, "Invalid alpha mode.");
            return false;
    }
    return true;
}

bool IRenderingSystem::SubmitUIViewportData(const IPlatformWindow* pTargetWindow, const ImDrawData& srcDrawData)
{
    if (!srcDrawData.Valid)
    {
        return false;
    }

    RenderQueue& queue = mRenderQueues[mFrontRenderQueueIndex];

    if (queue.ActiveViewportCount >= sMaxViewportCount)
    {
        HO_ASSERT(false, "UI viewport data count exceeds the maximum limit. Excess data will be truncated.");
        return false;
    }

    UIViewportData& dstData = queue.UIViewports[queue.ActiveViewportCount];

    dstData.pTargetWindow = pTargetWindow;

    dstData.pDrawData->Valid = srcDrawData.Valid;
    dstData.pDrawData->CmdListsCount = srcDrawData.CmdListsCount;
    dstData.pDrawData->TotalIdxCount = srcDrawData.TotalIdxCount;
    dstData.pDrawData->TotalVtxCount = srcDrawData.TotalVtxCount;

    // deep copy ImDrawData::CmdLists
    const int32_t oldSize = dstData.pDrawData->CmdLists.Size;
    dstData.pDrawData->CmdLists.resize(srcDrawData.CmdLists.Size);

    for (int32_t i = oldSize; i < dstData.pDrawData->CmdLists.Size; ++i)
    {
        dstData.pDrawData->CmdLists[i] = nullptr;
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

    ++queue.ActiveViewportCount;
    return true;
}

bool IRenderingSystem::EndRenderPass()
{
    if (mCurrentRenderPassIndex == -1)
    {
        HO_ASSERT(false, "Render pass was already ended. You must call 'BeginRenderPass' before ending.");
        return false;
    }
    mCurrentRenderPassIndex = -1;
    return true;
}

GpuStaticMeshHandle IRenderingSystem::EnqueueUploadStaticMesh(StaticMeshHandle hStaticMesh, bool bPersistent)
{
    if (mhPendingStaticMeshQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return GpuStaticMeshHandle::sNULL;
    }
    GpuStaticMeshHandle hGpuStaticMesh = prepareUploadStaticMesh(hStaticMesh, bPersistent);
    if (hGpuStaticMesh.IsNULL())
    {
        HO_ASSERT(false, "Preparing upload was failed. Asset might be invalid.");
        return GpuStaticMeshHandle::sNULL;
    }

    mPendingStaticMeshQueueLock.Lock();
    mhPendingStaticMeshQueue.Push(hGpuStaticMesh);
    mPendingStaticMeshQueueLock.Unlock();
    return hGpuStaticMesh;
}

GpuMaterialHandle IRenderingSystem::EnqueueUploadMaterial(MaterialHandle hMaterial, bool bPersistent)
{
    if (mhPendingMaterialQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return GpuMaterialHandle::sNULL;
    }
    GpuMaterialHandle hGpuMaterial = prepareUploadMaterial(hMaterial, bPersistent);
    if (hGpuMaterial.IsNULL())
    {
        HO_ASSERT(false, "Preparing upload was failed. Asset might be invalid.");
        return GpuMaterialHandle::sNULL;
    }

    mPendingMaterialQueueLock.Lock();
    mhPendingMaterialQueue.Push(hGpuMaterial);
    mPendingMaterialQueueLock.Unlock();
    return hGpuMaterial;
}

GpuTextureHandle IRenderingSystem::EnqueueUploadTexture(TextureHandle hTexture, bool bPersistent)
{
    if (mhPendingTextureQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return GpuTextureHandle::sNULL;
    }
    GpuTextureHandle hGpuTexture = prepareUploadTexture(hTexture, bPersistent);
    if (hGpuTexture.IsNULL())
    {
        HO_ASSERT(false, "Preparing upload was failed. Asset might be invalid.");
        return GpuTextureHandle::sNULL;
    }

    mPendingTextureQueueLock.Lock();
    mhPendingTextureQueue.Push(hGpuTexture);
    mPendingTextureQueueLock.Unlock();
    return hGpuTexture;
}

GpuShaderHandle IRenderingSystem::EnqueueUploadShader(ShaderHandle hShader, bool bPersistent)
{
    if (mhPendingShaderQueue.IsFull())
    {
        HO_ASSERT(false, "Pending queue is full.");
        return GpuShaderHandle::sNULL;
    }
    GpuShaderHandle hGpuShader = prepareUploadShader(hShader, bPersistent);
    if (hGpuShader.IsNULL())
    {
        HO_ASSERT(false, "Preparing upload was failed. Asset might be invalid.");
        return GpuShaderHandle::sNULL;
    }

    mPendingShaderQueueLock.Lock();
    mhPendingShaderQueue.Push(hGpuShader);
    mPendingShaderQueueLock.Unlock();
    return hGpuShader;
}

IRenderingSystem::IRenderingSystem()
  : mbVSyncEnabled(false)
{
    HO_ASSERT(!spInstance, "Rendering system muse be created by 'createInstance' function.");
}

void IRenderingSystem::createInstance(eGraphicsAPI api)
{
    if (spInstance)
    {
        HO_ASSERT(false, "Rendering system is already created.");
    }
    else
    {
        switch (api)
        {
            case GL:
                spInstance = new RenderingSystemGL();
                break;
            default:
                HO_ASSERT(false, "Unsupported API.");
        }
    }
    spInstance->mFrameBufferToCreateQueue = FixedQueue<FrameBufferDesc>(sPendingFrameBufferQueueSize);
    spInstance->mhFrameBufferToDestroyQueue = FixedQueue<StringHandle>(sPendingFrameBufferQueueSize);
    spInstance->mhPendingStaticMeshQueue = FixedQueue<GpuStaticMeshHandle>(sPendingResourceQueueSize);
    spInstance->mhPendingMaterialQueue = FixedQueue<GpuMaterialHandle>(sPendingResourceQueueSize);
    spInstance->mhPendingTextureQueue = FixedQueue<GpuTextureHandle>(sPendingResourceQueueSize);
    spInstance->mhPendingShaderQueue = FixedQueue<GpuShaderHandle>(sPendingResourceQueueSize);
}

void IRenderingSystem::deleteInstance()
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

void IRenderingSystem::run()
{
    HO_ASSERT(spInstance, "Rendering system is not created.");

    // This deactivation is needed for render thread to activate context.
    IPlatformApplication::GetInstance().GetMainWindow()->DeactivateContext();
    mpRenderThread = std::make_unique<Thread>(reinterpret_cast<Thread::Entry>(runInternal));
    mpRenderThread->SetName("Render Thread");
}

void IRenderingSystem::runInternal(void* unused)
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

        spInstance->createPendingFrameBuffers();

        const uint64_t currentFrame = spInstance->mRenderQueues[spInstance->mBackRenderQueueIndex].FrameCount;

        spInstance->uploadPendingResources(currentFrame);

        spInstance->executeRenderQueue();

        spInstance->evictStaleResources(currentFrame, 1000);

        spInstance->flush(); // TODO: CPU might do something while waiting GPU completes own jobs.

        spInstance->renderUI();

        spInstance->destroyPendingFrameBuffers();

        spInstance->mRenderSync.NotifyRenderDone();
    }

    spInstance->releaseAllResources();

    IPlatformApplication::GetInstance().GetMainWindow()->DeactivateContext();
}

void IRenderingSystem::shutdown()
{
    mbRunning = false;

    mRenderSync.NotifyEngineSwapComplete();

    mpRenderThread->Join();
}

void IRenderingSystem::executeRenderQueue()
{
    prepareExecuteRenderQueue();

    RenderQueue& queue = mRenderQueues[mBackRenderQueueIndex];

    for (int32_t qi = 0; qi < queue.ActivePassCount; ++qi)
    {
        RenderPass& pass = queue.RenderPasses[qi];

        sortDrawCommands(pass);

        prepareExecuteRenderPass(pass);

        if (pass.OpaqueDrawCommandCount > 0)
        {
            prepareExecuteOpaqueDrawCommands();

            GpuMaterialHandle hPrevRenderedMaterial = GpuMaterialHandle::sNULL;
            for (int32_t ci = 0; ci < pass.OpaqueDrawCommandCount; ++ci)
            {
                if (hPrevRenderedMaterial != pass.OpaqueDrawCommands[ci].hGpuMaterial)
                {
                    applyMaterial(pass.OpaqueDrawCommands[ci].hGpuMaterial);
                    hPrevRenderedMaterial = pass.OpaqueDrawCommands[ci].hGpuMaterial;
                }
                executeDrawCommand(pass.OpaqueDrawCommands[ci]);
                updateDrawCommandResourceUsageStates(pass.OpaqueDrawCommands[ci], queue.FrameCount);
            }
        }

        if (pass.MaskedDrawCommandCount > 0)
        {
            prepareExecuteMaskedDrawCommands();

            GpuMaterialHandle hPrevRenderedMaterial = GpuMaterialHandle::sNULL;
            for (int32_t ci = 0; ci < pass.MaskedDrawCommandCount; ++ci)
            {
                if (hPrevRenderedMaterial != pass.MaskedDrawCommands[ci].hGpuMaterial)
                {
                    applyMaterial(pass.MaskedDrawCommands[ci].hGpuMaterial);
                    hPrevRenderedMaterial = pass.MaskedDrawCommands[ci].hGpuMaterial;
                }
                executeDrawCommand(pass.MaskedDrawCommands[ci]);
                updateDrawCommandResourceUsageStates(pass.MaskedDrawCommands[ci], queue.FrameCount);
            }
        }

        if (pass.pSkyMap)
        {
            executeSkyMapPass(pass.pSkyMap);
        }

        executeInWorldDebugDrawPass();

        if (pass.BlendedDrawCommandCount > 0)
        {
            prepareExecuteBlendedDrawCommands();

            GpuMaterialHandle hPrevRenderedMaterial = GpuMaterialHandle::sNULL;
            for (int32_t ci = 0; ci < pass.BlendedDrawCommandCount; ++ci)
            {
                if (hPrevRenderedMaterial != pass.BlendedDrawCommands[ci].hGpuMaterial)
                {
                    applyMaterial(pass.BlendedDrawCommands[ci].hGpuMaterial);
                    hPrevRenderedMaterial = pass.BlendedDrawCommands[ci].hGpuMaterial;
                }
                executeDrawCommand(pass.BlendedDrawCommands[ci]);
                updateDrawCommandResourceUsageStates(pass.BlendedDrawCommands[ci], queue.FrameCount);
            }
        }

        executeOverlayDebugDrawPass();

        resolveFrameBuffer(*pass.pFrameBuffer);

        finishExecuteRenderPass(pass);

        updateRenderPassResourceUsageStates(pass, queue.FrameCount);
    }

    finishExecuteRenderQueue();
}

void IRenderingSystem::updateDrawCommandResourceUsageStates(DrawCommand& command, uint64_t currentFrame)
{
    command.hGpuStaticMesh.Get()->LastUsedFrame = currentFrame;

    if (command.hGpuMaterial.Get()->LastUsedFrame < currentFrame)
    {
        command.hGpuMaterial.Get()->LastUsedFrame = currentFrame;
        for (auto& hGpuTexture : command.hGpuMaterial.Get()->hGpuTextures)
        {
            if (hGpuTexture.IsValid())
            {
                hGpuTexture.Get()->LastUsedFrame = currentFrame;
            }
        }
        for (auto& hGpuShader : command.hGpuMaterial.Get()->hGpuShaders)
        {
            if (hGpuShader.IsValid())
            {
                hGpuShader.Get()->LastUsedFrame = currentFrame;
            }
        }
    }
}

void IRenderingSystem::updateRenderPassResourceUsageStates(RenderPass& pass, uint64_t currentFrame)
{
    if (pass.pSkyMap)
    {
        pass.pSkyMap->LastUsedFrame = currentFrame;
    }

    if (pass.pIrradianceMap)
    {
        pass.pIrradianceMap->LastUsedFrame = currentFrame;
    }
}

void IRenderingSystem::sortDrawCommands(RenderPass& pass)
{
    std::sort(pass.OpaqueDrawCommands,
              pass.OpaqueDrawCommands + pass.OpaqueDrawCommandCount,
              [](const DrawCommand& a, const DrawCommand& b)
              {
                  if (a.hGpuMaterial != b.hGpuMaterial)
                  {
                      return a.hGpuMaterial.GetIndex() < b.hGpuMaterial.GetIndex();
                  }
                  return a.SqrdDistanceFromCamera < b.SqrdDistanceFromCamera;
              });

    std::sort(pass.MaskedDrawCommands,
              pass.MaskedDrawCommands + pass.MaskedDrawCommandCount,
              [](const DrawCommand& a, const DrawCommand& b)
              {
                  if (a.hGpuMaterial != b.hGpuMaterial)
                  {
                      return a.hGpuMaterial.GetIndex() < b.hGpuMaterial.GetIndex();
                  }
                  return a.SqrdDistanceFromCamera < b.SqrdDistanceFromCamera;
              });

    std::sort(pass.BlendedDrawCommands,
              pass.BlendedDrawCommands + pass.BlendedDrawCommandCount,
              [](const DrawCommand& a, const DrawCommand& b)
              {
                  if (a.SqrdDistanceFromCamera != b.SqrdDistanceFromCamera)
                  {
                      return a.SqrdDistanceFromCamera > b.SqrdDistanceFromCamera;
                  }
                  return a.hGpuMaterial.GetIndex() < b.hGpuMaterial.GetIndex();
              });
}

void IRenderingSystem::swapRenderQueues(uint64_t currentFrame)
{
    mRenderSync.WaitUntilRenderDone();

    mFrontRenderQueueIndex = (mFrontRenderQueueIndex + 1) % 2;
    mRenderQueues[mFrontRenderQueueIndex].ActivePassCount = 0;
    mRenderQueues[mFrontRenderQueueIndex].ActiveViewportCount = 0;
    mCurrentRenderPassIndex = -1;

    mBackRenderQueueIndex = (mBackRenderQueueIndex + 1) % 2;
    mRenderQueues[mBackRenderQueueIndex].FrameCount = currentFrame;

    mRenderSync.NotifyEngineSwapComplete();
}

bool IRenderingSystem::trySwapRenderQueues(uint64_t currentFrame)
{
    if (!mRenderSync.IsRenderDone())
    {
        return false;
    }

    swapRenderQueues(currentFrame);
    return true;
}

void IRenderingSystem::createPendingFrameBuffers()
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

        spInstance->createFrameBuffer(desc);
    }
}

void IRenderingSystem::destroyPendingFrameBuffers()
{
    while (true)
    {
        StringHandle hFrameBuffer;
        bool bEmpty = false;

        spInstance->mFrameBufferToDestroyQueueLock.Lock();
        if (!spInstance->mhFrameBufferToDestroyQueue.IsEmpty())
        {
            hFrameBuffer = spInstance->mhFrameBufferToDestroyQueue.Front();
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

        spInstance->destroyFrameBuffer(hFrameBuffer);
    }
}

// TODO: Currently, always upload all pending resources.
//       Need to apply time slicing method later.
void IRenderingSystem::uploadPendingResources(uint64_t currentFrame)
{
    mPendingStaticMeshQueueLock.Lock();
    int32_t pendingStaticMeshCount = static_cast<int32_t>(mhPendingStaticMeshQueue.GetCount());
    mPendingStaticMeshQueueLock.Unlock();

    while (pendingStaticMeshCount > 0)
    {
        mPendingStaticMeshQueueLock.Lock();
        const GpuStaticMeshHandle hPending = mhPendingStaticMeshQueue.Front();
        mhPendingStaticMeshQueue.Pop();
        mPendingStaticMeshQueueLock.Unlock();

        switch (uploadStaticMesh(hPending, currentFrame))
        {
            case eResourceTransferResult::Success:
            case eResourceTransferResult::Failed:
                break;
            case eResourceTransferResult::NeedsRetry:
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
    int32_t pendingMaterialCount = static_cast<int32_t>(mhPendingMaterialQueue.GetCount());
    mPendingMaterialQueueLock.Unlock();

    while (pendingMaterialCount > 0)
    {
        mPendingMaterialQueueLock.Lock();
        const GpuMaterialHandle hPending = mhPendingMaterialQueue.Front();
        mhPendingMaterialQueue.Pop();
        mPendingMaterialQueueLock.Unlock();

        switch (uploadMaterial(hPending, currentFrame))
        {
            case eResourceTransferResult::Success:
            case eResourceTransferResult::Failed:
                break;
            case eResourceTransferResult::NeedsRetry:
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
    int32_t pendingTextureCount = static_cast<int32_t>(mhPendingTextureQueue.GetCount());
    mPendingTextureQueueLock.Unlock();

    while (pendingTextureCount > 0)
    {
        mPendingTextureQueueLock.Lock();
        const GpuTextureHandle hPending = mhPendingTextureQueue.Front();
        mhPendingTextureQueue.Pop();
        mPendingTextureQueueLock.Unlock();

        switch (uploadTexture(hPending, currentFrame))
        {
            case eResourceTransferResult::Success:
            case eResourceTransferResult::Failed:
                break;
            case eResourceTransferResult::NeedsRetry:
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
    int32_t pendingShaderCount = static_cast<int32_t>(mhPendingShaderQueue.GetCount());
    mPendingShaderQueueLock.Unlock();

    while (pendingShaderCount > 0)
    {
        mPendingShaderQueueLock.Lock();
        const GpuShaderHandle hPending = mhPendingShaderQueue.Front();
        mhPendingShaderQueue.Pop();
        mPendingShaderQueueLock.Unlock();

        switch (uploadShader(hPending, currentFrame))
        {
            case eResourceTransferResult::Success:
            case eResourceTransferResult::Failed:
                break;
            case eResourceTransferResult::NeedsRetry:
                mPendingShaderQueueLock.Lock();
                mhPendingShaderQueue.Push(hPending);
                mPendingShaderQueueLock.Unlock();
                break;
            default:
                HO_ASSERT(false, "Invalid transfer result.");
        }

        --pendingShaderCount;
    }
}

// TODO: Check cache hit rate 1 loop way vs 4 loop way later.
int32_t IRenderingSystem::evictStaleResources(uint64_t currentFrame, uint64_t frameThreshold)
{
    int32_t evictedCount = 0;

    for (auto& staticMesh : mGpuStaticMeshPool.GetMutableRawPool())
    {
        if (staticMesh.has_value())
        {
            if (!staticMesh.value().bPersistent && !staticMesh.value().bPendingUpload &&
                (currentFrame - staticMesh.value().LastUsedFrame) > frameThreshold)
            {
                unloadStaticMesh(staticMesh.value().hThis);
                ++evictedCount;
            }
        }
    }
    for (auto& material : mGpuMaterialPool.GetMutableRawPool())
    {
        if (material.has_value())
        {
            if (!material.value().bPersistent && !material.value().bPendingUpload &&
                (currentFrame - material.value().LastUsedFrame) > frameThreshold)
            {
                unloadMaterial(material.value().hThis);
                ++evictedCount;
            }
        }
    }
    for (auto& texture : mGpuTexturePool.GetMutableRawPool())
    {
        if (texture.has_value())
        {
            if (!texture.value().bPersistent && !texture.value().bPendingUpload &&
                (currentFrame - texture.value().LastUsedFrame) > frameThreshold)
            {
                unloadTexture(texture.value().hThis);
                ++evictedCount;
            }
        }
    }
    for (auto& shader : mGpuShaderPool.GetMutableRawPool())
    {
        if (shader.has_value())
        {
            if (!shader.value().bPersistent && !shader.value().bPendingUpload &&
                (currentFrame - shader.value().LastUsedFrame) > frameThreshold)
            {
                unloadShader(shader.value().hThis);
                ++evictedCount;
            }
        }
    }
    return evictedCount;
}

GpuStaticMeshHandle IRenderingSystem::prepareUploadStaticMesh(StaticMeshHandle hStaticMesh, bool bPersistent)
{
    if (hStaticMesh.IsValid())
    {
        const uint32_t idx = mGpuStaticMeshPool.Add(GpuStaticMesh());
        GpuStaticMeshHandle hNewGpuStaticMesh = GpuStaticMeshHandle(&mGpuStaticMeshPool, idx);
        hStaticMesh.Get()->hRenderProxy = hNewGpuStaticMesh;
        hNewGpuStaticMesh.Get()->hOrigin = hStaticMesh;
        hNewGpuStaticMesh.Get()->LastUsedFrame = 0;
        hNewGpuStaticMesh.Get()->hThis = hNewGpuStaticMesh;
        hNewGpuStaticMesh.Get()->bPersistent = bPersistent;
        hNewGpuStaticMesh.Get()->bPendingUpload = true;
        return hNewGpuStaticMesh;
    }
    else
    {
        HO_ASSERT(false, "Original asset is invalid.");
        return GpuStaticMeshHandle::sNULL;
    }
}

GpuMaterialHandle IRenderingSystem::prepareUploadMaterial(MaterialHandle hMaterial, bool bPersistent)
{
    if (hMaterial.IsValid())
    {
        const uint32_t idx = mGpuMaterialPool.Add(GpuMaterial());
        GpuMaterialHandle hNewGpuMaterial = GpuMaterialHandle(&mGpuMaterialPool, idx);
        hMaterial.Get()->hRenderProxy = hNewGpuMaterial;

        hNewGpuMaterial.Get()->PipelineState.AlphaMode = hMaterial.Get()->PipelineState.AlphaMode;
        hNewGpuMaterial.Get()->PipelineState.AlphaBlendMode = hMaterial.Get()->PipelineState.AlphaBlendMode;
        hNewGpuMaterial.Get()->PipelineState.bWireframe = hMaterial.Get()->PipelineState.bWireframe;
        hNewGpuMaterial.Get()->PipelineState.bBackfaceCulling = hMaterial.Get()->PipelineState.bBackfaceCulling;

        hNewGpuMaterial.Get()->Type = hMaterial.Get()->Type;

        for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
        {
            if (hMaterial.Get()->hTextures[i].IsValid())
            {
                hNewGpuMaterial.Get()->hGpuTextures[i] =
                    prepareUploadTexture(hMaterial.Get()->hTextures[i], bPersistent);
                if (hNewGpuMaterial.Get()->hGpuTextures[i].IsValid())
                {
                    mPendingTextureQueueLock.Lock();
                    mhPendingTextureQueue.Push(hNewGpuMaterial.Get()->hGpuTextures[i]);
                    mPendingTextureQueueLock.Unlock();
                }
                else
                {
                    hNewGpuMaterial.Get()->hGpuTextures[i] = GpuTextureHandle::sNULL;
                }
            }
        }

        for (int32_t i = 0; i < static_cast<int32_t>(eShaderStage::Last); ++i)
        {
            if (hMaterial.Get()->hShaders[i].IsValid())
            {
                hNewGpuMaterial.Get()->hGpuShaders[i] = prepareUploadShader(hMaterial.Get()->hShaders[i], bPersistent);
                if (hNewGpuMaterial.Get()->hGpuShaders[i].IsValid())
                {
                    mPendingShaderQueueLock.Lock();
                    mhPendingShaderQueue.Push(hNewGpuMaterial.Get()->hGpuShaders[i]);
                    mPendingShaderQueueLock.Unlock();
                }
                else
                {
                    hNewGpuMaterial.Get()->hGpuShaders[i] = GpuShaderHandle::sNULL;
                }
            }
        }

        hNewGpuMaterial.Get()->hOrigin = hMaterial;
        hNewGpuMaterial.Get()->LastUsedFrame = 0;
        hNewGpuMaterial.Get()->hThis = hNewGpuMaterial;
        hNewGpuMaterial.Get()->bPersistent = bPersistent;
        hNewGpuMaterial.Get()->bPendingUpload = true;
        return hNewGpuMaterial;
    }
    else
    {
        HO_ASSERT(false, "Original asset is invalid.");
        return GpuMaterialHandle::sNULL;
    }
}

GpuTextureHandle IRenderingSystem::prepareUploadTexture(TextureHandle hTexture, bool bPersistent)
{
    if (hTexture.IsValid() || (hTexture == TextureHandle::sNULL && bPersistent))
    {
        const uint32_t idx = mGpuTexturePool.Add(GpuTexture());
        GpuTextureHandle hNewGpuTexture = GpuTextureHandle(&mGpuTexturePool, idx);
        // when texture is from asset.
        if (hTexture.IsValid())
        {
            hTexture.Get()->hRenderProxy = hNewGpuTexture;
        }

        hNewGpuTexture.Get()->hOrigin = hTexture;
        hNewGpuTexture.Get()->LastUsedFrame = 0;
        hNewGpuTexture.Get()->hThis = hNewGpuTexture;
        hNewGpuTexture.Get()->bPersistent = bPersistent;
        hNewGpuTexture.Get()->bPendingUpload = hTexture.IsValid();
        return hNewGpuTexture;
    }
    else
    {
        HO_ASSERT(false, "Original asset is invalid or mismatched with render target creation arguments.");
        return GpuTextureHandle::sNULL;
    }
}

GpuShaderHandle IRenderingSystem::prepareUploadShader(ShaderHandle hShader, bool bPersistent)
{
    if (hShader.IsValid())
    {
        const uint32_t idx = mGpuShaderPool.Add(GpuShader());
        GpuShaderHandle hNewGpuShader = GpuShaderHandle(&mGpuShaderPool, idx);
        hShader.Get()->hRenderProxy = hNewGpuShader;

        hNewGpuShader.Get()->hOrigin = hShader;
        hNewGpuShader.Get()->LastUsedFrame = 0;
        hNewGpuShader.Get()->hThis = hNewGpuShader;
        hNewGpuShader.Get()->bPersistent = bPersistent;
        hNewGpuShader.Get()->bPendingUpload = true;
        return hNewGpuShader;
    }
    else
    {
        HO_ASSERT(false, "Original asset is invalid.");
        return GpuShaderHandle::sNULL;
    }
}

void IRenderingSystem::finishUnloadStaticMesh(GpuStaticMeshHandle hUnloadedGpuStaticMesh)
{
    if (hUnloadedGpuStaticMesh.IsValid())
    {
        mGpuStaticMeshPool.Remove(hUnloadedGpuStaticMesh.GetIndex());
    }
}

void IRenderingSystem::finishUnloadMaterial(GpuMaterialHandle hUnloadedGpuMaterial)
{
    if (hUnloadedGpuMaterial.IsValid())
    { // TODO: needs to handle bound textures, shaders on material.
        mGpuMaterialPool.Remove(hUnloadedGpuMaterial.GetIndex());
    }
}

void IRenderingSystem::finishUnloadTexture(GpuTextureHandle hUnloadedGpuTexture)
{
    if (hUnloadedGpuTexture.IsValid())
    {
        mGpuTexturePool.Remove(hUnloadedGpuTexture.GetIndex());
    }
}

void IRenderingSystem::finishUnloadShader(GpuShaderHandle hUnloadedGpuShader)
{
    if (hUnloadedGpuShader.IsValid())
    {
        mGpuShaderPool.Remove(hUnloadedGpuShader.GetIndex());
    }
}

bool IRenderingSystem::validateFrameBufferDesc(const FrameBufferDesc& desc)
{
    if (desc.Width == 0 || desc.Height == 0 || desc.SampleCount == 0)
    {
        return false;
    }

    int32_t usedTargetCount = 0;
    for (int32_t i = 0; i < static_cast<int32_t>(eRenderTargetType::Last); ++i)
    {
        if (i != static_cast<int32_t>(eRenderTargetType::Depth))
        {
            ++usedTargetCount;
        }
    }
    if (usedTargetCount == 0 &&
        desc.TargetDesc[static_cast<int32_t>(eRenderTargetType::Depth)].Format != eRenderTargetFormat::None)
    {
        return false;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(eRenderTargetType::Last); ++i)
    {
        if (i != static_cast<int32_t>(eRenderTargetType::Depth))
        {
            const eRenderTargetFormat format = desc.TargetDesc[i].Format;
            if (format == eRenderTargetFormat::None || format == eRenderTargetFormat::D24_UNORM_S8_UINT ||
                format == eRenderTargetFormat::R32_FLOAT)
            {
                return false;
            }
        }
    }

    if (desc.TargetDesc[static_cast<int32_t>(eRenderTargetType::Depth)].Format != eRenderTargetFormat::None)
    {
        const eRenderTargetFormat format = desc.TargetDesc[static_cast<int32_t>(eRenderTargetType::Depth)].Format;

        if (format != eRenderTargetFormat::D24_UNORM_S8_UINT && format != eRenderTargetFormat::R16G16_FLOAT)
        {
            return false;
        }
    }

    return true;
}

IRenderingSystem* IRenderingSystem::spInstance = nullptr;

IRenderingSystem::UIViewportData::UIViewportData()
  : pTargetWindow(nullptr)
  , pDrawData(std::make_unique<ImDrawData>())
{
}

IRenderingSystem::UIViewportData::~UIViewportData() = default;
IRenderingSystem::UIViewportData::UIViewportData(UIViewportData&&) noexcept = default;
IRenderingSystem::UIViewportData& IRenderingSystem::UIViewportData::operator=(UIViewportData&&) noexcept = default;
} // namespace ho