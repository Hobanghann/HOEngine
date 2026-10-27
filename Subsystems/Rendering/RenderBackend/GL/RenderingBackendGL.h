#pragma once

#include <glad/include/glad/glad.h>

#include "../IRenderingBackend.h"
#include "NativeHandlesGL.h"

namespace ho
{
struct DrawCommand;
struct FrameBufferDesc;
struct UIViewportData;

class RenderingBackendGL final : public IRenderingBackend
{
  public:
    RenderingBackendGL();
    ~RenderingBackendGL() override;

    bool Init() override;
    void Shutdown() override;

    void BeginDrawSequence() override;
    void EndDrawSequence() override;

    void SetAlphaBlendMode(eAlphaBlendMode mode) override;
    void SetPolygonMode(ePolygonMode mode) override;
    void SetCullMode(eCullMode mode) override;

    void SetDepthTest(bool bEnabled) override;
    void SetDepthWrite(bool bEnabled) override;
    void SetDepthFunc(eComparisonFunc func) override;

    void SetStencilTest(bool bEnabled) override;
    void SetStencilWriteMask(uint8_t writeMask) override;
    void SetStencilState(eComparisonFunc func, uint8_t compareMask) override;
    void SetStencilReference(uint8_t ref) override;
    void SetStencilOperator(eStencilOperator stencilFailOp,
                            eStencilOperator depthFailOp,
                            eStencilOperator depthPassOp) override;

    void BindFrameBuffer(FrameBufferHandle hFrameBuffer) override;
    void AttachColorTargetRange(FrameBufferHandle hFrameBuffer,
                                eRenderTargetBindingIndex targetIndex,
                                GpuTextureHandle hTarget,
                                int32_t baseSlice,
                                int32_t sliceCount,
                                int32_t mipLevel) override;
    void AttachDepthTargetRange(FrameBufferHandle hFrameBuffer,
                                GpuTextureHandle hTarget,
                                int32_t baseSlice,
                                int32_t sliceCount,
                                int32_t mipLevel) override;

    void DetachColorTarget(FrameBufferHandle hFrameBuffer, eRenderTargetBindingIndex targetIndex) override;
    void DetachDepthTarget(FrameBufferHandle hFrameBuffer) override;

    void SetViewport(const Rect& viewport) override;

    void ClearColorTarget(FrameBufferHandle hFrameBuffer,
                          eRenderTargetBindingIndex targetIndex,
                          const float* pColor) const override;
    void ClearDepthTarget(FrameBufferHandle hFrameBuffer, float depth) const override;
    void ClearDepthStencilTarget(FrameBufferHandle hFrameBuffer, float depth, int32_t stencil) const override;
    void ClearStencilTarget(FrameBufferHandle hFrameBuffer, int32_t stencil) const override;

    void BindBuffer(GpuBufferHandle hGpuBuffer, eBufferBindingIndex bindingIndex) override;
    void BindBufferSlice(GpuBufferHandle hGpuBuffer,
                         eBufferBindingIndex bindingIndex,
                         int32_t size,
                         int32_t offset) override;

    void BindStorageBuffer(GpuBufferHandle hGpuBuffer, eBufferBindingIndex bindingIndex) override;
    void BindStorageBufferSlice(GpuBufferHandle hGpuBuffer,
                                eBufferBindingIndex bindingIndex,
                                int32_t size,
                                int32_t offset) override;

    void BindStaticMesh(GpuStaticMeshHandle hMesh) override;

    void BindTexture(GpuTextureHandle hTexture, eTextureBindingIndex bindingIndex) override;

    void BindStorageTexture(GpuTextureHandle hTexture,
                            eStorageTextureBindingIndex bindingIndex,
                            int32_t mipLevel,
                            int32_t arrayIdx,
                            eResourceAccessType access) override;

    void BindShader(GpuShaderHandle hShader, eShaderStage stage) override;

    void BindMorphTargets(GpuMorphTargetsHandle hGpuMorphTargets) override;

    void ExecuteDrawCommand(const DrawCommand& command) override;

    // Issues a bufferless draw call without VBO/IBO bindings.
    void DrawProcedural(int32_t vertexCount) override;

    void Dispatch(uint32_t groupXCount, uint32_t groupYCount, uint32_t groupZCount) override;

    void RenderUI(const UIViewportData& data) override;

    void Flush() override;

    void ResourceBarrier(GpuTextureHandle hTexture, eResourceState srcState, eResourceState dstState) override;
    void ResourceBarrier(GpuBufferHandle hBuffer, eResourceState srcState, eResourceState dstState) override;

    void SetVSync(bool bEnabled) override;

    FrameBufferHandle CreateFrameBuffer(const FrameBufferDesc& desc) override;
    bool DestroyFrameBuffer(FrameBufferHandle hFrameBuffer) override;

    void* GetColorTargetNativeHandle(FrameBufferHandle hFrameBuffer,
                                     eRenderTargetBindingIndex targetIndex) const override;

    void* GetDepthTargetNativeHandle(FrameBufferHandle hFrameBuffer) const override;

    eResourceTransferResult CreateBuffer(GpuBufferHandle hPreparedGpuBuffer) override;
    eResourceTransferResult CreateTexture(GpuTextureHandle hPreparedGpuTexture) override;

    eResourceTransferResult UploadBufferData(GpuBufferHandle hGpuBuffer,
                                             const void* pSrcData,
                                             int32_t size,
                                             int32_t offset = 0) override;

    // TODO: Currently, all vertex attributes are always packed and uploaded interleaved
    //       regardless of their actual presence in the asset.
    //       Need to optimize vertex buffer generation to pack and upload only the active attributes.
    eResourceTransferResult UploadStaticMesh(GpuStaticMeshHandle hPreparedGpuStaticMesh) override;
    eResourceTransferResult UploadTexture(GpuTextureHandle hPreparedGpuTexture) override;
    eResourceTransferResult UploadShader(GpuShaderHandle hPreparedGpuShader) override;
    eResourceTransferResult UploadMorphTargets(GpuMorphTargetsHandle hPreparedGpuMorphTargets) override;

    eResourceTransferResult DownloadBufferData(GpuBufferHandle hGpuBuffer,
                                               void* pDstData,
                                               int32_t size,
                                               int32_t offset = 0) override;

    eResourceTransferResult DownloadTexture(GpuTextureHandle hGpuTexture,
                                            ObjectHandle<RTextureAsset> hDstTexture) override;

    eResourceTransferResult UnloadBuffer(GpuBufferHandle hGpuBuffer) override;
    eResourceTransferResult UnloadStaticMesh(GpuStaticMeshHandle hGpuStaticMesh) override;
    eResourceTransferResult UnloadTexture(GpuTextureHandle hGpuTexture) override;
    eResourceTransferResult UnloadShader(GpuShaderHandle hGpuShader) override;
    eResourceTransferResult UnloadMorphTargets(GpuMorphTargetsHandle hGpuMorphTargets) override;

  private:
    bool initializeSamplers() override;

    static GLint sAlignmentUBO;
    static GLint sAlignmentSSBO;

    GLuint mGlVAO = 0;
    static const GLuint mAttribBindingIndex = 0;
    static const GLuint mPosAttribIndex = 0;
    static const GLuint mNormalAttribIndex = 1;
    static const GLuint mTangentsAttribIndex = 2;
    static const GLuint mUV0AttribIndex = 3;
    static const GLuint mUV1AttribIndex = 4;
    static const GLuint mColorAttribIndex = 5;

    GLuint mGlProgramPipeline = 0;

    GLuint mMaterialTextureSampler = 0;
    GLuint mDirLightShadowMapSampler = 0;
    GLuint mPointLightShadowMapSampler = 0;
    GLuint mSpotLightShadowMapSampler = 0;
    GLuint mIBLSampler = 0;

    std::unordered_map<FrameBufferHandle, FrameBufferNativeHandleGL> mFrameBufferToNativeHandleMap;

    TypedObjectPool<BufferNativeHandleGL> mBufferNativeHandlePool;
    TypedObjectPool<StaticMeshNativeHandleGL> mStaticMeshNativeHandlePool;
    TypedObjectPool<TextureNativeHandleGL> mTextureNativeHandlePool;
    TypedObjectPool<ShaderNativeHandleGL> mShaderNativeHandlePool;
    TypedObjectPool<MorphTargetsNativeHandleGL> mMorphTargetsNativeHandlePool;
};
} // namespace ho