#pragma once

#include "Config.h"
#include "Core/Math/Color128.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector3.h"
#include "Core/Templates/FixedArray.h"
#include "Core/Templates/FixedQueue.h"
#include "Core/Templates/GlobalPoolIndex.h"
#include "Core/Templates/ObjectPool.h"
#include "Core/Thread/SpinLock.h"
#include "GpuResources.h"
#include "RenderSync.h"

// =================================================================================
//  Rendering System Overview
// =================================================================================
//
// [ Architecture ]
// - Uses a Double-Buffering strategy with a Front and a Back render queue.
// - Each render queue contains a list of RenderPasses.
// - A RenderPass stores everything needed for drawing:
//   - WHERE to draw : Frame buffer target
//   - HOW to look   : Camera position, View & Proj matrices
//   - LIGHTS        : Light source data (Directional lights, etc.)
//   - WHAT to draw  : A collection of DrawCommands (Opaque / Transparent)
//
// [ Frame Workflow ]
// All draw calls flow in the following order per frame:
//
//   BeginRenderPass() -> SubmitDrawCommand() -> EndRenderPass()
//   -> executeRenderQueue() -> renderUI() -> swapRenderQueues()
//
// =================================================================================

struct ImDrawData;

namespace ho
{
struct StaticMeshAsset;
using StaticMeshHandle = WeakLocalPoolIndex<StaticMeshAsset, ObjectPool>;
struct MaterialAsset;
using MaterialHandle = WeakLocalPoolIndex<MaterialAsset, ObjectPool>;
struct ShaderAsset;
using ShaderHandle = WeakLocalPoolIndex<ShaderAsset, ObjectPool>;
struct TextureAsset;
using TextureHandle = WeakLocalPoolIndex<TextureAsset, ObjectPool>;

class Thread;
class Engine;
class IPlatformWindow;

class IRenderingSystem
{
    friend Engine;

  protected:
    static const int32_t sMaxDirLightCount = 4;
    static const int32_t sMaxPointLightCount = 32;
    static const int32_t sMaxSpotLightCount = 16;
    static const int32_t sMaxOpaqueDrawCommandCount = 4096;
    static const int32_t sMaxMaskedDrawCommandCount = 1024;
    static const int32_t sMaxBlendedDrawCommandCount = 1024;
    static const int32_t sMaxRenderPassCount = 4;
    static const int32_t sMaxViewportCount = 16;
    static const int32_t sPendingFrameBufferQueueSize = 10;
    static const int32_t sPendingResourceQueueSize = 1024;

  public:
    enum class eRenderTargetType
    {
        Color = 0,
        Depth = 1,
        Last = 2,
    };

    enum class eRenderTargetFormat
    {
        None = 0,
        R8G8B8A8_UNORM,
        B8G8R8A8_UNORM,
        R16G16B16A16_FLOAT,
        R32_FLOAT,
        R16G16_FLOAT,
        R32G32B32A32_FLOAT,
        D24_UNORM_S8_UINT,
        D32_FLOAT,
    };

    struct RenderTargetDesc
    {
        eRenderTargetFormat Format = eRenderTargetFormat::None;
        Color128 ClearColor;
        float ClearDepth = 1.0f;
        uint8_t ClearStencil = 0;
    };

    struct FrameBufferDesc
    {
        StringHandle hName = StringHandle::sNULL;
        int32_t Width = 0;
        int32_t Height = 0;
        int32_t SampleCount = 1;
        RenderTargetDesc TargetDesc[static_cast<int32_t>(eRenderTargetType::Last)] = {};
    };

    struct DirLightDesc
    {
        Vector3 Direction;
        Color128 Color;
        float Intensity = 1.0f;
    };

    struct PointLightDesc
    {
        Vector3 Position;
        Color128 Color;
        float Intensity = 1.0f;

        float Constant = 1.0f;
        float Linear = 0.045f;
        float Quadratic = 0.0075f;
    };

    struct SpotLightDesc
    {
        Vector3 Position;
        Vector3 Direction;
        Color128 Color;
        float Intensity = 0.0f;

        float InnerCutOff = 0.0f;
        float OuterCutOff = 0.0f;

        float Constant = 1.0f;
        float Linear = 0.045f;
        float Quadratic = 0.0075f;
    };

    struct DrawCommandDesc
    {
        GpuStaticMeshHandle hGpuStaticMesh = GpuStaticMeshHandle::sNULL;
        GpuMaterialHandle hGpuMaterial = GpuMaterialHandle::sNULL;
        int32_t VertexOffset = 0;
        int32_t IndexCount = 0;
        int32_t IndexOffset = 0;
        Matrix4x4 WorldMat = Matrix4x4::sIdentity;
        eMaterialAlphaMode AlphaMode = eMaterialAlphaMode::None;
    };

    struct Rect
    {
        int32_t X = 0;
        int32_t Y = 0;
        int32_t Width = 0;
        int32_t Height = 0;
    };

    struct RenderPassDesc
    {
        StringHandle hFrameBufferName = StringHandle::sNULL;
        bool bClearTarget[static_cast<int32_t>(eRenderTargetType::Last)] = {};
        Rect Viewport;

        Vector3 WorldCameraPos;
        Matrix4x4 ViewMat = Matrix4x4::sIdentity;
        Matrix4x4 ProjMat = Matrix4x4::sIdentity;

        DirLightDesc DirLights[sMaxDirLightCount] = {};
        int32_t DirLightCount = 0;

        PointLightDesc PointLights[sMaxPointLightCount] = {};
        int32_t PointLightCount = 0;

        SpotLightDesc SpotLights[sMaxSpotLightCount] = {};
        int32_t SpotLightCount = 0;

        GpuTextureHandle hSkyMap = GpuTextureHandle::sNULL;
        GpuTextureHandle hIrradianceMap = GpuTextureHandle::sNULL;
    };

    static IRenderingSystem& GetInstance()
    {
        HO_ASSERT(spInstance, "RenderingSystem is not created");
        return *spInstance;
    }

    virtual ~IRenderingSystem() = default;

    bool EnqueueCreateFramebuffer(const FrameBufferDesc& frameBufferDesc);

    bool EnqueueDestroyFramebuffer(StringHandle hFrameBufferName);

    [[nodiscard]] virtual void* GetRenderTargetNativeHandle(StringHandle hFrameBufferName,
                                                            eRenderTargetType type,
                                                            bool bRequireMultisample = false) const = 0;

    bool BeginRenderPass(RenderPassDesc& renderPassDesc);

    bool SubmitDrawCommand(const DrawCommandDesc& commandDesc);

    bool SubmitUIViewportData(const IPlatformWindow* pTargetWindow, const ImDrawData& srcDrawData);

    bool EndRenderPass();

    GpuStaticMeshHandle EnqueueUploadStaticMesh(StaticMeshHandle hStaticMesh, bool bPersistent = false);
    GpuMaterialHandle EnqueueUploadMaterial(MaterialHandle hMaterial, bool bPersistent = false);
    GpuTextureHandle EnqueueUploadTexture(TextureHandle hTexture, bool bPersistent = false);
    GpuShaderHandle EnqueueUploadShader(ShaderHandle hShader, bool bPersistent = false);

  protected:
    struct FrameBuffer
    {
        StringHandle hName = StringHandle::sNULL;
        int32_t Width = 0;
        int32_t Height = 0;
        int32_t SampleCount = 1;
        // render targets are double buffered too. It's synchronized current render queue.
        GpuTextureHandle hRenderTargets[2][static_cast<int32_t>(eRenderTargetType::Last)] = {};
        GpuTextureHandle hResolvedRenderTargets[2][static_cast<int32_t>(eRenderTargetType::Last)] = {};
        Vector4 ClearColors[static_cast<int32_t>(eRenderTargetType::Last)] = {};
        float ClearDepth = 1.f;
        uint8_t ClearStencil = 0;
    };

    struct DrawCommand
    {
        GpuStaticMeshHandle hGpuStaticMesh = GpuStaticMeshHandle::sNULL;
        GpuMaterialHandle hGpuMaterial = GpuMaterialHandle::sNULL;
        int32_t VertexOffset = 0;
        int32_t IndexCount = 0;
        int32_t IndexOffset = 0;
        Matrix4x4 WorldMat = Matrix4x4::sIdentity;
        float SqrdDistanceFromCamera = 0.0f;
    };

    // Layout struct used for uploading to GPU.
    struct MatrixLayout
    {
        Matrix4x4 WorldMat;
        Matrix4x4 InvWorldMat;
        Matrix4x4 ViewMat;
        Matrix4x4 ProjMat;
    };

    struct DirLightLayout
    {
        float Direction[3] = {};
        float Padding = 0.0f;

        float Color[3] = {};
        float Intensity = 0.0f;
    };

    struct PointLightLayout
    {
        float Position[3] = {};
        float Padding0 = 0.0f;

        float Color[3] = {};

        float Intensity = 0.0f;
        float Constant = 0.0f;
        float Linear = 0.0f;
        float Quadratic = 0.0f;
        float Padding1 = 0.0f;
    };

    struct SpotLightLayout
    {
        float Position[3] = {};
        float Padding0 = 0.0f;

        float Direction[3] = {};
        float Padding1 = 0.0f;

        float Color[3] = {};

        float Intensity = 0.0f;
        float InnerCutOff = 0.0f;
        float OuterCutOff = 0.0f;

        float Constant = 1.0f;
        float Linear = 0.045f;
        float Quadratic = 0.0075f;
        float Padding3[3] = {};
    };

    struct LightLayout
    {
        int32_t DirLightCount = 0;
        int32_t Padding0[3] = {};
        DirLightLayout DirLights[sMaxDirLightCount] = {};
        int32_t PointLightCount = 0;
        int32_t Padding1[3] = {};
        PointLightLayout PointLights[sMaxPointLightCount] = {};
        int32_t SpotLightCount = 0;
        int32_t Padding2[3] = {};
        SpotLightLayout SpotLights[sMaxSpotLightCount] = {};
    };

    struct GeneralPurposeLayout
    {
        float WorldCameraPos[3] = {};
        float Padding = 0;
    };

    struct RenderPass
    {
        FrameBuffer* pFrameBuffer = nullptr;
        bool bClearTarget[static_cast<int32_t>(eRenderTargetType::Last)] = {};
        Rect Viewport;

        Vector3 WorldCameraPos;
        Matrix4x4 ViewMat = Matrix4x4::sIdentity;
        Matrix4x4 ProjMat = Matrix4x4::sIdentity;

        LightLayout Lights;
        DrawCommand OpaqueDrawCommands[sMaxOpaqueDrawCommandCount] = {};
        int32_t OpaqueDrawCommandCount = 0;
        DrawCommand MaskedDrawCommands[sMaxMaskedDrawCommandCount] = {};
        int32_t MaskedDrawCommandCount = 0;
        DrawCommand BlendedDrawCommands[sMaxBlendedDrawCommandCount] = {};
        int32_t BlendedDrawCommandCount = 0;

        GpuTexture* pSkyMap = nullptr;
        GpuTexture* pIrradianceMap = nullptr;
    };

    struct UIViewportData
    {
        UIViewportData();
        ~UIViewportData();
        UIViewportData(UIViewportData&&) noexcept;
        UIViewportData& operator=(UIViewportData&&) noexcept;

        const IPlatformWindow* pTargetWindow = nullptr;
        std::unique_ptr<ImDrawData> pDrawData;
    };

    struct RenderQueue
    {
        uint64_t FrameCount = 0;
        RenderPass RenderPasses[sMaxRenderPassCount] = {};
        int32_t ActivePassCount = 0;

        UIViewportData UIViewports[sMaxViewportCount] = {};
        int32_t ActiveViewportCount = 0;
    };

    enum class eResourceTransferResult
    {
        Success = 0,
        Failed = 1,
        NeedsRetry = 2,
    };

    IRenderingSystem();

    static void createInstance(eGraphicsAPI api);
    static void deleteInstance();

    // 'init' called by engine.
    virtual bool init() = 0;
    // 'run' function creates rendering thread.
    // Engine must be create all needed frame buffers before create render thread.
    void run();
    static void runInternal(void* unused);
    // 'shutdown' called by engine and it will terminate 'run' function
    void shutdown();

    virtual void releaseAllResources() = 0;

    void executeRenderQueue();

    void updateDrawCommandResourceUsageStates(DrawCommand& command, uint64_t currentFrame);
    void updateRenderPassResourceUsageStates(RenderPass& pass, uint64_t currentFrame);

    void sortDrawCommands(RenderPass& pass);

    virtual void applyMaterial(GpuMaterialHandle hGpuMaterial) = 0;
    virtual void executeDrawCommand(const DrawCommand& command) = 0;

    virtual void executeSkyMapPass(const GpuTexture* pSkyMap) = 0;
    virtual void executeInWorldDebugDrawPass() = 0;
    virtual void executeOverlayDebugDrawPass() = 0;

    virtual void prepareExecuteRenderQueue() = 0;
    virtual void prepareExecuteRenderPass(const RenderPass& pass) = 0;
    virtual void prepareExecuteOpaqueDrawCommands() = 0;
    virtual void prepareExecuteMaskedDrawCommands() = 0;
    virtual void prepareExecuteBlendedDrawCommands() = 0;
    virtual void finishExecuteRenderPass(const RenderPass& pass) = 0;
    virtual void finishExecuteRenderQueue() = 0;

    virtual void resolveFrameBuffer(const FrameBuffer& frameBuffer) = 0;

    virtual void renderUI() = 0;

    virtual void flush() = 0;

    void swapRenderQueues(uint64_t currentFrame);

    bool trySwapRenderQueues(uint64_t currentFrame);

    virtual bool createFrameBuffer(const FrameBufferDesc& frameBufferDesc) = 0;
    virtual bool destroyFrameBuffer(StringHandle hFrameBufferName) = 0;

    void createPendingFrameBuffers();
    void destroyPendingFrameBuffers();

    void uploadPendingResources(uint64_t currentFrame);

    int32_t evictStaleResources(uint64_t currentFrame, uint64_t frameThreshold);

    GpuStaticMeshHandle prepareUploadStaticMesh(StaticMeshHandle hStaticMesh, bool bPersistent);
    GpuMaterialHandle prepareUploadMaterial(MaterialHandle hMaterial, bool bPersistent);
    // 'prepareUploadTexture' can be used for uploading render target.
    // When hTextureTarget is TextureHandle::sNULL, bPersistent is true, it will create render target.
    GpuTextureHandle prepareUploadTexture(TextureHandle hTexture, bool bPersistent);
    GpuShaderHandle prepareUploadShader(ShaderHandle hShader, bool bPersistent);

    virtual eResourceTransferResult uploadStaticMesh(GpuStaticMeshHandle hPreparedGpuStaticMesh,
                                                     uint64_t currentFrame) = 0;
    virtual eResourceTransferResult uploadMaterial(GpuMaterialHandle hPreparedGpuMaterial, uint64_t currentFrame) = 0;
    virtual eResourceTransferResult uploadTexture(GpuTextureHandle hPreparedGpuTexture, uint64_t currentFrame) = 0;
    virtual eResourceTransferResult uploadShader(GpuShaderHandle hPreparedGpuShader, uint64_t currentFrame) = 0;

    virtual eResourceTransferResult unloadStaticMesh(GpuStaticMeshHandle hGpuStaticMesh) = 0;
    virtual eResourceTransferResult unloadMaterial(GpuMaterialHandle hGpuMaterial) = 0;
    virtual eResourceTransferResult unloadTexture(GpuTextureHandle hGpuTexture) = 0;
    virtual eResourceTransferResult unloadShader(GpuShaderHandle hGpuShader) = 0;

    void finishUnloadStaticMesh(GpuStaticMeshHandle hUnloadedGpuStaticMesh);
    void finishUnloadMaterial(GpuMaterialHandle hUnloadedGpuMaterial);
    void finishUnloadTexture(GpuTextureHandle hUnloadedGpuTexture);
    void finishUnloadShader(GpuShaderHandle hUnloadedGpuShader);

    bool validateFrameBufferDesc(const FrameBufferDesc& desc);

    // -1 means render pass is not begun.
    int32_t mCurrentRenderPassIndex = -1;
    int32_t mFrontRenderQueueIndex = 0;
    int32_t mBackRenderQueueIndex = 1;
    RenderQueue mRenderQueues[2];

    std::unordered_map<StringHandle, FrameBuffer> mNameToFrameBufferMap;
    FixedQueue<FrameBufferDesc> mFrameBufferToCreateQueue;
    SpinLock mFrameBufferToCreateQueueLock;
    FixedQueue<StringHandle> mhFrameBufferToDestroyQueue;
    SpinLock mFrameBufferToDestroyQueueLock;

    ObjectPool<GpuStaticMesh> mGpuStaticMeshPool;
    ObjectPool<GpuMaterial> mGpuMaterialPool;
    ObjectPool<GpuShader> mGpuShaderPool;
    ObjectPool<GpuTexture> mGpuTexturePool;

    FixedQueue<GpuStaticMeshHandle> mhPendingStaticMeshQueue;
    SpinLock mPendingStaticMeshQueueLock;
    FixedQueue<GpuMaterialHandle> mhPendingMaterialQueue;
    SpinLock mPendingMaterialQueueLock;
    FixedQueue<GpuTextureHandle> mhPendingTextureQueue;
    SpinLock mPendingTextureQueueLock;
    FixedQueue<GpuShaderHandle> mhPendingShaderQueue;
    SpinLock mPendingShaderQueueLock;

    bool mbRunning = true;
    RenderSync mRenderSync;

    std::unique_ptr<Thread> mpRenderThread;

    static IRenderingSystem* spInstance;
};

} // namespace ho