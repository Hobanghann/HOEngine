#pragma once

#include <glad/include/glad/glad.h>

#include "../IRenderingSystem.h"
#include "NativeHandlesGL.h"

namespace ho
{
class IRenderingSystem;

class RenderingSystemGL final : public IRenderingSystem
{
    friend void IRenderingSystem::createInstance(eGraphicsAPI api);

  public:
    void* GetRenderTargetNativeHandle(StringHandle hFrameBufferName,
                                      eRenderTargetType type,
                                      bool bRequireMultisample = false) const override;

    void SetVSync(bool bEnabled) override;

  private:
    struct GlFBO
    {
        GLuint FBO[2] = {};
        GLuint ResolvedFBO[2] = {};
    };

    RenderingSystemGL();
    ~RenderingSystemGL() override = default;

    bool init() override;
    void releaseAllResources() override;

    void applyMaterial(GpuMaterialHandle hGpuMaterial) override;
    void executeDrawCommand(const DrawCommand& command) override;

    void executeSkyMapPass(const GpuTexture* pSkyMap) override;
    void executeInWorldDebugDrawPass() override;
    void executeOverlayDebugDrawPass() override;

    void prepareExecuteRenderQueue() override;
    void prepareExecuteRenderPass(const RenderPass& pass) override;
    void prepareExecuteOpaqueDrawCommands() override;
    void prepareExecuteMaskedDrawCommands() override;
    void prepareExecuteBlendedDrawCommands() override;
    void finishExecuteRenderPass(const RenderPass& pass) override;
    void finishExecuteRenderQueue() override;

    void resolveFrameBuffer(const FrameBuffer& frameBuffer) override;

    void renderUI() override;

    void flush() override;

    bool createFrameBuffer(const FrameBufferDesc& frameBufferDesc) override;
    bool destroyFrameBuffer(StringHandle hFrameBufferName) override;

    eResourceTransferResult uploadStaticMesh(GpuStaticMeshHandle hPreparedGpuStaticMesh,
                                             uint64_t currentFrame) override;
    eResourceTransferResult uploadMaterial(GpuMaterialHandle hPreparedGpuMaterial, uint64_t currentFrame) override;
    eResourceTransferResult uploadTexture(GpuTextureHandle hPreparedGpuTexture, uint64_t currentFrame) override;
    eResourceTransferResult uploadShader(GpuShaderHandle hPreparedGpuShader, uint64_t currentFrame) override;

    eResourceTransferResult unloadStaticMesh(GpuStaticMeshHandle hGpuStaticMesh) override;
    eResourceTransferResult unloadMaterial(GpuMaterialHandle hGpuMaterial) override;
    eResourceTransferResult unloadTexture(GpuTextureHandle hGpuTexture) override;
    eResourceTransferResult unloadShader(GpuShaderHandle hGpuShader) override;

    void cancelInitialization();

    void cancelFrameBufferCreation(GLuint* glFBO,
                                   GLuint* glResolvedFBO,
                                   GLuint (*pGlRenderTargets)[static_cast<int32_t>(eRenderTargetType::Last)],
                                   GLuint (*pGlResolvedRenderTargets)[static_cast<int32_t>(eRenderTargetType::Last)]);

    std::unordered_map<StringHandle, GlFBO> mNameToGlFboMap;

    GLuint mGlVAO = 0;
    GLuint mAttribBindingIndex = 0;
    GLuint mPosAttribIndex = 0;
    GLuint mNormalAttribIndex = 1;
    GLuint mTangentsAttribIndex = 2;
    GLuint mUV0AttribIndex = 3;
    GLuint mUV1AttribIndex = 4;
    GLuint mColorAttribIndex = 5;

    GLuint mGlProgramPipeline = 0;
    GLuint mGlDefaultVS = 0;
    GLuint mGlDefaultPhongFS = 0;
    GLuint mGlDefaultPhongMaskedFS = 0;
    GLuint mGlDefaultPbrFS = 0;
    GLuint mGlDefaultPbrMaskedFS = 0;
    GLuint mGlDefaultSkyMapVS = 0;
    GLuint mGlDefaultSkyMapFS = 0;
    GLuint mGlDefaultWorldGizmoVS = 0;
    GLuint mGlDefaultWorldGizmoFS = 0;

    GLuint mGlDefaultCubeMap = 0;

    GLuint mGlMatrixUBO = 0;
    GLuint mGlLightUBO = 0;
    GLuint mGlGeneralPurposeUBO = 0;

    GLuint mMaterialUboIndex = 0;
    GLuint mMatrixUboIndex = 1;
    GLuint mLightUboIndex = 2;
    GLuint mGeneralPurposeUboIndex = 3;

    ObjectPool<StaticMeshNativeHandleGL> mStaticMeshNativeHandlePool;
    ObjectPool<MaterialNativeHandleGL> mMaterialNativeHandlePool;
    ObjectPool<TextureNativeHandleGL> mTextureNativeHandlePool;
    ObjectPool<ShaderNativeHandleGL> mShaderNativeHandlePool;
};
} // namespace ho