#pragma once

#include "Config.h"
#include "Core/Memory/WeakTypedHandle.h"
#include "Core/String/StringHandle.h"
#include "Core/Templates/AtomicNumeric.h"
#include "Core/Templates/FixedArray.h"
#include "Core/Templates/FixedQueue.h"
#include "Core/Thread/SpinLock.h"
#include "RenderBackend/RenderResourceContext.h"
#include "RenderData/DrawCommand.h"
#include "RenderData/FrameBufferDesc.h"
#include "RenderPass/RenderPassDescs.h"
#include "RenderQueue.h"
#include "RenderStage/IRenderStage.h"
#include "RenderStage/RenderStageDescs.h"
#include "RenderSync.h"
#include "RenderingBuiltInAssets.h"
#include "Subsystems/Object/ObjectHandle.h"

struct ImDrawData;

namespace ho
{
enum class eRenderTargetBindingIndex;

struct RStaticMeshAsset;
struct RMaterialAsset;
struct RShaderAsset;
struct RTextureAsset;
struct RMorphTargetsAsset;

struct GpuStaticMesh;
using GpuStaticMeshHandle = WeakTypedHandle<GpuStaticMesh, TypedObjectPool>;
struct GpuMaterial;
using GpuMaterialHandle = WeakTypedHandle<GpuMaterial, TypedObjectPool>;
struct GpuTexture;
using GpuTextureHandle = WeakTypedHandle<GpuTexture, TypedObjectPool>;
struct GpuShader;
using GpuShaderHandle = WeakTypedHandle<GpuShader, TypedObjectPool>;
struct GpuMorphTargets;
using GpuMorphTargetsHandle = WeakTypedHandle<GpuMorphTargets, TypedObjectPool>;

struct InstanceData;
struct MorphControlData;

class IRenderPass;
class EnvironmentBakePass;
class ShadowMapBakePass;
class DirLightShadowMapRenderPass;
class PointLightShadowMapRenderPass;
class SpotLightShadowMapRenderPass;
class ForwardRenderPass;

class OpaqueRenderStage;
class MaskedRenderStage;
class InWorldDebugRenderStage;
class EnvironmentRenderStage;
class BlendedRenderStage;
class OutlineRenderStage;
class PostProcessingStage;
class WorldOverlayDebugRenderStage;

class Thread;
class Engine;
class IPlatformWindow;
class IRenderingBackend;

class RenderingSystem final
{
    friend Engine;

  public:
    static const int32_t sDirLightShadowMapSize = 4096;
    static const int32_t sPointLightShadowMapSize = 1024;
    static const int32_t sSpotLightShadowMapSize = 1024;

    static RenderingSystem& GetInstance()
    {
        HO_ASSERT(spInstance, "RenderingSystem is not created");
        return *spInstance;
    }

    // The rendering system automatically creates render targets for any format specified other than
    // eTextureFormat::None.
    bool CreateFramebuffer(const FrameBufferDesc& frameBufferDesc);

    bool DestroyFramebuffer(StringHandle hFrameBufferName);

    [[nodiscard]] void* GetRenderResultNativeHandle(StringHandle hFrameBufferName) const;

    [[nodiscard]] Vector2 GetFrameBufferSize(StringHandle hFrameBufferName) const;

    [[nodiscard]] bool HasFrameBuffer(StringHandle hFrameBufferName) const;

    bool BeginFrame();

    bool BeginEnvironmentBakePass(const EnvironmentBakePassDesc& desc);
    bool BeginShadowMapBakePass(const ShadowMapBakePassDesc& desc);
    bool BeginDirLightShadowMapRenderPass(const DirLightShadowMapRenderPassDesc& desc);
    bool BeginPointLightShadowMapRenderPass(const PointLightShadowMapRenderPassDesc& desc);
    bool BeginSpotLightShadowMapRenderPass(const SpotLightShadowMapRenderPassDesc& desc);
    bool BeginForwardRenderPass(const ForwardRenderPassDesc& desc);

    // Attach render stage on currently recording render pass.
    void AttachOpaqueRenderStage(const OpaqueRenderStageDesc& desc);
    void AttachMaskedRenderStage(const MaskedRenderStageDesc& desc);
    void AttachInWorldDebugRenderStage(const InWorldDebugRenderStageDesc& desc);
    void AttachEnvironmentRenderStage(const EnvironmentRenderStageDesc& desc);
    void AttachBlendedRenderStage(const BlendedRenderStageDesc& desc);
    void AttachOutlineRenderStage(const OutlineRenderStageDesc& desc);
    void AttachPostProcessingStage(const PostProcessingStageDesc& desc);
    void AttachWorldOverlayDebugRenderStage(const WorldOverlayDebugRenderStageDesc& desc);

    void EndFrame();

    void EndRenderPass();

    void BeginSubmitDrawCommands();

    bool SubmitDrawCommandsToStage(eRenderStageType stage,
                                   const DrawCommandDesc& commandDesc,
                                   int32_t instanceCount,
                                   const InstanceData* pInstanceData);

    bool SubmitDrawCommandWithMorphToStage(eRenderStageType stage,
                                           const DrawCommandDesc& commandDesc,
                                           GpuMorphTargetsHandle hMorphTargets,
                                           const MorphControlData& morphControlData,
                                           int32_t instanceCount,
                                           const InstanceData* pInstanceData);

    void EndSubmitDrawCommands();

    bool SubmitUIViewportData(const IPlatformWindow* pTargetWindow, const ImDrawData& srcDrawData);

    const RenderingBuiltInAssets& GetBuiltInAssets() const;

    // TODO: This functions make race condition currently.
    // It must be fix by remove Prepare~ function calls.
    bool UploadStaticMesh(ObjectHandle<RStaticMeshAsset> hStaticMesh, bool bPersistent = false);
    bool UploadMaterial(ObjectHandle<RMaterialAsset> hMaterial, bool bPersistent = false);
    bool UploadTexture(ObjectHandle<RTextureAsset> hTexture, bool bPersistent = false);
    bool UploadShader(ObjectHandle<RShaderAsset> hShader, bool bPersistent = false);
    bool UploadMorphTargets(ObjectHandle<RMorphTargetsAsset> hMorphTargets, bool bPersistent = false);

    // Uploads textures attached to the material and sets up GPU bindings.
    bool UploadMaterialTexture(ObjectHandle<RMaterialAsset> hMaterial,
                               eMaterialTextureUsage usage,
                               bool bPersistent = false);
    bool UploadMaterialTextures(ObjectHandle<RMaterialAsset> hMaterial, bool bPersistent = false);
    // Uploads shader assets attached to the material and sets up GPU bindings.
    bool UploadMaterialShaders(ObjectHandle<RMaterialAsset> hMaterial, bool bPersistent = false);

    template <typename TAssetHandle>
    bool IsUploaded(TAssetHandle hAsset) const;

    template <typename TAssetHandle>
    bool IsObsolete(TAssetHandle hAsset) const;

    template <typename TAssetHandle>
    void SetObsolete(TAssetHandle hAsset) const;

    bool IsVSyncEnabled() const;
    void SetVSync(bool bEnable);

  protected:
    static const int32_t sSwapChainSize = 2;
    static const int32_t sRenderPassPoolSize = 3; // Supposed each render queue use once per each render pass.

    static const int32_t sPendingFrameBufferQueueSize = 16;
    static const int32_t sPendingResourceQueueSize = 256;

    static const int32_t sInitialInstanceDataCount = 4096;
    static const int32_t sInitialDrawCommandCount = 4096;
    static const int32_t sInitialMorphControlDataCount = 256;

    struct FrameBufferChain
    {
        FrameBufferHandle hFrameBuffers[sSwapChainSize];
    };

    RenderingSystem(eGraphicsAPI api);
    ~RenderingSystem();

    static void createInstance(eGraphicsAPI api);
    static void deleteInstance();

    // 'init' called by engine.
    bool init();
    // 'run' function creates rendering thread.
    void run();
    static void runInternal(void* unused);
    // 'shutdown' called by engine and it will terminate 'run' function
    void shutdown();

    bool uploadBuiltInAssets();

    bool uploadResourceContext();

    // its called by enine on main thread.
    void swapRenderQueues(uint64_t currentFrame);

    bool trySwapRenderQueues(uint64_t currentFrame);

    void createPendingFrameBuffers();
    void destroyPendingFrameBuffers();

    void updateGpuMaterialTextureShader(ObjectHandle<RMaterialAsset> hMaterial);

    void uploadPendingResources();

    IRenderPass* mpRecordingPass = nullptr;
    FixedArray<EnvironmentBakePass> mEnvironmentBakePassPool;
    FixedArray<ShadowMapBakePass> mShadowMapBakePassPool;
    FixedArray<DirLightShadowMapRenderPass> mDirLightShadowMapRenderPassPool;
    FixedArray<PointLightShadowMapRenderPass> mPointLightShadowMapRenderPassPool;
    FixedArray<SpotLightShadowMapRenderPass> mSpotLightShadowMapRenderPassPool;
    FixedArray<ForwardRenderPass> mForwardRenderPassPool;

    std::unordered_map<StringHandle, FrameBufferChain> mNameToFrameBufferMap;

    int32_t mFrontRenderQueueIndex = 0;
    int32_t mBackRenderQueueIndex = 1;
    FixedArray<RenderQueue> mRenderQueues;

    FixedQueue<FrameBufferDesc> mFrameBufferToCreateQueue;
    SpinLock mFrameBufferToCreateQueueLock;
    FixedQueue<StringHandle> mhFrameBufferToDestroyQueue;
    SpinLock mFrameBufferToDestroyQueueLock;

    bool mbBuiltInAssetLoaded = false;
    RenderingBuiltInAssets mBuiltInAssets;

    RenderResourceContext mResourceContext;

    FixedQueue<GpuStaticMeshHandle> mhPendingStaticMeshQueue;
    SpinLock mPendingStaticMeshQueueLock;
    FixedQueue<GpuMaterialHandle> mhPendingMaterialQueue;
    SpinLock mPendingMaterialQueueLock;
    FixedQueue<GpuTextureHandle> mhPendingTextureQueue;
    SpinLock mPendingTextureQueueLock;
    FixedQueue<GpuShaderHandle> mhPendingShaderQueue;
    SpinLock mPendingShaderQueueLock;
    FixedQueue<GpuMorphTargetsHandle> mhPendingMorphTargetsQueue;
    SpinLock mPendingMorphTargetsQueueLock;

    bool mbRunning = true;
    uint64_t mCurrentFrame = 0;
    RenderSync mRenderSync;

    AtomicNumeric<bool> mbVSyncEnabled;

    std::unique_ptr<Thread> mpRenderThread;

    static RenderingSystem* spInstance;
    IRenderingBackend* mpBackend;
};

template <typename TAssetHandle>
bool RenderingSystem::IsUploaded(TAssetHandle hAsset) const
{
    HO_ASSERT(hAsset.IsValid(), "Asset is invalid");
    return hAsset->hRenderProxy.IsValid() && (hAsset->hRenderProxy->State != eGpuResourceState::PendingUpload);
}

template <typename TAssetHandle>
bool RenderingSystem::IsObsolete(TAssetHandle hAsset) const
{
    HO_ASSERT(hAsset.IsValid(), "Asset is invalid");
    return IsUploaded(hAsset) && (hAsset->hRenderProxy->State == eGpuResourceState::Obsolete);
}

template <typename TAssetHandle>
void RenderingSystem::SetObsolete(TAssetHandle hAsset) const
{
    HO_ASSERT(hAsset.IsValid(), "Asset is invalid");
    HO_ASSERT(IsUploaded(hAsset),
              "Asset is not uploaded yet. This function should be called after checking resource state through "
              "'IsUploaded' or 'IsObsolete'.");

    hAsset->hRenderProxy->State = eGpuResourceState::Obsolete;
}
} // namespace ho