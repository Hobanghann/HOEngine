#pragma once

#include "../RenderData/GpuResources.h"
#include "Core/Math/Rect.h"
#include "Core/Memory/TypedObjectPool.h"
#include "Core/Templates/FixedQueue.h"
#include "RenderBindingIndex.h"

namespace ho
{
template <typename T>
class ObjectHandle;

struct DrawCommand;

struct FrameBufferDesc;

struct UIViewportData;

struct MaterialAttributeLayout;

class IRenderingBackend
{
  public:
    enum class eAlphaBlendMode
    {
        None,     // Disable alpha blend.
        Default,  // SrcFactor = SrcAlpha, DstFactor = 1 - SrcAlpha
        Additive, // SrcFactor = SrcAlpha, DstFactor = 1
    };

    enum class ePolygonMode
    {
        Point,
        Line,
        Triangle,
    };

    enum class eCullMode
    {
        None, // Disable face culling.
        Front,
        Back,
    };

    enum class eComparisonFunc
    {
        Never,
        Less,
        Equal,
        LEqual,
        Greater,
        NotEqual,
        GEqual,
        Always,
    };

    enum class eStencilOperator
    {
        Keep,
        Zero,
        Replace,
        IncreaseWrap,
        IncreaseClamp,
        DecreaseWrap,
        DecreaseClamp,
        Invert,
    };

    enum class eResourceAccessType
    {
        ReadOnly,
        WriteOnly,
        ReadAndWrite,
    };

    enum class eResourceTransferResult
    {
        Success = 0,
        Failed = 1,
        NeedsRetry = 2,
    };

    enum class eResourceState
    {
        // Initial or unknown state where previous contents can be discarded
        Undefined,

        // Used as vertex buffer data for geometry rendering
        VertexBuffer,

        // Used as index buffer data for geometry rendering
        IndexBuffer,

        // Used as constant or uniform buffer data read by shaders
        ConstantBuffer,

        // Used as a texture sampled inside shaders (e.g., albedo, normal maps)
        ShaderResource,

        // Used as a color render target attachment for pixel writes
        RenderTarget,

        // Used as a depth/stencil attachment for writing depth values
        DepthStencilWrite,

        // Used as a read-only depth/stencil resource (e.g., depth testing or sampling)
        DepthStencilRead,

        // Used for unordered read/write access (e.g., in compute shaders or storage buffers)
        UnorderedAccess,

        // Used as a source for copy, transfer, or blit operations
        CopySource,

        // Used as a destination for copy, transfer, or blit operations
        CopyDestination,

        // Used for the final swapchain backbuffer presented to the display
        Present
    };

    IRenderingBackend();
    virtual ~IRenderingBackend();

    virtual bool Init();
    virtual void Shutdown() = 0;

    // Begins a new rendering frame and sets the current frame index.
    // NOTE: All rendering backend APIs must be invoked between BeginFrame() and EndFrame().
    // Performs per-frame setup and handles resource maintenance/eviction for the frame.
    // It's safe to call on main thread.
    void BeginFrame(uint64_t currentFrame);

    // Initializes pipeline states before starting a sequence of draw calls.
    virtual void BeginDrawSequence() = 0;

    // Restores pipeline states to their default or previous baseline.
    virtual void EndDrawSequence() = 0;

    // Concludes the current rendering frame and flushes pending frame operations.
    // NOTE: All rendering backend APIs must be invoked between BeginFrame() and EndFrame().
    // It's safe to call on main thread.
    void EndFrame();

    virtual void SetAlphaBlendMode(eAlphaBlendMode mode) = 0;
    virtual void SetPolygonMode(ePolygonMode mode) = 0;
    virtual void SetCullMode(eCullMode mode) = 0;

    virtual void SetDepthTest(bool bEnabled) = 0;
    virtual void SetDepthWrite(bool bEnabled) = 0;
    virtual void SetDepthFunc(eComparisonFunc func) = 0;

    virtual void SetStencilTest(bool bEnabled) = 0;
    virtual void SetStencilWriteMask(uint8_t writeMask) = 0;
    virtual void SetStencilState(eComparisonFunc func, uint8_t compareMask) = 0;
    virtual void SetStencilReference(uint8_t ref) = 0;
    virtual void SetStencilOperator(eStencilOperator stencilFailOp,
                                    eStencilOperator depthFailOp,
                                    eStencilOperator depthPassOp) = 0;

    virtual void BindFrameBuffer(FrameBufferHandle hFrameBuffer) = 0;

    // AttachTarget functions bind a single 2D texture.
    // Use the corresponding Range functions to bind a specific range of array textures or cubemaps.
    // NOTE: Textures attached to a framebuffer are treated as persistent and protected from eviction.
    void AttachColorTarget(FrameBufferHandle hFrameBuffer,
                           eRenderTargetBindingIndex targetIndex,
                           GpuTextureHandle hTarget);
    void AttachDepthTarget(FrameBufferHandle hFrameBuffer, GpuTextureHandle hTarget);
    virtual void AttachColorTargetRange(FrameBufferHandle hFrameBuffer,
                                        eRenderTargetBindingIndex targetIndex,
                                        GpuTextureHandle hTarget,
                                        int32_t baseSlice,
                                        int32_t sliceCount,
                                        int32_t mipLevel) = 0;
    virtual void AttachDepthTargetRange(FrameBufferHandle hFrameBuffer,
                                        GpuTextureHandle hTarget,
                                        int32_t baseSlice,
                                        int32_t sliceCount,
                                        int32_t mipLevel) = 0;

    // NOTE: Unbinding a target unloads its corresponding resolved target as well.
    virtual void DetachColorTarget(FrameBufferHandle hFrameBuffer, eRenderTargetBindingIndex targetIndex) = 0;
    virtual void DetachDepthTarget(FrameBufferHandle hFrameBuffer) = 0;

    virtual void SetViewport(const Rect& viewport) = 0;

    virtual void ClearColorTarget(FrameBufferHandle hFrameBuffer,
                                  eRenderTargetBindingIndex targetIndex,
                                  const float* pColor) const = 0;
    virtual void ClearDepthTarget(FrameBufferHandle hFrameBuffer, float depth) const = 0;
    virtual void ClearDepthStencilTarget(FrameBufferHandle hFrameBuffer, float depth, int32_t stencil) const = 0;
    virtual void ClearStencilTarget(FrameBufferHandle hFrameBuffer, int32_t stencil) const = 0;

    virtual void BindBuffer(GpuBufferHandle hGpuBuffer, eBufferBindingIndex bindingIndex) = 0;
    virtual void BindBufferSlice(GpuBufferHandle hGpuBuffer,
                                 eBufferBindingIndex bindingIndex,
                                 int32_t size,
                                 int32_t offset) = 0;

    virtual void BindStorageBuffer(GpuBufferHandle hGpuBuffer, eBufferBindingIndex bindingIndex) = 0;
    virtual void BindStorageBufferSlice(GpuBufferHandle hGpuBuffer,
                                        eBufferBindingIndex bindingIndex,
                                        int32_t size,
                                        int32_t offset) = 0;

    virtual void BindStaticMesh(GpuStaticMeshHandle hMesh) = 0;

    void UploadMaterialAttributes(GpuMaterialHandle hMaterial, const MaterialAttributeLayout& attributes);

    void BindMaterialAttributes(GpuMaterialHandle hMaterial);

    virtual void BindTexture(GpuTextureHandle hTexture, eTextureBindingIndex bindingIndex) = 0;

    virtual void BindStorageTexture(GpuTextureHandle hTexture,
                                    eStorageTextureBindingIndex bindingIndex,
                                    int32_t mipLevel,
                                    int32_t arrayIdx,
                                    eResourceAccessType access) = 0;

    virtual void BindShader(GpuShaderHandle hShader, eShaderStage stage) = 0;

    virtual void BindMorphTargets(GpuMorphTargetsHandle hGpuMorphTargets) = 0;

    virtual void ExecuteDrawCommand(const DrawCommand& command) = 0;

    // Issues a bufferless draw call (gl_VertexID/SV_VertexID based) without VBO/IBO bindings.
    virtual void DrawProcedural(int32_t vertexCount) = 0;

    virtual void Dispatch(uint32_t groupXCount, uint32_t groupYCount, uint32_t groupZCount) = 0;

    virtual void RenderUI(const UIViewportData& data) = 0;

    virtual void Flush() = 0;

    virtual void ResourceBarrier(GpuTextureHandle hTexture, eResourceState srcState, eResourceState dstState) = 0;
    virtual void ResourceBarrier(GpuBufferHandle hBuffer, eResourceState srcState, eResourceState dstState) = 0;

    virtual void SetVSync(bool bEnabled) = 0;

    // The rendering backend automatically creates render targets for any format specified other than
    // eTextureFormat::None.
    virtual FrameBufferHandle CreateFrameBuffer(const FrameBufferDesc& desc) = 0;

    // The rendering backend automatically unloads all attached render targets.
    virtual bool DestroyFrameBuffer(FrameBufferHandle hFrameBuffer) = 0;

    // NOTE: These getter functions are read-only and can be invoked at any time,
    //       independently of the BeginFrame() and EndFrame() lifecycle.
    virtual void* GetColorTargetNativeHandle(FrameBufferHandle hFrameBuffer,
                                             eRenderTargetBindingIndex targetIndex) const = 0;
    virtual void* GetDepthTargetNativeHandle(FrameBufferHandle hFrameBuffer) const = 0;

    // Evicts GPU resources unused for more than 'frameThreshold' frames relative to 'currentFrame'.
    // Returns the total number of evicted resources.
    // TODO: Check cache hit rate 1 loop way vs 4 loop way later.
    // FIX: This function make origin asset's hRenderProxy invalid. It causes race condition!!!!!!!!!!!!!!!!!!
    int32_t EvictStaleResources(uint64_t frameThreshold);

    GpuBufferHandle PrepareCreateBuffer(int32_t byteSize,
                                        eGpuBufferUsage usage,
                                        eGpuBufferAccess access,
                                        bool bPersistent);

    // NOTE: APIs starting with 'Prepare' do not invoke any graphics APIs and can be invoked at any time,
    //       independently of the BeginFrame() and EndFrame() lifecycle.
    GpuStaticMeshHandle PrepareUploadStaticMesh(ObjectHandle<RStaticMeshAsset> hStaticMesh, bool bPersistent);
    GpuMaterialHandle PrepareUploadMaterial(ObjectHandle<RMaterialAsset> hMaterial, bool bPersistent);
    GpuTextureHandle PrepareUploadTexture(ObjectHandle<RTextureAsset> hTexture, bool bPersistent);
    GpuShaderHandle PrepareUploadShader(ObjectHandle<RShaderAsset> hShader, bool bPersistent);
    GpuMorphTargetsHandle PrepareUploadMorphTargets(ObjectHandle<RMorphTargetsAsset> hMorphTargets, bool bPersistent);

    GpuTextureHandle PrepareCreateTexture1D(eTextureFormat format, int32_t width, int32_t mipLevels, bool bPersistent);
    GpuTextureHandle PrepareCreateTexture2D(
        eTextureFormat format, int32_t width, int32_t height, int32_t mipLevels, int32_t sampleCount, bool bPersistent);
    GpuTextureHandle PrepareCreateTexture3D(
        eTextureFormat format, int32_t width, int32_t height, int32_t depth, int32_t mipLevels, bool bPersistent);
    GpuTextureHandle PrepareCreateTextureCubeMap(
        eTextureFormat format, int32_t width, int32_t height, int32_t mipLevels, bool bPersistent);
    GpuTextureHandle PrepareCreateTexture1DArray(
        eTextureFormat format, int32_t width, int32_t arraySize, int32_t mipLevels, bool bPersistent);
    GpuTextureHandle PrepareCreateTexture2DArray(eTextureFormat format,
                                                 int32_t width,
                                                 int32_t height,
                                                 int32_t arraySize,
                                                 int32_t mipLevels,
                                                 int32_t sampleCount,
                                                 bool bPersistent);
    GpuTextureHandle PrepareCreateTextureCubeMapArray(
        eTextureFormat format, int32_t width, int32_t height, int32_t arraySize, int32_t mipLevels, bool bPersistent);

    // Does not allocate GPU memory immediately. Memory will be allocated at upload time.
    virtual eResourceTransferResult CreateBuffer(GpuBufferHandle hPreparedGpuBuffer) = 0;

    virtual eResourceTransferResult CreateTexture(GpuTextureHandle hPreparedGpuTexture) = 0;

    virtual eResourceTransferResult UploadBufferData(GpuBufferHandle hGpuBuffer,
                                                     const void* pSrcData,
                                                     int32_t size,
                                                     int32_t offset = 0) = 0;

    // TODO: Optimize vertex attribute memory allocation.
    // Currently, memory is redundantly allocated under the assumption that all vertex attributes are present.
    // Optimize this in the future to pack and upload only the attributes that actually exist.
    virtual eResourceTransferResult UploadStaticMesh(GpuStaticMeshHandle hPreparedGpuStaticMesh) = 0;

    eResourceTransferResult UploadMaterial(GpuMaterialHandle hPreparedGpuMaterial);
    virtual eResourceTransferResult UploadTexture(GpuTextureHandle hPreparedGpuTexture) = 0;
    virtual eResourceTransferResult UploadShader(GpuShaderHandle hPreparedGpuShader) = 0;
    virtual eResourceTransferResult UploadMorphTargets(GpuMorphTargetsHandle hPreparedGpuMorphTargets) = 0;

    virtual eResourceTransferResult DownloadBufferData(GpuBufferHandle hGpuBuffer,
                                                       void* pDstData,
                                                       int32_t size,
                                                       int32_t offset = 0) = 0;

    // NOTE: Destination asset's transfer state mube be in 'PendingDownload' state.
    // FIX: This function change destination texture's transter state. It cause race condition!!!!!!!!!!!!!!!!!
    virtual eResourceTransferResult DownloadTexture(GpuTextureHandle hGpuTexture,
                                                    ObjectHandle<RTextureAsset> hDstTexture) = 0;

    virtual eResourceTransferResult UnloadBuffer(GpuBufferHandle hGpuBuffer) = 0;
    virtual eResourceTransferResult UnloadStaticMesh(GpuStaticMeshHandle hGpuStaticMesh) = 0;
    eResourceTransferResult UnloadMaterial(GpuMaterialHandle hGpuMaterial);
    virtual eResourceTransferResult UnloadTexture(GpuTextureHandle hGpuTexture) = 0;
    virtual eResourceTransferResult UnloadShader(GpuShaderHandle hGpuShader) = 0;
    virtual eResourceTransferResult UnloadMorphTargets(GpuMorphTargetsHandle hGpuMorphTargets) = 0;

    void UnloadAllResources();

  protected:
    struct BufferBindingState
    {
        GpuBufferHandle hBoundBuffer = GpuBufferHandle::sNULL;
        int32_t Size = 0;
        int32_t Offset = 0;
    };

    struct StorageTextureBindingState
    {
        GpuTextureHandle hBoundTexture = GpuTextureHandle::sNULL;
        int32_t MipLevel = 0;
        int32_t ArrayIdx = 0;
        eResourceAccessType Access = eResourceAccessType::WriteOnly;
    };

    struct RenderState
    {
        bool bFrameBegun = false;
        uint64_t CurrentFrame = 0;

        FrameBufferHandle hBoundFrameBuffer;
        Rect Viewport;

        eAlphaBlendMode AlphaBlendMode = eAlphaBlendMode::None;
        ePolygonMode PolygonMode = ePolygonMode::Triangle;
        eCullMode CullMode = eCullMode::None;

        bool bDepthTestEnabled = false;
        bool bDepthWriteEnabled = false;
        eComparisonFunc DepthFunc = eComparisonFunc::Less;

        bool bStencilTestEnabled = false;
        uint8_t StencilWriteMask = 0xFF;
        eComparisonFunc StencilFunc = eComparisonFunc::Equal;
        uint8_t StencilCompareMask = 0xFF;
        uint8_t StencilReference = 1;
        eStencilOperator StencilFailOp = eStencilOperator::Keep;
        eStencilOperator DepthFailOp = eStencilOperator::Keep;
        eStencilOperator DepthPassOp = eStencilOperator::Keep;

        BufferBindingState BufferBindingStates[static_cast<int32_t>(eBufferBindingIndex::Last)] = {};

        GpuStaticMeshHandle hBoundStaticMesh;
        GpuMaterialHandle hBoundMaterial;
        GpuTextureHandle hBoundTextures[static_cast<int32_t>(eTextureBindingIndex::Last)] = {};
        StorageTextureBindingState
            StorageTextureBindingStates[static_cast<int32_t>(eStorageTextureBindingIndex::Last)] = {};
        GpuShaderHandle hBoundShaders[static_cast<int32_t>(eShaderStage::Last)] = {};

        GpuMorphTargetsHandle hBoundMorphTargets;
    };

    void finishUnloadBuffer(GpuBufferHandle hUnloadedGpuBuffer);
    void finishUnloadStaticMesh(GpuStaticMeshHandle hUnloadedGpuStaticMesh);
    void finishUnloadMaterial(GpuMaterialHandle hUnloadedGpuMaterial);
    void finishUnloadTexture(GpuTextureHandle hUnloadedGpuTexture);
    void finishUnloadShader(GpuShaderHandle hUnloadedGpuShader);
    void finishUnloadMorphTargets(GpuMorphTargetsHandle hUnloadedGpuMorphTargets);

    // Configures texture sampling methods (filtering, wrapping, compare mode) for each binding index in
    // eTextureBindingIndex via the graphics API and pre-binds them to their slots.
    virtual bool initializeSamplers() = 0;

    void clearResourceBindingState();

    RenderState mCurrentState;

    TypedObjectPool<FrameBuffer> mFrameBufferPool;

    TypedObjectPool<GpuStaticMesh> mGpuStaticMeshPool;
    TypedObjectPool<GpuMaterial> mGpuMaterialPool;
    TypedObjectPool<GpuShader> mGpuShaderPool;
    TypedObjectPool<GpuTexture> mGpuTexturePool;
    TypedObjectPool<GpuMorphTargets> mGpuMorphTargetsPool;

    TypedObjectPool<GpuBuffer> mGpuBufferPool;

    bool mbVSyncEnabled = false;
};
} // namespace ho