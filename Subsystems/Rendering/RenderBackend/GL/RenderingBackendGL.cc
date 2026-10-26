#include "RenderingBackendGL.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include "../../RenderData/DrawCommand.h"
#include "../../RenderData/FrameBufferDesc.h"
#include "../../RenderData/UIViewportData.h"
#include "Platforms/Windows/GL/Win32ApplicationGL.h"
#include "Subsystems/Asset/Assets.h"
#include "Subsystems/Asset/TextureUtils.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
static GLenum toGlFunc(IRenderingBackend::eComparisonFunc func);
static GLenum toGlStencilOp(IRenderingBackend::eStencilOperator op);
[[maybe_unused]] static bool isStorableTextureFormat(eTextureFormat format);
static GLenum toGlInternalFormat(eTextureFormat format);
static GLenum toGlExternalFormat(eTextureFormat format);
static GLenum toGlExternalType(eTextureFormat format);
static GLenum toGlTextureType(eTextureType type, int32_t sampleCount);
static GLenum toGlShaderType(eShaderStage stage);
[[maybe_unused]] static GLbitfield toGlShaderBitfield(eShaderStage stage);
static GLenum toGlAccessType(IRenderingBackend::eResourceAccessType type);
static bool ASSERT_GL();

struct Vertex
{
    Vector3 Position;        // 16 byte
    Vector3 Normal;          // 16 byte
    Vector4 Tangent;         // 16 byte
    Vector2 UV0;             // 4 byte
    Vector2 UV1;             // 4 byte
    uint8_t Color0[4] = {};  // 4 byte
    uint8_t Padding[4] = {}; // 4 byte
}; // 64 byte

struct MorphTarget
{
    int32_t PositionOffset = 0;
    int32_t NormalOffset = 0;
    int32_t TangentOffset = 0;
};

RenderingBackendGL::RenderingBackendGL() {}

RenderingBackendGL::~RenderingBackendGL()
{
    HO_ASSERT(!glIsVertexArray(mGlVAO), "OpenGL VAO was not deleted. Call glDeleteVertexArrays before destruction.");
    HO_ASSERT(!glIsProgramPipeline(mGlProgramPipeline),
              "OpenGL Program Pipeline was not deleted. Call glDeleteProgramPipelines before destruction.");

    HO_ASSERT(mFrameBufferToNativeHandleMap.empty(),
              "GPU FrameBuffers leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mBufferNativeHandlePool.IsEmpty(),
              "GPU Buffers leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mStaticMeshNativeHandlePool.IsEmpty(),
              "GPU StaticMeshes leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mTextureNativeHandlePool.IsEmpty(),
              "GPU Textures leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mShaderNativeHandlePool.IsEmpty(),
              "GPU Shaders leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mMorphTargetsNativeHandlePool.IsEmpty(),
              "GPU MorphTargets leaked. Release all resources before destroying the backend.");
}

bool RenderingBackendGL::Init()
{
    IRenderingBackend::Init();

    // Get alignment
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &sAlignmentUBO);
    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &sAlignmentSSBO);

    // Create default GPU resources.

    // VAO
    glCreateVertexArrays(1, &mGlVAO);

    glVertexArrayAttribFormat(mGlVAO, mPosAttribIndex, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(mGlVAO, mNormalAttribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3));
    glVertexArrayAttribFormat(mGlVAO, mTangentsAttribIndex, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector3));
    glVertexArrayAttribFormat(mGlVAO, mUV0AttribIndex, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector3) + sizeof(Vector4));
    glVertexArrayAttribFormat(
        mGlVAO, mUV1AttribIndex, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector3) + sizeof(Vector4) + sizeof(Vector2));
    glVertexArrayAttribFormat(mGlVAO,
                              mColorAttribIndex,
                              4,
                              GL_UNSIGNED_BYTE,
                              GL_TRUE,
                              2 * sizeof(Vector3) + sizeof(Vector4) + 2 * sizeof(Vector2));

    glVertexArrayAttribBinding(mGlVAO, mPosAttribIndex, mAttribBindingIndex);
    glEnableVertexArrayAttrib(mGlVAO, mPosAttribIndex);
    glVertexArrayAttribBinding(mGlVAO, mNormalAttribIndex, mAttribBindingIndex);
    glEnableVertexArrayAttrib(mGlVAO, mNormalAttribIndex);
    glVertexArrayAttribBinding(mGlVAO, mTangentsAttribIndex, mAttribBindingIndex);
    glEnableVertexArrayAttrib(mGlVAO, mTangentsAttribIndex);
    glVertexArrayAttribBinding(mGlVAO, mUV0AttribIndex, mAttribBindingIndex);
    glEnableVertexArrayAttrib(mGlVAO, mUV0AttribIndex);
    glVertexArrayAttribBinding(mGlVAO, mUV1AttribIndex, mAttribBindingIndex);
    glEnableVertexArrayAttrib(mGlVAO, mUV1AttribIndex);
    glVertexArrayAttribBinding(mGlVAO, mColorAttribIndex, mAttribBindingIndex);
    glEnableVertexArrayAttrib(mGlVAO, mColorAttribIndex);

    if (!ASSERT_GL())
    {
        glDeleteVertexArrays(1, &mGlVAO);
        return false;
    }

    // ProgramPipeline
    glCreateProgramPipelines(1, &mGlProgramPipeline);

    if (!ASSERT_GL())
    {
        glDeleteVertexArrays(1, &mGlVAO);
        return false;
    }

    if (!initializeSamplers())
    {
        glDeleteVertexArrays(1, &mGlVAO);
        glDeleteProgramPipelines(1, &mGlProgramPipeline);
    }

    return true;
}

void RenderingBackendGL::Shutdown()
{
    UnloadAllResources();

    Flush();

    // Unload all GPU resources
    for (auto& nativeHandle : mBufferNativeHandlePool)
    {
        glDeleteBuffers(1, &nativeHandle.GlBuffer);
    }
    for (auto& nativeHandle : mStaticMeshNativeHandlePool)
    {
        glDeleteBuffers(1, &nativeHandle.GlVBO);
        glDeleteBuffers(1, &nativeHandle.GlEBO);
    }
    for (auto& nativeHandle : mTextureNativeHandlePool)
    {
        glDeleteTextures(1, &nativeHandle.GlTexture);
    }
    for (auto& nativeHandle : mShaderNativeHandlePool)
    {
        glDeleteProgram(nativeHandle.GlProgram);
    }
    for (auto& nativeHandle : mMorphTargetsNativeHandlePool)
    {
        glDeleteBuffers(1, &nativeHandle.GlMorphTargetsSSBO);
        glDeleteBuffers(1, &nativeHandle.GlTargetAttributesSSBO);
    }

    for (auto& it : mFrameBufferToNativeHandleMap)
    {
        glDeleteFramebuffers(1, &it.second.GlFBO);
        for (const GLuint glColorTargetView : it.second.GlColorTargetViews)
        {
            glDeleteTextures(1, &glColorTargetView);
        }
        glDeleteTextures(1, &it.second.GlDepthTargetView);
    }

    mFrameBufferToNativeHandleMap.clear();

    glDeleteVertexArrays(1, &mGlVAO);
    glDeleteProgramPipelines(1, &mGlProgramPipeline);
    glDeleteSamplers(1, &mMaterialTextureSampler);
    glDeleteSamplers(1, &mDirLightShadowMapSampler);
    glDeleteSamplers(1, &mPointLightShadowMapSampler);
    glDeleteSamplers(1, &mSpotLightShadowMapSampler);
    glDeleteSamplers(1, &mIBLSampler);

    ASSERT_GL();

    mBufferNativeHandlePool.Clear();
    mStaticMeshNativeHandlePool.Clear();
    mTextureNativeHandlePool.Clear();
    mShaderNativeHandlePool.Clear();
    mMorphTargetsNativeHandlePool.Clear();
}

void RenderingBackendGL::BeginDrawSequence()
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    glUseProgram(0);

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glFrontFace(GL_CW);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glBindVertexArray(mGlVAO);
    glBindProgramPipeline(mGlProgramPipeline);

    for (int32_t slot = static_cast<int32_t>(eTextureBindingIndex::Diffuse);
         slot <= static_cast<int32_t>(eTextureBindingIndex::AmbientOcclusion);
         ++slot)
    {
        glBindSampler(slot, mMaterialTextureSampler);
    }

    for (int32_t slot = static_cast<int32_t>(eTextureBindingIndex::RadianceMap);
         slot <= static_cast<int32_t>(eTextureBindingIndex::PrefilteredEnvironmentMap);
         ++slot)
    {
        glBindSampler(slot, mIBLSampler);
    }

    glBindSampler(static_cast<int32_t>(eTextureBindingIndex::DirLightDynamicShadowMap), mDirLightShadowMapSampler);
    glBindSampler(static_cast<int32_t>(eTextureBindingIndex::PointLightDynamicShadowMap), mPointLightShadowMapSampler);
    glBindSampler(static_cast<int32_t>(eTextureBindingIndex::SpotLightDynamicShadowMap), mSpotLightShadowMapSampler);

    for (int32_t slot = static_cast<int32_t>(eTextureBindingIndex::DirLightStaticShadowMap0);
         slot <= static_cast<int32_t>(eTextureBindingIndex::PointLightStaticShadowMap0);
         ++slot)
    {
        glBindSampler(slot, mDirLightShadowMapSampler);
    }
    for (int32_t slot = static_cast<int32_t>(eTextureBindingIndex::PointLightStaticShadowMap0);
         slot <= static_cast<int32_t>(eTextureBindingIndex::SpotLightStaticShadowMap0);
         ++slot)
    {
        glBindSampler(slot, mPointLightShadowMapSampler);
    }
    for (int32_t slot = static_cast<int32_t>(eTextureBindingIndex::SpotLightStaticShadowMap0);
         slot <= static_cast<int32_t>(eTextureBindingIndex::Last);
         ++slot)
    {
        glBindSampler(slot, mSpotLightShadowMapSampler);
    }

    ASSERT_GL();
}

void RenderingBackendGL::EndDrawSequence()
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    BindFrameBuffer(FrameBufferHandle::sNULL);

    ASSERT_GL();
}

void RenderingBackendGL::SetAlphaBlendMode(eAlphaBlendMode mode)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.AlphaBlendMode == mode)
    {
        return;
    }

    switch (mode)
    {
        case eAlphaBlendMode::None:
            glDisable(GL_BLEND);
            break;
        case eAlphaBlendMode::Default:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case eAlphaBlendMode::Additive:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        default:
            HO_ASSERT(false, "Invalid alpha blend mode.");
    }

    mCurrentState.AlphaBlendMode = mode;

    ASSERT_GL();
}

void RenderingBackendGL::SetPolygonMode(ePolygonMode mode)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.PolygonMode == mode)
    {
        return;
    }

    switch (mode)
    {
        case ePolygonMode::Point:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        case ePolygonMode::Line:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case ePolygonMode::Triangle:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        default:
            HO_ASSERT(false, "Invalid polygon mode.");
    }

    mCurrentState.PolygonMode = mode;

    ASSERT_GL();
}

void RenderingBackendGL::SetCullMode(eCullMode mode)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.CullMode == mode)
    {
        return;
    }

    switch (mode)
    {
        case eCullMode::None:
            glDisable(GL_CULL_FACE);
            break;
        case eCullMode::Front:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case eCullMode::Back:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        default:
            HO_ASSERT(false, "Invalid cull mode.");
    }

    mCurrentState.CullMode = mode;

    ASSERT_GL();
}

void RenderingBackendGL::SetDepthTest(bool bEnabled)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.bDepthTestEnabled == bEnabled)
    {
        return;
    }

    if (bEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    mCurrentState.bDepthTestEnabled = bEnabled;

    ASSERT_GL();
}

void RenderingBackendGL::SetDepthWrite(bool bEnabled)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.bDepthWriteEnabled == bEnabled)
    {
        return;
    }

    glDepthMask(bEnabled ? GL_TRUE : GL_FALSE);

    mCurrentState.bDepthWriteEnabled = bEnabled;

    ASSERT_GL();
}

void RenderingBackendGL::SetDepthFunc(eComparisonFunc func)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.DepthFunc == func)
    {
        return;
    }

    glDepthFunc(toGlFunc(func));

    mCurrentState.DepthFunc = func;

    ASSERT_GL();
}

void RenderingBackendGL::SetStencilTest(bool bEnabled)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.bStencilTestEnabled == bEnabled)
    {
        return;
    }

    if (bEnabled)
    {
        glEnable(GL_STENCIL_TEST);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }

    mCurrentState.bStencilTestEnabled = bEnabled;

    ASSERT_GL();
}

void RenderingBackendGL::SetStencilWriteMask(uint8_t writeMask)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.StencilWriteMask == writeMask)
    {
        return;
    }

    glStencilMask(writeMask);

    mCurrentState.StencilWriteMask = writeMask;

    ASSERT_GL();
}

void RenderingBackendGL::SetStencilState(eComparisonFunc func, uint8_t compareMask)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.StencilFunc == func && mCurrentState.StencilCompareMask == compareMask)
    {
        return;
    }

    glStencilFunc(toGlFunc(func), mCurrentState.StencilReference, compareMask);

    mCurrentState.StencilFunc = func;
    mCurrentState.StencilCompareMask = compareMask;

    ASSERT_GL();
}

void RenderingBackendGL::SetStencilReference(uint8_t ref)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.StencilReference == ref)
    {
        return;
    }

    glStencilFunc(toGlFunc(mCurrentState.StencilFunc), ref, mCurrentState.StencilCompareMask);

    mCurrentState.StencilReference = ref;

    ASSERT_GL();
}

void RenderingBackendGL::SetStencilOperator(eStencilOperator stencilFailOp,
                                            eStencilOperator depthFailOp,
                                            eStencilOperator depthPassOp)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.StencilFailOp == stencilFailOp && mCurrentState.DepthFailOp == depthPassOp &&
        mCurrentState.DepthPassOp == depthPassOp)
    {
        return;
    }

    glStencilOp(toGlStencilOp(stencilFailOp), toGlStencilOp(depthFailOp), toGlStencilOp(depthPassOp));

    mCurrentState.StencilFailOp = stencilFailOp;
    mCurrentState.DepthFailOp = depthPassOp;
    mCurrentState.DepthPassOp = depthPassOp;

    ASSERT_GL();
}

void RenderingBackendGL::BindFrameBuffer(FrameBufferHandle hFrameBuffer)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (mCurrentState.hBoundFrameBuffer == hFrameBuffer)
    {
        return;
    }

    if (mCurrentState.hBoundFrameBuffer.IsValid())
    {
        for (const FrameBuffer::TargetBindingState& colorTargetState :
             mCurrentState.hBoundFrameBuffer->ColorTargetStates)
        {
            if (colorTargetState.hTarget.IsValid())
            {
                colorTargetState.hTarget->LastUsedFrame = mCurrentState.CurrentFrame;
            }
        }
        if (mCurrentState.hBoundFrameBuffer->DepthTargetState.hTarget.IsValid())
        {
            mCurrentState.hBoundFrameBuffer->DepthTargetState.hTarget->LastUsedFrame = mCurrentState.CurrentFrame;
        }
    }

    if (hFrameBuffer.IsNULL())
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        mCurrentState.hBoundFrameBuffer = FrameBufferHandle::sNULL;
    }
    else
    {
        auto it = mFrameBufferToNativeHandleMap.find(hFrameBuffer);
        HO_ASSERT(it != mFrameBufferToNativeHandleMap.end(),
                  "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

        glBindFramebuffer(GL_FRAMEBUFFER, it->second.GlFBO);
        mCurrentState.hBoundFrameBuffer = hFrameBuffer;
    }

    ASSERT_GL();
}

void RenderingBackendGL::AttachColorTargetRange(FrameBufferHandle hFrameBuffer,
                                                eRenderTargetBindingIndex targetIndex,
                                                GpuTextureHandle hTarget,
                                                int32_t baseSlice,
                                                int32_t sliceCount,
                                                int32_t mipLevel)
{
    const int32_t targetIdxVal = static_cast<int32_t>(targetIndex);

    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(targetIdxVal >= 0 && targetIdxVal < FrameBufferDesc::sMaxColorTargetCount, "Target index out of range.");

    if (hFrameBuffer.IsNULL())
    {
        return;
    }

    auto it = mFrameBufferToNativeHandleMap.find(hFrameBuffer);
    HO_ASSERT(it != mFrameBufferToNativeHandleMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

    HO_ASSERT(hTarget.IsValid(), "Invalid Texture handle.");
    HO_ASSERT(hTarget->State != eGpuResourceState::PendingUpload, "Texture is not uploaded yet.");

    HO_ASSERT(hTarget->Format >= eTextureFormat::R8_UNORM && hTarget->Format <= eTextureFormat::R32G32B32A32_FLOAT,
              "Invalid format for color render target.");

    HO_ASSERT(mipLevel >= 0 && mipLevel < hTarget->MipLevels, "mipLevel out of range.");
    HO_ASSERT(baseSlice >= 0, "baseSlice must be non-negative.");
    HO_ASSERT(sliceCount > 0, "sliceCount must be greater than 0.");

    // NOLINTBEGIN(bugprone-branch-clone)
    switch (hTarget->Type)
    {
        case eTextureType::Texture1D:
        case eTextureType::Texture2D:
            HO_ASSERT(baseSlice == 0 && sliceCount == 1, "1D/2D Texture baseSlice must be 0 and sliceCount must be 1.");
            break;
        case eTextureType::Texture1DArray:
        case eTextureType::Texture2DArray:
            HO_ASSERT(baseSlice + sliceCount <= hTarget->ArraySize, "Slice range out of ArraySize range.");
            break;
        case eTextureType::Texture3D:
            HO_ASSERT(baseSlice + sliceCount <= hTarget->Depth, "Slice range out of Depth range.");
            break;
        case eTextureType::TextureCubeMap:
            HO_ASSERT(baseSlice + sliceCount <= 6, "Cubemap slice range out of range (must be <= 6).");
            break;
        case eTextureType::TextureCubeMapArray:
            HO_ASSERT(baseSlice + sliceCount <= (hTarget->ArraySize * 6), "CubemapArray slice range out of range.");
            break;
        default:
            HO_ASSERT(false, "Unsupported texture type for color target.");
            break;
    }
    // NOLINTEND(bugprone-branch-clone)

    HO_ASSERT(math::Max(1, hTarget->Width >> static_cast<uint32_t>(mipLevel)) == hFrameBuffer->Width &&
                  math::Max(1, hTarget->Height >> static_cast<uint32_t>(mipLevel)) == hFrameBuffer->Height,
              "Texture Mip resolution does not match FrameBuffer resolution.");

    if (hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget == hTarget &&
        hFrameBuffer->ColorTargetStates[targetIdxVal].BaseSlice == baseSlice &&
        hFrameBuffer->ColorTargetStates[targetIdxVal].SliceCount == sliceCount &&
        hFrameBuffer->ColorTargetStates[targetIdxVal].MipLevel == mipLevel)
    {
        return;
    }

    if (hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget.IsValid())
    {
        --(hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget->FrameBufferAttachmentCount);
        if (hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget->FrameBufferAttachmentCount == 0)
        {
            hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget->LastUsedFrame = mCurrentState.CurrentFrame;
        }
    }

    hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget = hTarget;
    ++(hTarget->FrameBufferAttachmentCount);
    hFrameBuffer->ColorTargetStates[targetIdxVal].BaseSlice = baseSlice;
    hFrameBuffer->ColorTargetStates[targetIdxVal].SliceCount = sliceCount;
    hFrameBuffer->ColorTargetStates[targetIdxVal].MipLevel = mipLevel;

    const GLuint glTarget = mTextureNativeHandlePool.Get(hTarget->NativeHandlePoolID)->GlTexture;

    if (sliceCount == 1)
    {
        if (hTarget->Type == eTextureType::Texture1D || hTarget->Type == eTextureType::Texture2D)
        {
            glNamedFramebufferTexture(it->second.GlFBO, GL_COLOR_ATTACHMENT0 + targetIdxVal, glTarget, mipLevel);
        }
        else
        {
            glNamedFramebufferTextureLayer(
                it->second.GlFBO, GL_COLOR_ATTACHMENT0 + targetIdxVal, glTarget, mipLevel, baseSlice);
        }
    }
    else
    {
        HO_ASSERT(hTarget->Type != eTextureType::Texture3D,
                  "OpenGL specification does not support glTextureView for Texture3D.");

        GLenum viewTarget = GL_NONE;
        switch (hTarget->Type)
        {
            case eTextureType::Texture1DArray:
                viewTarget = GL_TEXTURE_1D_ARRAY;
                break;

            case eTextureType::Texture2DArray:
                viewTarget = GL_TEXTURE_2D_ARRAY;
                break;

            case eTextureType::TextureCubeMap:
                viewTarget = (sliceCount == 6 && baseSlice % 6 == 0) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D_ARRAY;
                break;

            case eTextureType::TextureCubeMapArray:
                if (baseSlice % 6 == 0)
                {
                    if (sliceCount == 6)
                    {
                        viewTarget = GL_TEXTURE_CUBE_MAP;
                    }
                    else if (sliceCount % 6 == 0)
                    {
                        viewTarget = GL_TEXTURE_CUBE_MAP_ARRAY;
                    }
                    else
                    {
                        viewTarget = GL_TEXTURE_2D_ARRAY;
                    }
                }
                else
                {
                    viewTarget = GL_TEXTURE_2D_ARRAY;
                }
                break;

            default:
                HO_ASSERT(false, "Unsupported texture type for texture view attachment.");
                return;
        }

        const GLenum glInternalFormat = toGlInternalFormat(hTarget->Format);

        GLuint& glViewTex = it->second.GlColorTargetViews[targetIdxVal];
        if (glViewTex != 0)
        {
            glDeleteTextures(1, &glViewTex);
            glViewTex = 0;
        }

        glGenTextures(1, &glViewTex);
        glTextureView(glViewTex, viewTarget, glTarget, glInternalFormat, mipLevel, 1, baseSlice, sliceCount);

        glNamedFramebufferTexture(it->second.GlFBO, GL_COLOR_ATTACHMENT0 + targetIdxVal, glViewTex, 0);
    }

    ASSERT_GL();

    GLenum drawBuffers[FrameBufferDesc::sMaxColorTargetCount] = {};
    int32_t maxActiveSlot = -1;

    for (int32_t i = 0; i < FrameBufferDesc::sMaxColorTargetCount; ++i)
    {
        if (hFrameBuffer->ColorTargetStates[i].hTarget.IsValid())
        {
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
            maxActiveSlot = i;
        }
        else
        {
            drawBuffers[i] = GL_NONE;
        }
    }

    if (maxActiveSlot >= 0)
    {
        glNamedFramebufferDrawBuffers(it->second.GlFBO, maxActiveSlot + 1, drawBuffers);
    }
    else
    {
        const GLenum noneTarget = GL_NONE;
        glNamedFramebufferDrawBuffers(it->second.GlFBO, 1, &noneTarget);
    }

    ASSERT_GL();
}

void RenderingBackendGL::AttachDepthTargetRange(
    FrameBufferHandle hFrameBuffer, GpuTextureHandle hTarget, int32_t baseSlice, int32_t sliceCount, int32_t mipLevel)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (hFrameBuffer.IsNULL())
    {
        return;
    }

    auto it = mFrameBufferToNativeHandleMap.find(hFrameBuffer);
    HO_ASSERT(it != mFrameBufferToNativeHandleMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

    HO_ASSERT(hTarget.IsValid(), "Invalid Texture handle.");
    HO_ASSERT(hTarget->State != eGpuResourceState::PendingUpload, "Texture is not uploaded yet.");

    const eTextureFormat format = hTarget->Format;
    HO_ASSERT(format == eTextureFormat::D24_UNORM_S8_UINT || format == eTextureFormat::D32_FLOAT,
              "Invalid format for depth render target.");

    HO_ASSERT(mipLevel >= 0 && mipLevel < hTarget->MipLevels, "mipLevel out of range.");
    HO_ASSERT(baseSlice >= 0, "baseSlice must be non-negative.");
    HO_ASSERT(sliceCount > 0, "sliceCount must be greater than 0.");

    // NOLINTBEGIN(bugprone-branch-clone)
    switch (hTarget->Type)
    {
        case eTextureType::Texture1D:
        case eTextureType::Texture2D:
            HO_ASSERT(baseSlice == 0 && sliceCount == 1, "1D/2D Texture baseSlice must be 0 and sliceCount must be 1.");
            break;
        case eTextureType::Texture1DArray:
        case eTextureType::Texture2DArray:
            HO_ASSERT(baseSlice + sliceCount <= hTarget->ArraySize, "Slice range out of ArraySize range.");
            break;
        case eTextureType::Texture3D:
            HO_ASSERT(baseSlice + sliceCount <= hTarget->Depth, "Slice range out of Depth range.");
            break;
        case eTextureType::TextureCubeMap:
            HO_ASSERT(baseSlice + sliceCount <= 6, "Cubemap slice range out of range (must be <= 6).");
            break;
        case eTextureType::TextureCubeMapArray:
            HO_ASSERT(baseSlice + sliceCount <= (hTarget->ArraySize * 6), "CubemapArray slice range out of range.");
            break;
        default:
            HO_ASSERT(false, "Unsupported texture type for depth target.");
            break;
    }
    // NOLINTEND(bugprone-branch-clone)

    HO_ASSERT(math::Max(1, hTarget->Width >> mipLevel) == hFrameBuffer->Width &&
                  math::Max(1, hTarget->Height >> mipLevel) == hFrameBuffer->Height,
              "Texture Mip resolution does not match FrameBuffer resolution.");

    if (hFrameBuffer->DepthTargetState.hTarget == hTarget && hFrameBuffer->DepthTargetState.BaseSlice == baseSlice &&
        hFrameBuffer->DepthTargetState.SliceCount == sliceCount && hFrameBuffer->DepthTargetState.MipLevel == mipLevel)
    {
        return;
    }

    if (hFrameBuffer->DepthTargetState.hTarget.IsValid())
    {
        --(hFrameBuffer->DepthTargetState.hTarget->FrameBufferAttachmentCount);
        if (hFrameBuffer->DepthTargetState.hTarget->FrameBufferAttachmentCount == 0)
        {
            hFrameBuffer->DepthTargetState.hTarget->LastUsedFrame = mCurrentState.CurrentFrame;
        }
    }

    hFrameBuffer->DepthTargetState.hTarget = hTarget;
    ++(hTarget->FrameBufferAttachmentCount);
    hFrameBuffer->DepthTargetState.BaseSlice = baseSlice;
    hFrameBuffer->DepthTargetState.SliceCount = sliceCount;
    hFrameBuffer->DepthTargetState.MipLevel = mipLevel;

    const GLenum attachment =
        (format == eTextureFormat::D24_UNORM_S8_UINT) ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

    const GLuint glTarget = mTextureNativeHandlePool.Get(hTarget->NativeHandlePoolID)->GlTexture;

    if (sliceCount == 1)
    {
        if (hTarget->Type == eTextureType::Texture1D || hTarget->Type == eTextureType::Texture2D)
        {
            glNamedFramebufferTexture(it->second.GlFBO, attachment, glTarget, mipLevel);
        }
        else
        {
            glNamedFramebufferTextureLayer(it->second.GlFBO, attachment, glTarget, mipLevel, baseSlice);
        }
    }
    else
    {
        HO_ASSERT(hTarget->Type != eTextureType::Texture3D,
                  "OpenGL specification does not support glTextureView for Texture3D.");

        GLenum viewTarget = GL_NONE;
        switch (hTarget->Type)
        {
            case eTextureType::Texture1DArray:
                viewTarget = GL_TEXTURE_1D_ARRAY;
                break;

            case eTextureType::Texture2DArray:
                viewTarget = GL_TEXTURE_2D_ARRAY;
                break;

            case eTextureType::TextureCubeMap:
                viewTarget = (sliceCount == 6 && baseSlice % 6 == 0) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D_ARRAY;
                break;

            case eTextureType::TextureCubeMapArray:
                if (baseSlice % 6 == 0)
                {
                    if (sliceCount == 6)
                    {
                        viewTarget = GL_TEXTURE_CUBE_MAP;
                    }
                    else if (sliceCount % 6 == 0)
                    {
                        viewTarget = GL_TEXTURE_CUBE_MAP_ARRAY;
                    }
                    else
                    {
                        viewTarget = GL_TEXTURE_2D_ARRAY;
                    }
                }
                else
                {
                    viewTarget = GL_TEXTURE_2D_ARRAY;
                }
                break;

            default:
                HO_ASSERT(false, "Unsupported texture type for texture view attachment.");
                return;
        }

        const GLenum glInternalFormat = toGlInternalFormat(hTarget->Format);

        GLuint& glViewTex = it->second.GlDepthTargetView;
        if (glViewTex != 0)
        {
            glDeleteTextures(1, &glViewTex);
            glViewTex = 0;
        }

        glGenTextures(1, &glViewTex);
        glTextureView(glViewTex, viewTarget, glTarget, glInternalFormat, mipLevel, 1, baseSlice, sliceCount);

        glNamedFramebufferTexture(it->second.GlFBO, attachment, glViewTex, mipLevel);
    }

    ASSERT_GL();
}

void RenderingBackendGL::DetachColorTarget(FrameBufferHandle hFrameBuffer, eRenderTargetBindingIndex targetIndex)
{
    const int32_t targetIdxVal = static_cast<int32_t>(targetIndex);

    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(targetIdxVal >= 0 && targetIdxVal < FrameBufferDesc::sMaxColorTargetCount, "Target index out of range.");

    if (hFrameBuffer.IsNULL())
    {
        return;
    }

    auto it = mFrameBufferToNativeHandleMap.find(hFrameBuffer);
    HO_ASSERT(it != mFrameBufferToNativeHandleMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

    HO_ASSERT(hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget.IsValid(),
              "Frame buffer dosen't have sush color target.");
    --(hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget->FrameBufferAttachmentCount);

    // Update detached target's last used frame count when texture totally detached.
    if (hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget->FrameBufferAttachmentCount == 0)
    {
        hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget = GpuTextureHandle::sNULL;
    hFrameBuffer->ColorTargetStates[targetIdxVal].BaseSlice = 0;
    hFrameBuffer->ColorTargetStates[targetIdxVal].SliceCount = 0;
    hFrameBuffer->ColorTargetStates[targetIdxVal].MipLevel = 0;

    glNamedFramebufferTexture(it->second.GlFBO, GL_COLOR_ATTACHMENT0 + targetIdxVal, 0, 0);

    GLuint& glViewTex = it->second.GlColorTargetViews[targetIdxVal];
    if (glViewTex != 0)
    {
        glDeleteTextures(1, &glViewTex);
        glViewTex = 0;
    }

    ASSERT_GL();

    GLenum drawBuffers[FrameBufferDesc::sMaxColorTargetCount] = {};
    int32_t maxActiveSlot = -1;

    for (int32_t i = 0; i < FrameBufferDesc::sMaxColorTargetCount; ++i)
    {
        if (!hFrameBuffer->ColorTargetStates[i].hTarget.IsNULL())
        {
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
            maxActiveSlot = i;
        }
        else
        {
            drawBuffers[i] = GL_NONE;
        }
    }

    if (maxActiveSlot >= 0)
    {
        glNamedFramebufferDrawBuffers(it->second.GlFBO, maxActiveSlot + 1, drawBuffers);
    }
    else
    {
        const GLenum noneTarget = GL_NONE;
        glNamedFramebufferDrawBuffers(it->second.GlFBO, 1, &noneTarget);
    }

    ASSERT_GL();
}

void RenderingBackendGL::DetachDepthTarget(FrameBufferHandle hFrameBuffer)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (hFrameBuffer.IsNULL())
    {
        return;
    }

    auto it = mFrameBufferToNativeHandleMap.find(hFrameBuffer);
    HO_ASSERT(it != mFrameBufferToNativeHandleMap.end(), "FBO not found.");

    HO_ASSERT(hFrameBuffer->DepthTargetState.hTarget.IsValid(), "Frame buffer dosen't have sush depth target.");
    --(hFrameBuffer->DepthTargetState.hTarget->FrameBufferAttachmentCount);

    // Update detached target's last used frame count when texture totally detached.
    if (hFrameBuffer->DepthTargetState.hTarget->FrameBufferAttachmentCount == 0)
    {
        hFrameBuffer->DepthTargetState.hTarget->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    hFrameBuffer->DepthTargetState.hTarget = GpuTextureHandle::sNULL;
    glNamedFramebufferTexture(it->second.GlFBO, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);

    GLuint& glViewTex = it->second.GlDepthTargetView;
    if (glViewTex != 0)
    {
        glDeleteTextures(1, &glViewTex);
        glViewTex = 0;
    }

    ASSERT_GL();
}

void RenderingBackendGL::SetViewport(const Rect& viewport)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mCurrentState.Viewport == viewport)
    {
        return;
    }

    glViewport(static_cast<GLsizei>(viewport.X),
               static_cast<GLsizei>(viewport.Y),
               static_cast<GLsizei>(viewport.Width),
               static_cast<GLsizei>(viewport.Height));

    mCurrentState.Viewport = viewport;

    ASSERT_GL();
}

void RenderingBackendGL::ClearColorTarget(FrameBufferHandle hFrameBuffer,
                                          eRenderTargetBindingIndex targetIndex,
                                          const float* pColor) const
{
    const int32_t targetIdxVal = static_cast<int32_t>(targetIndex);

    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(targetIdxVal >= 0 && targetIdxVal < FrameBufferDesc::sMaxColorTargetCount, "Invalid target index.");

    if (hFrameBuffer.IsNULL())
    {
        return;
    }
    auto it = mFrameBufferToNativeHandleMap.find(hFrameBuffer);
    HO_ASSERT(it != mFrameBufferToNativeHandleMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

    HO_ASSERT(hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget.IsValid(),
              "Frame buffer dosen't have sush render target.");

    if (hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget->State == eGpuResourceState::PendingUpload)
    {
        return;
    }

    glClearNamedFramebufferfv(it->second.GlFBO, GL_COLOR, targetIdxVal, pColor);

    ASSERT_GL();
}

void RenderingBackendGL::ClearDepthTarget(FrameBufferHandle hFrameBuffer, float depth) const
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (hFrameBuffer.IsNULL())
    {
        return;
    }
    auto it = mFrameBufferToNativeHandleMap.find(hFrameBuffer);
    HO_ASSERT(it != mFrameBufferToNativeHandleMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

    HO_ASSERT(hFrameBuffer->DepthTargetState.hTarget.IsValid(), "Frame buffer dosen't have depth target.");

    if (hFrameBuffer->DepthTargetState.hTarget->State == eGpuResourceState::PendingUpload)
    {
        return;
    }

    glClearNamedFramebufferfv(it->second.GlFBO, GL_DEPTH, 0, &depth);

    ASSERT_GL();
}

void RenderingBackendGL::ClearDepthStencilTarget(FrameBufferHandle hFrameBuffer, float depth, int32_t stencil) const
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (hFrameBuffer.IsNULL())
    {
        return;
    }
    auto it = mFrameBufferToNativeHandleMap.find(hFrameBuffer);
    HO_ASSERT(it != mFrameBufferToNativeHandleMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

    HO_ASSERT(hFrameBuffer->DepthTargetState.hTarget.IsValid(), "Frame buffer dosen't have depth target.");

    if (hFrameBuffer->DepthTargetState.hTarget->State == eGpuResourceState::PendingUpload)
    {
        return;
    }

    HO_ASSERT(hFrameBuffer->DepthTargetState.hTarget->Format == eTextureFormat::D24_UNORM_S8_UINT,
              "depth target dosen't have stencil.");

    glClearNamedFramebufferfi(it->second.GlFBO, GL_DEPTH_STENCIL, 0, depth, stencil);

    ASSERT_GL();
}

void RenderingBackendGL::ClearStencilTarget(FrameBufferHandle hFrameBuffer, int32_t stencil) const
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (hFrameBuffer.IsNULL())
    {
        return;
    }
    auto it = mFrameBufferToNativeHandleMap.find(hFrameBuffer);
    HO_ASSERT(it != mFrameBufferToNativeHandleMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

    HO_ASSERT(hFrameBuffer->DepthTargetState.hTarget.IsValid(), "Frame buffer dosen't have depth target.");

    if (hFrameBuffer->DepthTargetState.hTarget->State == eGpuResourceState::PendingUpload)
    {
        return;
    }

    HO_ASSERT(hFrameBuffer->DepthTargetState.hTarget->Format == eTextureFormat::D24_UNORM_S8_UINT,
              "depth target dosen't have stencil.");

    glClearNamedFramebufferiv(it->second.GlFBO, GL_STENCIL, 0, &stencil);

    ASSERT_GL();
}

void RenderingBackendGL::BindBuffer(GpuBufferHandle hGpuBuffer, eBufferBindingIndex bindingIndex)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (hGpuBuffer.IsNULL())
    {
        HO_ASSERT(false, "Invalid gpu buffer handle. It might be not created by 'prepareCreateBuffer' yet.");
        return;
    }

    if (hGpuBuffer->State == eGpuResourceState::PendingUpload)
    {
        HO_ASSERT(false, "This buffer is not uploaded yet.");
        return;
    }

    if (!hGpuBuffer->bAllocated)
    {
        HO_ASSERT(false, "Buffer must be allocated/uploaded before binding.");
        return;
    }

    GLenum bufTarget = 0;
    switch (hGpuBuffer->Usage)
    {
        case eGpuBufferUsage::Uniform:
            bufTarget = GL_UNIFORM_BUFFER;
            break;
        case eGpuBufferUsage::Storage:
            bufTarget = GL_SHADER_STORAGE_BUFFER;
            break;
        default:
            HO_ASSERT(false, "Invalid buffer usage");
            return;
    }

    BufferBindingState& state = mCurrentState.BufferBindingStates[static_cast<int32_t>(bindingIndex)];
    if (state.hBoundBuffer == hGpuBuffer && state.Offset == 0 && state.Size == hGpuBuffer->ByteSize)
    {
        return;
    }

    if (state.hBoundBuffer.IsValid())
    {
        state.hBoundBuffer->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    const BufferNativeHandleGL* pNativeHandle = mBufferNativeHandlePool.Get(hGpuBuffer->NativeHandlePoolID);

    glBindBufferBase(bufTarget, static_cast<int32_t>(bindingIndex), pNativeHandle->GlBuffer);

    state.hBoundBuffer = hGpuBuffer;
    state.Offset = 0;
    state.Size = hGpuBuffer->ByteSize;
    ASSERT_GL();
}

void RenderingBackendGL::BindBufferSlice(GpuBufferHandle hGpuBuffer,
                                         eBufferBindingIndex bindingIndex,
                                         int32_t size,
                                         int32_t offset)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(offset + size <= hGpuBuffer->ByteSize, "Out of range.");

    if (hGpuBuffer.IsNULL())
    {
        HO_ASSERT(false, "Invalid gpu buffer handle. It might be not created by 'prepareCreateBuffer' yet.");
        return;
    }

    if (hGpuBuffer->State == eGpuResourceState::PendingUpload)
    {
        HO_ASSERT(false, "This buffer is not uploaded yet.");
        return;
    }

    if (!hGpuBuffer->bAllocated)
    {
        HO_ASSERT(false, "Buffer must be allocated/uploaded before binding.");
        return;
    }

    GLenum bufTarget = 0;
    switch (hGpuBuffer->Usage)
    {
        case eGpuBufferUsage::Uniform:
            HO_ASSERT(offset % sAlignmentUBO == 0,
                      "Uniform buffer slice offset must be aligned to GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT.");
            bufTarget = GL_UNIFORM_BUFFER;
            break;

        case eGpuBufferUsage::Storage:
            HO_ASSERT(offset % sAlignmentSSBO == 0,
                      "Storage buffer slice offset must be aligned to GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT.");
            bufTarget = GL_SHADER_STORAGE_BUFFER;
            break;

        default:
            HO_ASSERT(false, "Invalid buffer usage");
            return;
    }

    BufferBindingState& state = mCurrentState.BufferBindingStates[static_cast<int32_t>(bindingIndex)];
    if (state.hBoundBuffer == hGpuBuffer && state.Offset == offset && state.Size == hGpuBuffer->ByteSize)
    {
        return;
    }

    if (state.hBoundBuffer.IsValid())
    {
        state.hBoundBuffer->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    const BufferNativeHandleGL* pNativeHandle = mBufferNativeHandlePool.Get(hGpuBuffer->NativeHandlePoolID);

    glBindBufferRange(bufTarget, static_cast<int32_t>(bindingIndex), pNativeHandle->GlBuffer, offset, size);

    state.hBoundBuffer = hGpuBuffer;
    state.Offset = offset;
    state.Size = hGpuBuffer->ByteSize;
    ASSERT_GL();
}

void RenderingBackendGL::BindStorageBuffer(GpuBufferHandle hGpuBuffer, eBufferBindingIndex bindingIndex)
{
    if (hGpuBuffer.IsNULL())
    {
        HO_ASSERT(false, "Invalid gpu buffer handle. It might be not created by 'prepareCreateBuffer' yet.");
        return;
    }
    HO_ASSERT(hGpuBuffer->Usage == eGpuBufferUsage::Storage, "Only storage buffer can be bound as storage.");
    BindBuffer(hGpuBuffer, bindingIndex);
}

void RenderingBackendGL::BindStorageBufferSlice(GpuBufferHandle hGpuBuffer,
                                                eBufferBindingIndex bindingIndex,
                                                int32_t size,
                                                int32_t offset)
{
    if (hGpuBuffer.IsNULL())
    {
        HO_ASSERT(false, "Invalid gpu buffer handle. It might be not created by 'prepareCreateBuffer' yet.");
        return;
    }
    HO_ASSERT(hGpuBuffer->Usage == eGpuBufferUsage::Storage, "Only storage buffer can be bound as storage.");
    BindBufferSlice(hGpuBuffer, bindingIndex, size, offset);
}

void RenderingBackendGL::BindStaticMesh(GpuStaticMeshHandle hMesh)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (hMesh.IsNULL())
    {
        HO_ASSERT(false, "Invalid gpu static mesh handle. It might be not created by 'prepareUploadStaticMesh' yet.");
        return;
    }

    if (hMesh->State == eGpuResourceState::PendingUpload)
    {
        HO_ASSERT(false, "This static mesh is not uploaded yet.");
        return;
    }

    if (mCurrentState.hBoundStaticMesh == hMesh)
    {
        return;
    }

    // Update prev mesh Last used frame
    if (mCurrentState.hBoundStaticMesh.IsValid())
    {
        mCurrentState.hBoundStaticMesh->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    const StaticMeshNativeHandleGL* pMeshNativehandle = mStaticMeshNativeHandlePool.Get(hMesh->NativeHandlePoolID);

    glVertexArrayVertexBuffer(mGlVAO, mAttribBindingIndex, pMeshNativehandle->GlVBO, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(mGlVAO, pMeshNativehandle->GlEBO);

    mCurrentState.hBoundStaticMesh = hMesh;
    ASSERT_GL();
}

void RenderingBackendGL::BindTexture(GpuTextureHandle hTexture, eTextureBindingIndex bindingIndex)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    const int32_t binding = static_cast<int32_t>(bindingIndex);
    if (mCurrentState.hBoundTextures[binding] == hTexture)
    {
        return;
    }

    if (mCurrentState.hBoundTextures[binding].IsValid())
    {
        mCurrentState.hBoundTextures[binding]->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    if (hTexture.IsNULL())
    {
        glBindTextureUnit(binding, 0);
        mCurrentState.hBoundTextures[binding] = hTexture;
        ASSERT_GL();
        return;
    }

    if (hTexture->State == eGpuResourceState::PendingUpload)
    {
        HO_ASSERT(false, "This texture is not uploaded yet.");
        return;
    }

    glBindTextureUnit(binding, mTextureNativeHandlePool.Get(hTexture->NativeHandlePoolID)->GlTexture);
    mCurrentState.hBoundTextures[binding] = hTexture;

    ASSERT_GL();
}

void RenderingBackendGL::BindStorageTexture(GpuTextureHandle hTexture,
                                            eStorageTextureBindingIndex bindingIndex,
                                            int32_t mipLevel,
                                            int32_t arrayIdx,
                                            eResourceAccessType access)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    const int32_t slotIndex = static_cast<int32_t>(bindingIndex);
    StorageTextureBindingState& state = mCurrentState.StorageTextureBindingStates[slotIndex];

    if (hTexture.IsNULL())
    {
        if (state.hBoundTexture.IsNULL())
        {
            return;
        }

        if (state.hBoundTexture.IsValid())
        {
            state.hBoundTexture->LastUsedFrame = mCurrentState.CurrentFrame;
        }

        glBindImageTexture(slotIndex, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);

        state.hBoundTexture = GpuTextureHandle::sNULL;
        state.MipLevel = 0;
        state.ArrayIdx = 0;
        state.Access = access;

        ASSERT_GL();
        return;
    }

    HO_ASSERT(isStorableTextureFormat(hTexture->Format),
              "GL not support using compressed format or depth/stencil format as output image.");
    HO_ASSERT(mipLevel < hTexture->MipLevels && arrayIdx < hTexture->ArraySize, "Out of range.");

    if (hTexture->State == eGpuResourceState::PendingUpload)
    {
        HO_ASSERT(false, "This texture is not uploaded yet.");
        return;
    }

    if (state.hBoundTexture == hTexture && state.MipLevel == mipLevel && state.ArrayIdx == arrayIdx &&
        state.Access == access)
    {
        return;
    }

    if (state.hBoundTexture.IsValid())
    {
        state.hBoundTexture->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    const TextureNativeHandleGL* pNativeHandle = mTextureNativeHandlePool.Get(hTexture->NativeHandlePoolID);

    GLboolean bLayered = GL_FALSE;

    switch (hTexture->Type)
    {
        case eTextureType::Texture1D:
        case eTextureType::Texture2D:
            HO_ASSERT(arrayIdx == 0, "Non-array texture arrayIdx must be 0.");
            bLayered = GL_FALSE;
            break;

        case eTextureType::Texture3D:
        case eTextureType::TextureCubeMap:
            HO_ASSERT(arrayIdx == 0, "Full 3D/CubeMap binding arrayIdx must be 0.");
            bLayered = GL_TRUE;
            break;

        case eTextureType::Texture1DArray:
        case eTextureType::Texture2DArray:
        case eTextureType::TextureCubeMapArray:
            HO_ASSERT(arrayIdx >= 0 && arrayIdx < hTexture->ArraySize, "Array index out of bounds.");
            bLayered = GL_FALSE;
            break;

        default:
            HO_ASSERT(false, "Invalid texture type.");
    }

    glBindImageTexture(slotIndex,
                       pNativeHandle->GlTexture,
                       mipLevel,
                       bLayered,
                       arrayIdx,
                       toGlAccessType(access),
                       toGlInternalFormat(hTexture->Format));

    state.hBoundTexture = hTexture;
    state.MipLevel = mipLevel;
    state.ArrayIdx = arrayIdx;
    state.Access = access;
    ASSERT_GL();
}

void RenderingBackendGL::BindShader(GpuShaderHandle hShader, eShaderStage stage)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (hShader.IsValid() && hShader->State == eGpuResourceState::PendingUpload)
    {
        HO_ASSERT(false, "This shader is not uploaded yet.");
        return;
    }

    if (mCurrentState.hBoundShaders[static_cast<int32_t>(stage)] == hShader)
    {
        return;
    }

    // Update prev shader Last used frame
    if (mCurrentState.hBoundShaders[static_cast<int32_t>(stage)].IsValid())
    {
        mCurrentState.hBoundShaders[static_cast<int32_t>(stage)]->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    GLuint glShader = 0;
    if (hShader.IsValid())
    {
        glShader = mShaderNativeHandlePool.Get(hShader->NativeHandlePoolID)->GlProgram;
    }

    glUseProgramStages(mGlProgramPipeline, toGlShaderBitfield(stage), glShader);

    mCurrentState.hBoundShaders[static_cast<int32_t>(stage)] = hShader;
    ASSERT_GL();
}

void RenderingBackendGL::BindMorphTargets(GpuMorphTargetsHandle hGpuMorphTargets)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (hGpuMorphTargets.IsValid() && hGpuMorphTargets->State != eGpuResourceState::PendingUpload)
    {
        if (mCurrentState.hBoundMorphTargets == hGpuMorphTargets)
        {
            return;
        }

        // Update prev morph targets Last used frame
        if (mCurrentState.hBoundMorphTargets.IsValid())
        {
            mCurrentState.hBoundMorphTargets->LastUsedFrame = mCurrentState.CurrentFrame;
        }

        const MorphTargetsNativeHandleGL* pMorphTargetsNativeHandle =
            mMorphTargetsNativeHandlePool.Get(hGpuMorphTargets->NativeHandlePoolID);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                         static_cast<int>(eBufferBindingIndex::MorphTargets),
                         pMorphTargetsNativeHandle->GlMorphTargetsSSBO);
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER,
                          static_cast<int>(eBufferBindingIndex::MorphTargetPositions),
                          pMorphTargetsNativeHandle->GlTargetAttributesSSBO,
                          hGpuMorphTargets->AllPositionByteOffset,
                          hGpuMorphTargets->AllPositionByteSize);
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER,
                          static_cast<int>(eBufferBindingIndex::MorphTargetNormals),
                          pMorphTargetsNativeHandle->GlTargetAttributesSSBO,
                          hGpuMorphTargets->AllNormalByteOffset,
                          hGpuMorphTargets->AllNormalByteSize);
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER,
                          static_cast<int>(eBufferBindingIndex::MorphTargetTangents),
                          pMorphTargetsNativeHandle->GlTargetAttributesSSBO,
                          hGpuMorphTargets->AllTangentByteOffset,
                          hGpuMorphTargets->AllTangentByteSize);

        mCurrentState.hBoundMorphTargets = hGpuMorphTargets;
    }
    else
    {
        // Update prev morph targets Last used frame
        if (mCurrentState.hBoundMorphTargets.IsValid())
        {
            mCurrentState.hBoundMorphTargets->LastUsedFrame = mCurrentState.CurrentFrame;
        }

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, static_cast<int>(eBufferBindingIndex::MorphTargets), 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, static_cast<int>(eBufferBindingIndex::MorphTargetPositions), 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, static_cast<int>(eBufferBindingIndex::MorphTargetNormals), 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, static_cast<int>(eBufferBindingIndex::MorphTargetTangents), 0);
        mCurrentState.hBoundMorphTargets = GpuMorphTargetsHandle::sNULL;
    }

    ASSERT_GL();
}

void RenderingBackendGL::ExecuteDrawCommand(const DrawCommand& command)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    HO_ASSERT(mCurrentState.hBoundFrameBuffer.IsValid(), "FrameBuffer must be bound before executing draw command.");

    HO_ASSERT(mCurrentState.hBoundStaticMesh.IsValid(), "Static mesh must be bound.");
    HO_ASSERT(mCurrentState.hBoundStaticMesh->State != eGpuResourceState::PendingUpload,
              "Bound static mesh is still pending upload.");

    HO_ASSERT(mCurrentState.hBoundMaterial.IsValid(), "Material must be bound.");
    HO_ASSERT(mCurrentState.hBoundMaterial->State != eGpuResourceState::PendingUpload,
              "Bound material is still pending upload.");

    const GpuShaderHandle hVertexShader = mCurrentState.hBoundShaders[static_cast<int32_t>(eShaderStage::VertexShader)];
    const GpuShaderHandle hFragmentShader =
        mCurrentState.hBoundShaders[static_cast<int32_t>(eShaderStage::FragmentShader)];
    (void)hVertexShader;
    (void)hFragmentShader;
    HO_ASSERT(hVertexShader.IsValid(), "Vertex shader must be bound.");
    HO_ASSERT(hVertexShader->State != eGpuResourceState::PendingUpload, "Bound vertex shader is still pending upload.");

    HO_ASSERT(hFragmentShader.IsValid(), "Fragment shader must be bound.");
    HO_ASSERT(hFragmentShader->State != eGpuResourceState::PendingUpload,
              "Bound fragment shader is still pending upload.");

    HO_ASSERT(command.StencilWriteBitIndex >= -1 && command.StencilWriteBitIndex <= 7, "Out of bounds.");
    if (mCurrentState.hBoundFrameBuffer->DepthTargetState.hTarget->Format == eTextureFormat::D24_UNORM_S8_UINT)
    {
        uint8_t writeMask = 0;
        if (command.StencilWriteBitIndex >= 0)
        {
            writeMask = static_cast<uint8_t>(1 << command.StencilWriteBitIndex);
        }
        SetStencilWriteMask(writeMask);
    }

    glDrawElementsInstancedBaseVertexBaseInstance(
        GL_TRIANGLES,
        command.IndexCount,
        GL_UNSIGNED_INT,
        reinterpret_cast<void*>(command.IndexOffset * sizeof(uint32_t)), // NOLINT
        command.InstanceCount,
        command.VertexOffset,
        command.InstanceOffset);

    ASSERT_GL();
}

void RenderingBackendGL::DrawProcedural(int32_t vertexCount)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    HO_ASSERT(mCurrentState.hBoundFrameBuffer.IsValid(), "FrameBuffer must be bound before executing draw command.");

    const GpuShaderHandle hVertexShader = mCurrentState.hBoundShaders[static_cast<int32_t>(eShaderStage::VertexShader)];
    const GpuShaderHandle hFragmentShader =
        mCurrentState.hBoundShaders[static_cast<int32_t>(eShaderStage::FragmentShader)];
    (void)hVertexShader;
    (void)hFragmentShader;
    HO_ASSERT(hVertexShader.IsValid(), "Vertex shader must be bound.");
    HO_ASSERT(hVertexShader->State != eGpuResourceState::PendingUpload, "Bound vertex shader is still pending upload.");

    HO_ASSERT(hFragmentShader.IsValid(), "Fragment shader must be bound.");
    HO_ASSERT(hFragmentShader->State != eGpuResourceState::PendingUpload,
              "Bound fragment shader is still pending upload.");

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    ASSERT_GL();
}

void RenderingBackendGL::Dispatch(uint32_t groupXCount, uint32_t groupYCount, uint32_t groupZCount)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    const GpuShaderHandle hComputeShader =
        mCurrentState.hBoundShaders[static_cast<int32_t>(eShaderStage::ComputeShader)];
    (void)hComputeShader;
    HO_ASSERT(hComputeShader.IsValid(), "Compute shader must be bound.");
    HO_ASSERT(hComputeShader->State != eGpuResourceState::PendingUpload,
              "Bound compute shader is still pending upload.");

    glDispatchCompute(groupXCount, groupYCount, groupZCount);
    ASSERT_GL();
}

void RenderingBackendGL::RenderUI(const UIViewportData& data)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    const IPlatformWindow* pWindow = static_cast<const IPlatformWindow*>(data.pTargetWindow);

    pWindow->ActivateContext();

    // FIX: "INVALID_VALUE" error occurs in 'ImGui_ImplOpenGL3_RenderDrawData'.
    ImGui_ImplOpenGL3_RenderDrawData(data.pDrawData);
    ASSERT_GL();

    glFlush();
    ASSERT_GL();

    pWindow->Present();
    ASSERT_GL();
}

void RenderingBackendGL::Flush()
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    glFlush();
    ASSERT_GL();
}

void RenderingBackendGL::ResourceBarrier(GpuTextureHandle hTexture, eResourceState srcState, eResourceState dstState)
{
    HO_ASSERT(hTexture.IsValid(), "Invalid texture.");

    if ((srcState == eResourceState::RenderTarget || srcState == eResourceState::DepthStencilWrite) &&
        dstState == eResourceState::ShaderResource)
    {
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    ASSERT_GL();
}

void RenderingBackendGL::ResourceBarrier(GpuBufferHandle hBuffer, eResourceState srcState, eResourceState dstState)
{
    HO_ASSERT(hBuffer.IsValid(), "Invalid buffer.");

    if (srcState == eResourceState::UnorderedAccess &&
        (dstState == eResourceState::VertexBuffer || dstState == eResourceState::ConstantBuffer))
    {
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);
    }

    ASSERT_GL();
}

void RenderingBackendGL::SetVSync(bool bEnabled)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (mbVSyncEnabled == bEnabled)
    {
        return;
    }

#if defined(_WIN32)
    static_cast<Win32ApplicationGL&>(IPlatformApplication::GetInstance()).SetVSync(bEnabled);
    ASSERT_GL();
#elif defined(__linux__)
#error "Linux environment is currently not supported."
#endif

    mbVSyncEnabled = bEnabled;
}

FrameBufferHandle RenderingBackendGL::CreateFrameBuffer(const FrameBufferDesc& desc)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(desc.hName.IsValid() && !desc.hName.IsEmpty(), "Frame buffer's name must be valid and can't be empty.");

    HO_ASSERT(desc.Height > 0 && desc.Width > 0, "Must be grater than zero.");

    FrameBuffer newFrameBuffer;
    newFrameBuffer.hName = desc.hName;
    newFrameBuffer.Width = desc.Width;
    newFrameBuffer.Height = desc.Height;

    // Create FBO
    GLuint glFBO = 0;
    glCreateFramebuffers(1, &glFBO);
    if (!ASSERT_GL())
    {
        return FrameBufferHandle::sNULL;
    }

    for (int32_t targetIdx = 0; targetIdx < FrameBufferDesc::sMaxColorTargetCount; ++targetIdx)
    {
        if (desc.ColorTargetFormats[targetIdx] != eTextureFormat::None)
        {
            HO_ASSERT(desc.ColorTargetFormats[targetIdx] >= eTextureFormat::R8_UNORM &&
                          desc.ColorTargetFormats[targetIdx] <= eTextureFormat::R32G32B32A32_FLOAT,
                      "Invalid format for color render target.");

            newFrameBuffer.ColorTargetStates[targetIdx].hTarget =
                PrepareCreateTexture2D(desc.ColorTargetFormats[targetIdx], desc.Width, desc.Height, 1, 1, true);
            CreateTexture(newFrameBuffer.ColorTargetStates[targetIdx].hTarget);
            glNamedFramebufferTexture(
                glFBO,
                GL_COLOR_ATTACHMENT0 + targetIdx,
                mTextureNativeHandlePool.Get(newFrameBuffer.ColorTargetStates[targetIdx].hTarget->NativeHandlePoolID)
                    ->GlTexture,
                0);
            newFrameBuffer.ColorTargetStates[targetIdx].BaseSlice = 0;
            newFrameBuffer.ColorTargetStates[targetIdx].SliceCount = 1;
            newFrameBuffer.ColorTargetStates[targetIdx].MipLevel = 0;
            ++newFrameBuffer.ColorTargetStates[targetIdx].hTarget->FrameBufferAttachmentCount;

            ASSERT_GL();
        }
    }

    if (desc.DepthTargetFormat != eTextureFormat::None)
    {
        HO_ASSERT(desc.DepthTargetFormat == eTextureFormat::D24_UNORM_S8_UINT ||
                      desc.DepthTargetFormat == eTextureFormat::D32_FLOAT,
                  "Invalid format for depth render target.");

        const GLenum attachment = (desc.DepthTargetFormat == eTextureFormat::D24_UNORM_S8_UINT)
                                      ? GL_DEPTH_STENCIL_ATTACHMENT
                                      : GL_DEPTH_ATTACHMENT;

        newFrameBuffer.DepthTargetState.hTarget =
            PrepareCreateTexture2D(desc.DepthTargetFormat, desc.Width, desc.Height, 1, 1, true);
        CreateTexture(newFrameBuffer.DepthTargetState.hTarget);
        glNamedFramebufferTexture(
            glFBO,
            attachment,
            mTextureNativeHandlePool.Get(newFrameBuffer.DepthTargetState.hTarget->NativeHandlePoolID)->GlTexture,
            0);
        newFrameBuffer.DepthTargetState.BaseSlice = 0;
        newFrameBuffer.DepthTargetState.SliceCount = 1;
        newFrameBuffer.DepthTargetState.MipLevel = 0;
        ++newFrameBuffer.DepthTargetState.hTarget->FrameBufferAttachmentCount;
        ASSERT_GL();
    }

    GLenum drawBuffers[FrameBufferDesc::sMaxColorTargetCount] = {};
    int32_t maxActiveSlot = -1;

    for (int32_t i = 0; i < FrameBufferDesc::sMaxColorTargetCount; ++i)
    {
        if (newFrameBuffer.ColorTargetStates[i].hTarget.IsValid())
        {
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
            maxActiveSlot = i;
        }
        else
        {
            drawBuffers[i] = GL_NONE;
        }
    }

    if (maxActiveSlot >= 0)
    {
        glNamedFramebufferDrawBuffers(glFBO, maxActiveSlot + 1, drawBuffers);
    }
    else
    {
        const GLenum noneTarget = GL_NONE;
        glNamedFramebufferDrawBuffers(glFBO, 1, &noneTarget);
    }

    ASSERT_GL();

    const uint64_t newFrameBufferID = mFrameBufferPool.Add(newFrameBuffer);
    FrameBufferHandle hNewFB = FrameBufferHandle(&mFrameBufferPool, newFrameBufferID);
    hNewFB->hThis = hNewFB;
    FrameBufferNativeHandleGL nativeHandle;
    nativeHandle.GlFBO = glFBO;
    mFrameBufferToNativeHandleMap.insert({hNewFB, nativeHandle});

    if (maxActiveSlot >= 0 || newFrameBuffer.DepthTargetState.hTarget.IsValid())
    {
        if (glCheckNamedFramebufferStatus(glFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            HO_ASSERT(false, "Framebuffer is incomplete.");
            DestroyFrameBuffer(hNewFB);
            return FrameBufferHandle::sNULL;
        }
    }

    return hNewFB;
}

bool RenderingBackendGL::DestroyFrameBuffer(FrameBufferHandle hFrameBuffer)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (hFrameBuffer.IsNULL())
    {
        return true;
    }

    HO_ASSERT(hFrameBuffer != mCurrentState.hBoundFrameBuffer, "Frame buffer must be unbound before destroyed.");

    auto glit = mFrameBufferToNativeHandleMap.find(hFrameBuffer);
    HO_ASSERT(glit != mFrameBufferToNativeHandleMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

    // Delete tuxtures
    for (int32_t targetIdx = 0; targetIdx < FrameBufferDesc::sMaxColorTargetCount; ++targetIdx)
    {
        if (hFrameBuffer->ColorTargetStates[targetIdx].hTarget.IsValid())
        {
            if (hFrameBuffer->ColorTargetStates[targetIdx].hTarget->FrameBufferAttachmentCount == 1)
            {
                UnloadTexture(hFrameBuffer->ColorTargetStates[targetIdx].hTarget);
            }
            else
            {
                --hFrameBuffer->ColorTargetStates[targetIdx].hTarget->FrameBufferAttachmentCount;
            }
        }

        GLuint& glViewTex = glit->second.GlColorTargetViews[targetIdx];
        if (glViewTex != 0)
        {
            glDeleteTextures(1, &glViewTex);
            glViewTex = 0;
        }
    }

    if (hFrameBuffer->DepthTargetState.hTarget.IsValid())
    {
        if (hFrameBuffer->DepthTargetState.hTarget->FrameBufferAttachmentCount == 1)
        {
            UnloadTexture(hFrameBuffer->DepthTargetState.hTarget);
        }
        else
        {
            --hFrameBuffer->DepthTargetState.hTarget->FrameBufferAttachmentCount;
        }
    }

    if (glit->second.GlDepthTargetView != 0)
    {
        glDeleteTextures(1, &glit->second.GlDepthTargetView);
        glit->second.GlDepthTargetView = 0;
    }

    // Delete FBO
    glDeleteFramebuffers(1, &glit->second.GlFBO);
    mFrameBufferToNativeHandleMap.erase(hFrameBuffer);
    mFrameBufferPool.Remove(hFrameBuffer.GetID());
    return true;
}

void* RenderingBackendGL::GetColorTargetNativeHandle(FrameBufferHandle hFrameBuffer,
                                                     eRenderTargetBindingIndex targetIndex) const
{
    const int32_t targetIdxVal = static_cast<int32_t>(targetIndex);

    HO_ASSERT(targetIdxVal >= 0 && targetIdxVal < FrameBufferDesc::sMaxColorTargetCount, "Target index out of range.");
    HO_ASSERT(hFrameBuffer.IsValid(), "Framebuffer is invalid. It might be not created by 'CreateFrameBuffer' yet.");

    HO_ASSERT(hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget.IsValid(),
              "Frame buffer doesn't have such color target.");
    const TextureNativeHandleGL* nativeHandle =
        mTextureNativeHandlePool.Get(hFrameBuffer->ColorTargetStates[targetIdxVal].hTarget->NativeHandlePoolID);

    HO_ASSERT(nativeHandle, "Texture for color target was not created.");

    return reinterpret_cast<void*>(static_cast<uintptr_t>(nativeHandle->GlTexture)); // NOLINT
}

void* RenderingBackendGL::GetDepthTargetNativeHandle(FrameBufferHandle hFrameBuffer) const
{
    HO_ASSERT(hFrameBuffer.IsValid(), "Framebuffer is invalid. It might be not created by 'CreateFrameBuffer' yet.");

    HO_ASSERT(hFrameBuffer->DepthTargetState.hTarget.IsValid(), "Frame buffer doesn't have depth target.");
    const TextureNativeHandleGL* nativeHandle =
        mTextureNativeHandlePool.Get(hFrameBuffer->DepthTargetState.hTarget->NativeHandlePoolID);

    HO_ASSERT(nativeHandle, "Texture for color target was not created.");

    return reinterpret_cast<void*>(static_cast<uintptr_t>(nativeHandle->GlTexture)); // NOLINT
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::CreateBuffer(GpuBufferHandle hPreparedGpuBuffer)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hPreparedGpuBuffer.IsValid(),
              "Invalid gpu buffer handle. It might be not created by 'prepareCreateBuffer' yet.");

    GLuint glBuffer = 0;
    glCreateBuffers(1, &glBuffer);
    if (!ASSERT_GL())
    {
        return eResourceTransferResult::Failed;
    }
    BufferNativeHandleGL nativeHandle;
    nativeHandle.GlBuffer = glBuffer;
    hPreparedGpuBuffer->NativeHandlePoolID = mBufferNativeHandlePool.Add(nativeHandle);
    hPreparedGpuBuffer->LastUsedFrame = mCurrentState.CurrentFrame;
    hPreparedGpuBuffer->bAllocated = false;
    hPreparedGpuBuffer->State = eGpuResourceState::UpToDate;
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::CreateTexture(GpuTextureHandle hPreparedGpuTexture)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hPreparedGpuTexture.IsValid() && hPreparedGpuTexture->hOrigin.IsNULL(),
              "Invalid gpu texture handle. It might be not created by 'prepareCreateTexture' yet.");
    HO_ASSERT(hPreparedGpuTexture->State == eGpuResourceState::PendingUpload,
              "PrepareCreateTexture must be called before create.");

    GLuint glTexture;
    glCreateTextures(toGlTextureType(hPreparedGpuTexture->Type, hPreparedGpuTexture->SampleCount), 1, &glTexture);

    if (!ASSERT_GL())
    {
        return eResourceTransferResult::Failed;
    }

    switch (hPreparedGpuTexture->Type)
    {
        case eTextureType::Texture1D:
            glTextureStorage1D(glTexture,
                               hPreparedGpuTexture->MipLevels,
                               toGlInternalFormat(hPreparedGpuTexture->Format),
                               hPreparedGpuTexture->Width);
            break;
        case eTextureType::Texture2D:
            if (hPreparedGpuTexture->SampleCount > 1)
            {
                glTextureStorage2DMultisample(glTexture,
                                              hPreparedGpuTexture->SampleCount,
                                              toGlInternalFormat(hPreparedGpuTexture->Format),
                                              hPreparedGpuTexture->Width,
                                              hPreparedGpuTexture->Height,
                                              true);
            }
            else
            {
                glTextureStorage2D(glTexture,
                                   hPreparedGpuTexture->MipLevels,
                                   toGlInternalFormat(hPreparedGpuTexture->Format),
                                   hPreparedGpuTexture->Width,
                                   hPreparedGpuTexture->Height);
            }
            break;
        case eTextureType::TextureCubeMap:
            glTextureStorage2D(glTexture,
                               hPreparedGpuTexture->MipLevels,
                               toGlInternalFormat(hPreparedGpuTexture->Format),
                               hPreparedGpuTexture->Width,
                               hPreparedGpuTexture->Height);
            break;
        case eTextureType::Texture3D:
            glTextureStorage3D(glTexture,
                               hPreparedGpuTexture->MipLevels,
                               toGlInternalFormat(hPreparedGpuTexture->Format),
                               hPreparedGpuTexture->Width,
                               hPreparedGpuTexture->Height,
                               hPreparedGpuTexture->Depth);
            break;
        case eTextureType::Texture1DArray:
            glTextureStorage2D(glTexture,
                               hPreparedGpuTexture->MipLevels,
                               toGlInternalFormat(hPreparedGpuTexture->Format),
                               hPreparedGpuTexture->Width,
                               hPreparedGpuTexture->ArraySize);
            break;
        case eTextureType::Texture2DArray:
            if (hPreparedGpuTexture->SampleCount > 1)
            {
                glTextureStorage3DMultisample(glTexture,
                                              hPreparedGpuTexture->SampleCount,
                                              toGlInternalFormat(hPreparedGpuTexture->Format),
                                              hPreparedGpuTexture->Width,
                                              hPreparedGpuTexture->Height,
                                              hPreparedGpuTexture->ArraySize,
                                              true);
            }
            else
            {
                glTextureStorage3D(glTexture,
                                   hPreparedGpuTexture->MipLevels,
                                   toGlInternalFormat(hPreparedGpuTexture->Format),
                                   hPreparedGpuTexture->Width,
                                   hPreparedGpuTexture->Height,
                                   hPreparedGpuTexture->ArraySize);
            }
            break;
        case eTextureType::TextureCubeMapArray:
            glTextureStorage3D(glTexture,
                               hPreparedGpuTexture->MipLevels,
                               toGlInternalFormat(hPreparedGpuTexture->Format),
                               hPreparedGpuTexture->Width,
                               hPreparedGpuTexture->Height,
                               hPreparedGpuTexture->ArraySize * 6);
            break;
        default:
            HO_ASSERT(false, "Invalid texture type.");
            glDeleteTextures(1, &glTexture);
            return eResourceTransferResult::Failed;
    }

    glTextureParameteri(glTexture, GL_TEXTURE_MAX_LEVEL, hPreparedGpuTexture->MipLevels - 1);

    if (!ASSERT_GL())
    {
        glDeleteTextures(1, &glTexture);
        return eResourceTransferResult::Failed;
    }

    TextureNativeHandleGL nativeHandle;
    nativeHandle.GlTexture = glTexture;
    hPreparedGpuTexture->NativeHandlePoolID = mTextureNativeHandlePool.Add(nativeHandle);
    hPreparedGpuTexture->LastUsedFrame = mCurrentState.CurrentFrame;
    hPreparedGpuTexture->State = eGpuResourceState::UpToDate;
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::UploadBufferData(GpuBufferHandle hGpuBuffer,
                                                                                const void* pSrcData,
                                                                                int32_t size,
                                                                                int32_t offset)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hGpuBuffer.IsValid() && hGpuBuffer->State == eGpuResourceState::UpToDate,
              "Invalid gpu buffer handle. It might be not created by 'CreateBuffer' yet.");
    HO_ASSERT(offset + size <= hGpuBuffer->ByteSize, "Out of range.");
    HO_ASSERT(pSrcData, "Uploading data can't be null.");

    const GLuint glBuffer = mBufferNativeHandlePool.Get(hGpuBuffer->NativeHandlePoolID)->GlBuffer;
    if (!hGpuBuffer->bAllocated)
    {
        hGpuBuffer->bAllocated = true;
        GLbitfield flags = 0;
        switch (hGpuBuffer->Access)
        {
            case eGpuBufferAccess::Static:
                HO_ASSERT(offset == 0 && size == hGpuBuffer->ByteSize,
                          "Static buffer must be uploaded fully at once from offset 0.");
                glNamedBufferStorage(glBuffer, static_cast<GLsizeiptr>(hGpuBuffer->ByteSize), pSrcData, 0);
                return eResourceTransferResult::Success;
                break;
            case eGpuBufferAccess::Dynamic:
            case eGpuBufferAccess::ReadBack:
                flags |= GL_DYNAMIC_STORAGE_BIT;
                break;
            default:
                HO_ASSERT(false, "Invalid access flag.");
                return eResourceTransferResult::Failed;
        }
        glNamedBufferStorage(glBuffer, static_cast<GLsizeiptr>(hGpuBuffer->ByteSize), nullptr, flags);

        if (!ASSERT_GL())
        {
            return eResourceTransferResult::Failed;
        }
    }

    if (hGpuBuffer->Access == eGpuBufferAccess::Static)
    {
        HO_ASSERT(false, "Static buffer can be upload once.");
        return eResourceTransferResult::Failed;
    }

    if (size == 0)
    {
        return eResourceTransferResult::Success;
    }

    glNamedBufferSubData(glBuffer, offset, size, pSrcData);

    if (!ASSERT_GL())
    {
        return eResourceTransferResult::Failed;
    }
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::UploadStaticMesh(
    GpuStaticMeshHandle hPreparedGpuStaticMesh)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hPreparedGpuStaticMesh.IsValid(),
              "Invalid gpu static mesh handle. It might be not created by 'prepareUploadStatucMesh' yet.");
    HO_ASSERT(hPreparedGpuStaticMesh->State == eGpuResourceState::PendingUpload,
              "PrepareUploadStaticMesh must be called before upload.");

    if (hPreparedGpuStaticMesh->hOrigin.IsNULL())
    {
        HO_ASSERT(false, "Invalid static mesh asset. It might be removed from asset system while pending.");
        return eResourceTransferResult::Failed;
    }
    const ObjectHandle<const RStaticMeshAsset> hOrigin = hPreparedGpuStaticMesh->hOrigin;

    const int32_t vertexCount = hOrigin->Positions.GetSize();

    FixedArray<Vertex> vertexBuffer(vertexCount);

    for (int32_t i = 0; i < vertexCount; ++i)
    {
        vertexBuffer[i].Position = hOrigin->Positions[i];
        if (i < hOrigin->Normals.GetSize())
        {
            vertexBuffer[i].Normal = hOrigin->Normals[i];
        }
        if (i < hOrigin->Tangents.GetSize())
        {
            vertexBuffer[i].Tangent = hOrigin->Tangents[i];
        }
        if (i < hOrigin->UVs.GetSize())
        {
            vertexBuffer[i].UV0 = hOrigin->UVs[i][0];
            vertexBuffer[i].UV1 = hOrigin->UVs[i][1];
        }
        if (i < hOrigin->Colors.GetSize())
        {
            vertexBuffer[i].Color0[0] = hOrigin->Colors[i][0].R;
            vertexBuffer[i].Color0[1] = hOrigin->Colors[i][0].G;
            vertexBuffer[i].Color0[2] = hOrigin->Colors[i][0].B;
            vertexBuffer[i].Color0[3] = hOrigin->Colors[i][0].A;
        }
    }

    GLuint glVBO = 0;
    glCreateBuffers(1, &glVBO);
    glNamedBufferStorage(glVBO, static_cast<GLsizeiptr>(sizeof(Vertex) * vertexCount), vertexBuffer.Data(), 0);
    if (!ASSERT_GL())
    {
        glDeleteBuffers(1, &glVBO);
        return eResourceTransferResult::Failed;
    }
    GLuint glEBO = 0;
    glCreateBuffers(1, &glEBO);
    glNamedBufferStorage(
        glEBO, static_cast<GLsizeiptr>(sizeof(int32_t) * hOrigin->Indices.GetSize()), hOrigin->Indices.Data(), 0);
    if (!ASSERT_GL())
    {
        glDeleteBuffers(1, &glVBO);
        glDeleteBuffers(1, &glEBO);
        return eResourceTransferResult::Failed;
    }
    StaticMeshNativeHandleGL nativeHandle;
    nativeHandle.GlVBO = glVBO;
    nativeHandle.GlEBO = glEBO;
    hPreparedGpuStaticMesh->NativeHandlePoolID = mStaticMeshNativeHandlePool.Add(nativeHandle);
    hPreparedGpuStaticMesh->LastUsedFrame = mCurrentState.CurrentFrame;
    hPreparedGpuStaticMesh->State = eGpuResourceState::UpToDate;
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::UploadTexture(GpuTextureHandle hPreparedGpuTexture)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hPreparedGpuTexture.IsValid(),
              "Invalid gpu texture handle. It might be not created by 'prepareUploadTexture' yet.");
    HO_ASSERT(hPreparedGpuTexture->State == eGpuResourceState::PendingUpload,
              "PrepareUploadTexture must be called before upload.");

    if (hPreparedGpuTexture->hOrigin.IsNULL())
    {
        HO_ASSERT(false, "Invalid texture asset. It might be removed from asset system while pending.");
        return eResourceTransferResult::Failed;
    }

    const ObjectHandle<const RTextureAsset> hOrigin = hPreparedGpuTexture->hOrigin;

    GLuint glTexture;
    glCreateTextures(toGlTextureType(hOrigin->Type, 1), 1, &glTexture);

    if (!ASSERT_GL())
    {
        return eResourceTransferResult::Failed;
    }

    switch (hOrigin->Type)
    {
        case eTextureType::Texture1D:
            glTextureStorage1D(glTexture, hOrigin->MipLevels, toGlInternalFormat(hOrigin->Format), hOrigin->Width);
            break;
        case eTextureType::Texture2D:
        case eTextureType::TextureCubeMap:
            glTextureStorage2D(
                glTexture, hOrigin->MipLevels, toGlInternalFormat(hOrigin->Format), hOrigin->Width, hOrigin->Height);
            break;
        case eTextureType::Texture3D:
            glTextureStorage3D(glTexture,
                               hOrigin->MipLevels,
                               toGlInternalFormat(hOrigin->Format),
                               hOrigin->Width,
                               hOrigin->Height,
                               hOrigin->Depth);
            break;
        case eTextureType::Texture1DArray:
            glTextureStorage2D(
                glTexture, hOrigin->MipLevels, toGlInternalFormat(hOrigin->Format), hOrigin->Width, hOrigin->ArraySize);
            break;
        case eTextureType::Texture2DArray:
            glTextureStorage3D(glTexture,
                               hOrigin->MipLevels,
                               toGlInternalFormat(hOrigin->Format),
                               hOrigin->Width,
                               hOrigin->Height,
                               hOrigin->ArraySize);
            break;
        case eTextureType::TextureCubeMapArray:
            glTextureStorage3D(glTexture,
                               hOrigin->MipLevels,
                               toGlInternalFormat(hOrigin->Format),
                               hOrigin->Width,
                               hOrigin->Height,
                               hOrigin->ArraySize * 6);
            break;
        default:
            HO_ASSERT(false, "Invalid texture type.");
            glDeleteTextures(1, &glTexture);
            return eResourceTransferResult::Failed;
    }

    if (!ASSERT_GL())
    {
        glDeleteTextures(1, &glTexture);
        return eResourceTransferResult::Failed;
    }

    for (int32_t mipLevel = 0; mipLevel < hOrigin->MipLevels; ++mipLevel)
    {
        const int32_t mipWidth = math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(hOrigin->Width) >> mipLevel));
        const int32_t mipHeight =
            math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(hOrigin->Height) >> mipLevel));
        const int32_t mipDepth = math::Max(1, static_cast<int32_t>(hOrigin->Depth >> mipLevel));

        if (texutils::isUncompressedTextureFormat(hOrigin->Format))
        {
            switch (hOrigin->Type)
            {
                case eTextureType::Texture1D:
                    glTextureSubImage1D(glTexture,
                                        mipLevel,
                                        0,
                                        mipWidth,
                                        toGlExternalFormat(hOrigin->Format),
                                        toGlExternalType(hOrigin->Format),
                                        hOrigin->DataBlob.Data() + hOrigin->Layouts[mipLevel].Offset);
                    break;
                case eTextureType::Texture2D:
                    glTextureSubImage2D(glTexture,
                                        mipLevel,
                                        0,
                                        0,
                                        mipWidth,
                                        mipHeight,
                                        toGlExternalFormat(hOrigin->Format),
                                        toGlExternalType(hOrigin->Format),
                                        hOrigin->DataBlob.Data() + hOrigin->Layouts[mipLevel].Offset);
                    break;
                case eTextureType::TextureCubeMap:
                    glTextureSubImage3D(glTexture,
                                        mipLevel,
                                        0,
                                        0,
                                        0,
                                        mipWidth,
                                        mipHeight,
                                        6,
                                        toGlExternalFormat(hOrigin->Format),
                                        toGlExternalType(hOrigin->Format),
                                        hOrigin->DataBlob.Data() + hOrigin->Layouts[mipLevel].Offset);
                    break;
                case eTextureType::Texture3D:
                    glTextureSubImage3D(glTexture,
                                        mipLevel,
                                        0,
                                        0,
                                        0,
                                        mipWidth,
                                        mipHeight,
                                        mipDepth,
                                        toGlExternalFormat(hOrigin->Format),
                                        toGlExternalType(hOrigin->Format),
                                        hOrigin->DataBlob.Data() + hOrigin->Layouts[mipLevel].Offset);
                    break;
                case eTextureType::Texture1DArray:
                    for (int32_t arrIdx = 0; arrIdx < hOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * hOrigin->MipLevels) + mipLevel;
                        glTextureSubImage2D(glTexture,
                                            mipLevel,
                                            0,
                                            arrIdx,
                                            mipWidth,
                                            1,
                                            toGlExternalFormat(hOrigin->Format),
                                            toGlExternalType(hOrigin->Format),
                                            hOrigin->DataBlob.Data() + hOrigin->Layouts[linearIdx].Offset);
                    }
                    break;
                case eTextureType::Texture2DArray:
                    for (int32_t arrIdx = 0; arrIdx < hOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * hOrigin->MipLevels) + mipLevel;
                        glTextureSubImage3D(glTexture,
                                            mipLevel,
                                            0,
                                            0,
                                            arrIdx,
                                            mipWidth,
                                            mipHeight,
                                            1,
                                            toGlExternalFormat(hOrigin->Format),
                                            toGlExternalType(hOrigin->Format),
                                            hOrigin->DataBlob.Data() + hOrigin->Layouts[linearIdx].Offset);
                    }
                    break;
                case eTextureType::TextureCubeMapArray:
                    for (int32_t arrIdx = 0; arrIdx < hOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * hOrigin->MipLevels) + mipLevel;

                        glTextureSubImage3D(glTexture,
                                            mipLevel,
                                            0,
                                            0,
                                            arrIdx * 6,
                                            mipWidth,
                                            mipHeight,
                                            6,
                                            toGlExternalFormat(hOrigin->Format),
                                            toGlExternalType(hOrigin->Format),
                                            hOrigin->DataBlob.Data() + hOrigin->Layouts[linearIdx].Offset);
                    }
                    break;
                default:
                    HO_ASSERT(false, "Invalid texture type.");
                    glDeleteTextures(1, &glTexture);
                    return eResourceTransferResult::Failed;
            }
        }
        else
        {
            switch (hOrigin->Type)
            {
                case eTextureType::Texture1D:
                    glCompressedTextureSubImage1D(glTexture,
                                                  mipLevel,
                                                  0,
                                                  mipWidth,
                                                  toGlExternalFormat(hOrigin->Format),
                                                  hOrigin->Layouts[mipLevel].Size,
                                                  hOrigin->DataBlob.Data() + hOrigin->Layouts[mipLevel].Offset);
                    break;
                case eTextureType::Texture2D:
                    glCompressedTextureSubImage2D(glTexture,
                                                  mipLevel,
                                                  0,
                                                  0,
                                                  mipWidth,
                                                  mipHeight,
                                                  toGlExternalFormat(hOrigin->Format),
                                                  hOrigin->Layouts[mipLevel].Size,
                                                  hOrigin->DataBlob.Data() + hOrigin->Layouts[mipLevel].Offset);
                    break;
                case eTextureType::TextureCubeMap:
                    glCompressedTextureSubImage3D(glTexture,
                                                  mipLevel,
                                                  0,
                                                  0,
                                                  0,
                                                  mipWidth,
                                                  mipHeight,
                                                  6,
                                                  toGlExternalFormat(hOrigin->Format),
                                                  hOrigin->Layouts[mipLevel].Size,
                                                  hOrigin->DataBlob.Data() + hOrigin->Layouts[mipLevel].Offset);
                    break;
                case eTextureType::Texture3D:
                    glCompressedTextureSubImage3D(glTexture,
                                                  mipLevel,
                                                  0,
                                                  0,
                                                  0,
                                                  mipWidth,
                                                  mipHeight,
                                                  mipDepth,
                                                  toGlExternalFormat(hOrigin->Format),
                                                  hOrigin->Layouts[mipLevel].Size,
                                                  hOrigin->DataBlob.Data() + hOrigin->Layouts[mipLevel].Offset);
                    break;
                case eTextureType::Texture1DArray:
                    for (int32_t arrIdx = 0; arrIdx < hOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * hOrigin->MipLevels) + mipLevel;
                        glCompressedTextureSubImage2D(glTexture,
                                                      mipLevel,
                                                      0,
                                                      arrIdx,
                                                      mipWidth,
                                                      1,
                                                      toGlExternalFormat(hOrigin->Format),
                                                      hOrigin->Layouts[linearIdx].Size,
                                                      hOrigin->DataBlob.Data() + hOrigin->Layouts[linearIdx].Offset);
                    }
                    break;
                case eTextureType::Texture2DArray:
                    for (int32_t arrIdx = 0; arrIdx < hOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * hOrigin->MipLevels) + mipLevel;
                        glCompressedTextureSubImage3D(glTexture,
                                                      mipLevel,
                                                      0,
                                                      0,
                                                      arrIdx,
                                                      mipWidth,
                                                      mipHeight,
                                                      1,
                                                      toGlExternalFormat(hOrigin->Format),
                                                      hOrigin->Layouts[linearIdx].Size,
                                                      hOrigin->DataBlob.Data() + hOrigin->Layouts[linearIdx].Offset);
                    }
                    break;
                case eTextureType::TextureCubeMapArray:
                    for (int32_t arrIdx = 0; arrIdx < hOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * hOrigin->MipLevels) + mipLevel;
                        glCompressedTextureSubImage3D(glTexture,
                                                      mipLevel,
                                                      0,
                                                      0,
                                                      arrIdx * 6,
                                                      mipWidth,
                                                      mipHeight,
                                                      6,
                                                      toGlExternalFormat(hOrigin->Format),
                                                      hOrigin->Layouts[linearIdx].Size,
                                                      hOrigin->DataBlob.Data() + hOrigin->Layouts[linearIdx].Offset);
                    }
                    break;
                default:
                    HO_ASSERT(false, "Invalid texture type.");
                    glDeleteTextures(1, &glTexture);
                    return eResourceTransferResult::Failed;
            }
        }
        if (!ASSERT_GL())
        {
            glDeleteTextures(1, &glTexture);
            return eResourceTransferResult::Failed;
        }
    }

    glTextureParameteri(glTexture, GL_TEXTURE_MAX_LEVEL, hOrigin->MipLevels - 1);

    if (!ASSERT_GL())
    {
        glDeleteTextures(1, &glTexture);
        return eResourceTransferResult::Failed;
    }

    TextureNativeHandleGL nativeHandle;
    nativeHandle.GlTexture = glTexture;
    hPreparedGpuTexture->NativeHandlePoolID = mTextureNativeHandlePool.Add(nativeHandle);
    hPreparedGpuTexture->LastUsedFrame = mCurrentState.CurrentFrame;
    hPreparedGpuTexture->State = eGpuResourceState::UpToDate;
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::UploadShader(GpuShaderHandle hPreparedGpuShader)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hPreparedGpuShader.IsValid(),
              "Invalid gpu shader handle. It might be not created by 'prepareUploadShader' yet.");
    HO_ASSERT(hPreparedGpuShader->State == eGpuResourceState::PendingUpload,
              "PrepareUploadShader must be called before upload.");

    if (hPreparedGpuShader->hOrigin.IsNULL())
    {
        HO_ASSERT(false, "Invalid shader asset. It might be removed from asset system while pending.");
        return eResourceTransferResult::Failed;
    }

    const ObjectHandle<const RShaderAsset> hOrigin = hPreparedGpuShader->hOrigin;

    const GLuint glShader = glCreateShader(toGlShaderType(hOrigin->Stage));
    if (!ASSERT_GL())
    {
        glDeleteShader(glShader);
        return eResourceTransferResult::Failed;
    }
    glShaderBinary(1,
                   &glShader,
                   GL_SHADER_BINARY_FORMAT_SPIR_V,
                   hOrigin->Binary.Data(),
                   static_cast<GLsizei>(hOrigin->Binary.GetSize() * sizeof(uint32_t)));
    if (!ASSERT_GL())
    {
        glDeleteShader(glShader);
        return eResourceTransferResult::Failed;
    }
    glSpecializeShader(glShader, "main", 0, nullptr, nullptr);

    GLint isCompiled = 0;
    glGetShaderiv(glShader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        HO_ASSERT(false, "Shader is not compiled.");
        glDeleteShader(glShader);
        return eResourceTransferResult::Failed;
    }

    const GLuint glProgram = glCreateProgram();

    glProgramParameteri(glProgram, GL_PROGRAM_SEPARABLE, GL_TRUE);

    glAttachShader(glProgram, glShader);
    glLinkProgram(glProgram);

    GLint isLinked = 0;
    glGetProgramiv(glProgram, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        std::string glLogInfo;
        GLint glLogSize = 0;
        glGetProgramiv(glProgram, GL_INFO_LOG_LENGTH, &glLogSize);
        glLogInfo.resize(glLogSize);
        glGetProgramInfoLog(glProgram, glLogSize, nullptr, glLogInfo.data());
        HO_ASSERT(false, "Failed to link shader.");
        glDeleteShader(glShader);
        glDeleteProgram(glProgram);
        return eResourceTransferResult::Failed;
    }

    glDetachShader(glProgram, glShader);
    glDeleteShader(glShader);

    ShaderNativeHandleGL nativeHandle;
    nativeHandle.GlProgram = glProgram;
    hPreparedGpuShader->NativeHandlePoolID = mShaderNativeHandlePool.Add(nativeHandle);
    hPreparedGpuShader->LastUsedFrame = mCurrentState.CurrentFrame;
    hPreparedGpuShader->State = eGpuResourceState::UpToDate;
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::UploadMorphTargets(
    GpuMorphTargetsHandle hPreparedGpuMorphTargets)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hPreparedGpuMorphTargets.IsValid(),
              "Invalid gpu morph targets handle. It might be not created by 'prepareUploadMorphTargets' yet.");
    HO_ASSERT(hPreparedGpuMorphTargets->State == eGpuResourceState::PendingUpload,
              "PrepareUploadMorphTargets must be called before upload.");

    if (hPreparedGpuMorphTargets->hOrigin.IsNULL())
    {
        HO_ASSERT(false, "Invalid morph targets asset. It might be removed from asset system while pending.");
        return eResourceTransferResult::Failed;
    }

    const ObjectHandle<const RMorphTargetsAsset> hOrigin = hPreparedGpuMorphTargets->hOrigin;

    int32_t allMorphTargetCount = 0;
    for (const auto& subMeshMorphTargets : hOrigin->SubMeshMorphTargets)
    {
        allMorphTargetCount += subMeshMorphTargets.Targets.GetSize();
    }

    int32_t targetCount = 0;
    FixedArray<int32_t> subMeshBaseTargetOffset((hOrigin->SubMeshMorphTargets.end() - 1)->SubMeshIndex + 1, -1);
    FixedArray<MorphTarget> morphTargets(allMorphTargetCount);
    for (const auto& subMeshMorphTargets : hOrigin->SubMeshMorphTargets)
    {
        subMeshBaseTargetOffset[subMeshMorphTargets.SubMeshIndex] = targetCount;
        for (const auto& morphTarget : subMeshMorphTargets.Targets)
        {
            morphTargets[targetCount].PositionOffset = morphTarget.PositionOffset;
            morphTargets[targetCount].NormalOffset = morphTarget.NormalOffset;
            morphTargets[targetCount].TangentOffset = morphTarget.TangentOffset;
            ++targetCount;
        }
    }

    GLuint glMorphTargetsSSBO;
    glCreateBuffers(1, &glMorphTargetsSSBO);
    glNamedBufferStorage(
        glMorphTargetsSSBO, static_cast<int32_t>(sizeof(MorphTarget)) * morphTargets.GetSize(), morphTargets.Data(), 0);
    if (!ASSERT_GL())
    {
        glDeleteBuffers(1, &glMorphTargetsSSBO);
        return eResourceTransferResult::Failed;
    }

    const int32_t alignedPositionSize = HO_ALIGN_CEIL(sizeof(Vector3) * hOrigin->Positions.GetSize(), sAlignmentSSBO);
    const int32_t alignedNormalSize = HO_ALIGN_CEIL(sizeof(Vector3) * hOrigin->Normals.GetSize(), sAlignmentSSBO);
    const int32_t alignedTangentSize = HO_ALIGN_CEIL(sizeof(Vector4) * hOrigin->Tangents.GetSize(), sAlignmentSSBO);

    FixedArray<uint8_t> flattenTargets(alignedPositionSize + alignedNormalSize + alignedTangentSize);
    std::memcpy(flattenTargets.Data(), hOrigin->Positions.Data(), sizeof(Vector3) * hOrigin->Positions.GetSize());
    std::memcpy(flattenTargets.Data() + alignedPositionSize,
                hOrigin->Normals.Data(),
                sizeof(Vector3) * hOrigin->Normals.GetSize());
    std::memcpy(flattenTargets.Data() + alignedPositionSize + alignedNormalSize,
                hOrigin->Tangents.Data(),
                sizeof(Vector4) * hOrigin->Tangents.GetSize());

    GLuint glTargetAttributesSSBO;
    glCreateBuffers(1, &glTargetAttributesSSBO);
    glNamedBufferStorage(
        glTargetAttributesSSBO, alignedPositionSize + alignedNormalSize + alignedTangentSize, flattenTargets.Data(), 0);
    if (!ASSERT_GL())
    {
        glDeleteBuffers(1, &glMorphTargetsSSBO);
        glDeleteBuffers(1, &glTargetAttributesSSBO);
        return eResourceTransferResult::Failed;
    }

    MorphTargetsNativeHandleGL nativeHandle{};
    nativeHandle.GlMorphTargetsSSBO = glMorphTargetsSSBO;
    nativeHandle.GlTargetAttributesSSBO = glTargetAttributesSSBO;
    hPreparedGpuMorphTargets->NativeHandlePoolID = mMorphTargetsNativeHandlePool.Add(nativeHandle);
    hPreparedGpuMorphTargets->AllPositionByteOffset = 0;
    hPreparedGpuMorphTargets->AllPositionByteSize =
        static_cast<int32_t>(sizeof(Vector3)) * hOrigin->Positions.GetSize();
    hPreparedGpuMorphTargets->AllNormalByteOffset = alignedPositionSize;
    hPreparedGpuMorphTargets->AllNormalByteSize = static_cast<int32_t>(sizeof(Vector3)) * hOrigin->Normals.GetSize();
    hPreparedGpuMorphTargets->AllTangentByteOffset = alignedPositionSize + alignedNormalSize;
    hPreparedGpuMorphTargets->AllTangentByteSize = static_cast<int32_t>(sizeof(Vector4)) * hOrigin->Tangents.GetSize();
    hPreparedGpuMorphTargets->SubMeshBaseTargetOffset = std::move(subMeshBaseTargetOffset);
    hPreparedGpuMorphTargets->LastUsedFrame = mCurrentState.CurrentFrame;
    hPreparedGpuMorphTargets->State = eGpuResourceState::UpToDate;
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::DownloadBufferData(GpuBufferHandle hGpuBuffer,
                                                                                  void* pDstData,
                                                                                  int32_t size,
                                                                                  int32_t offset)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hGpuBuffer.IsValid() && hGpuBuffer->State != eGpuResourceState::PendingUpload,
              "Invalid gpu buffer handle. It might be not created by 'CreateBuffer' yet.");
    HO_ASSERT(offset + size <= hGpuBuffer->ByteSize, "Out of range.");
    HO_ASSERT(pDstData, "Downloading data can't be null.");
    HO_ASSERT(hGpuBuffer->bAllocated, "Buffer is not allocated.");
    HO_ASSERT(hGpuBuffer->Access == eGpuBufferAccess::ReadBack, "Only read back buffer can be downloaded.");

    const GLuint glBuffer = mBufferNativeHandlePool.Get(hGpuBuffer->NativeHandlePoolID)->GlBuffer;
    glGetNamedBufferSubData(glBuffer, offset, size, pDstData);

    if (!ASSERT_GL())
    {
        return eResourceTransferResult::Failed;
    }
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::DownloadTexture(GpuTextureHandle hGpuTexture,
                                                                               ObjectHandle<RTextureAsset> hDstTexture)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hGpuTexture.IsValid(),
              "Invalid gpu texture handle. It might be not created by 'prepareUploadTexture' or "
              "'prepareCreatedTexture' yet.");
    HO_ASSERT(hGpuTexture->State != eGpuResourceState::PendingUpload, "Texture must be uploaded before download.");
    HO_ASSERT(hDstTexture.IsValid(), "Invalid texture asset handle. Destination texture must be created.");
    HO_ASSERT(hDstTexture->DataBlob.GetSize() == 0 && hDstTexture->hRenderProxy.IsNULL(),
              "Destination texture must be emtpy. It can have name only.");
    HO_ASSERT(hDstTexture->TransferState == eAssetTransferState::PendingDownload,
              "Destination asset must be in 'PendingDownload' state before download.");

    hDstTexture->Width = hGpuTexture->Width;
    hDstTexture->Height = hGpuTexture->Height;
    hDstTexture->Depth = hGpuTexture->Depth;
    hDstTexture->ArraySize = hGpuTexture->ArraySize;
    hDstTexture->MipLevels = hGpuTexture->MipLevels;

    hDstTexture->Format = hGpuTexture->Format;
    hDstTexture->Type = hGpuTexture->Type;

    // Construct sub texture layouts
    int32_t totalDataSize = 0;
    hDstTexture->Layouts = FixedArray<RTextureAsset::SubTextureLayout>(hGpuTexture->MipLevels * hGpuTexture->ArraySize);

    int32_t linearIdx = 0;
    for (int32_t mipLevel = 0; mipLevel < hGpuTexture->MipLevels; ++mipLevel)
    {
        const int32_t mipWidth =
            math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(hGpuTexture->Width) >> mipLevel));
        const int32_t mipHeight =
            math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(hGpuTexture->Height) >> mipLevel));
        const int32_t mipDepth = math::Max(1, static_cast<int32_t>(hGpuTexture->Depth >> mipLevel));
        for (int32_t arrIdx = 0; arrIdx < hGpuTexture->ArraySize; ++arrIdx)
        {
            hDstTexture->Layouts[linearIdx].Offset = totalDataSize;

            if (texutils::isUncompressedTextureFormat(hGpuTexture->Format))
            {
                hDstTexture->Layouts[linearIdx].RowPitch =
                    mipWidth * texutils::getTexturePixelBytes(hGpuTexture->Format);
                hDstTexture->Layouts[linearIdx].Size = mipDepth * mipHeight * hDstTexture->Layouts[linearIdx].RowPitch;
            }
            else
            {
                hDstTexture->Layouts[linearIdx].RowPitch =
                    texutils::getTextureBlockCount(mipWidth) * texutils::getTextureBlockBytes(hGpuTexture->Format);
                hDstTexture->Layouts[linearIdx].Size =
                    mipDepth * texutils::getTextureBlockCount(mipHeight) * hDstTexture->Layouts[linearIdx].RowPitch;
            }

            if (hGpuTexture->Type == eTextureType::TextureCubeMap ||
                hGpuTexture->Type == eTextureType::TextureCubeMapArray)
            {
                hDstTexture->Layouts[linearIdx].Size *= 6;
            }

            totalDataSize += hDstTexture->Layouts[linearIdx].Size;
            ++linearIdx;
        }
    }

    hDstTexture->DataBlob = FixedArray<uint8_t>(totalDataSize);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if (!ASSERT_GL())
    {
        hDstTexture->TransferState = eAssetTransferState::Failed;
        return eResourceTransferResult::Failed;
    }

    linearIdx = 0;
    const GLuint glTexture = mTextureNativeHandlePool.Get(hGpuTexture->NativeHandlePoolID)->GlTexture;
    for (int32_t mipLevel = 0; mipLevel < hGpuTexture->MipLevels; ++mipLevel)
    {
        const int32_t mipWidth =
            math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(hGpuTexture->Width) >> mipLevel));
        const int32_t mipHeight =
            math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(hGpuTexture->Height) >> mipLevel));
        int32_t mipDepth = math::Max(1, static_cast<int32_t>(hGpuTexture->Depth >> mipLevel));

        for (int32_t arrIdx = 0; arrIdx < hGpuTexture->ArraySize; ++arrIdx)
        {
            int32_t yOffset = 0;
            int32_t zOffset = 0;
            switch (hGpuTexture->Type)
            {
                case eTextureType::Texture1D:
                case eTextureType::Texture2D:
                case eTextureType::Texture3D:
                    yOffset = 0;
                    zOffset = 0;
                    break;
                case eTextureType::Texture1DArray:
                    yOffset = arrIdx;
                    zOffset = 0;
                    break;
                case eTextureType::Texture2DArray:
                    yOffset = 0;
                    zOffset = arrIdx;
                    break;
                case eTextureType::TextureCubeMap:
                case eTextureType::TextureCubeMapArray:
                    yOffset = 0;
                    zOffset = arrIdx * 6;
                    mipDepth = 6;
                    break;
                default:
                    HO_ASSERT(false, "Invalid texture type.");
            }

            if (texutils::isUncompressedTextureFormat(hGpuTexture->Format))
            {
                glGetTextureSubImage(glTexture,
                                     mipLevel,
                                     0,
                                     yOffset,
                                     zOffset,
                                     mipWidth,
                                     mipHeight,
                                     mipDepth,
                                     toGlExternalFormat(hDstTexture->Format),
                                     toGlExternalType(hDstTexture->Format),
                                     hDstTexture->Layouts[linearIdx].Size,
                                     hDstTexture->DataBlob.Data() + hDstTexture->Layouts[linearIdx].Offset);
            }
            else
            {
                glGetCompressedTextureSubImage(glTexture,
                                               mipLevel,
                                               0,
                                               yOffset,
                                               zOffset,
                                               mipWidth,
                                               mipHeight,
                                               mipDepth,
                                               hDstTexture->Layouts[linearIdx].Size,
                                               hDstTexture->DataBlob.Data() + hDstTexture->Layouts[linearIdx].Offset);
            }

            ++linearIdx;
        }
        if (!ASSERT_GL())
        {
            hDstTexture->TransferState = eAssetTransferState::Failed;
            return eResourceTransferResult::Failed;
        }
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    if (!ASSERT_GL())
    {
        hDstTexture->TransferState = eAssetTransferState::Failed;
        return eResourceTransferResult::Failed;
    }

    hDstTexture->TransferState = eAssetTransferState::Ready;
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::UnloadBuffer(GpuBufferHandle hGpuBuffer)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hGpuBuffer->State != eGpuResourceState::PendingUpload, "Gpu buffer is not uploaded yet.");

    if (hGpuBuffer.IsNULL())
    {
        return eResourceTransferResult::Failed;
    }
    const BufferNativeHandleGL* nativeHandle = mBufferNativeHandlePool.Get(hGpuBuffer->NativeHandlePoolID);
    if (!nativeHandle)
    {
        return eResourceTransferResult::Failed;
    }
    glDeleteBuffers(1, &nativeHandle->GlBuffer);
    mBufferNativeHandlePool.Remove(hGpuBuffer->NativeHandlePoolID);
    hGpuBuffer->NativeHandlePoolID = 0;
    finishUnloadBuffer(hGpuBuffer);
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::UnloadStaticMesh(GpuStaticMeshHandle hGpuStaticMesh)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hGpuStaticMesh->State != eGpuResourceState::PendingUpload, "Static mesh is not uploaded yet.");

    if (hGpuStaticMesh.IsNULL())
    {
        return eResourceTransferResult::Failed;
    }
    const StaticMeshNativeHandleGL* nativeHandle = mStaticMeshNativeHandlePool.Get(hGpuStaticMesh->NativeHandlePoolID);
    if (!nativeHandle)
    {
        return eResourceTransferResult::Failed;
    }
    glDeleteBuffers(1, &nativeHandle->GlVBO);
    glDeleteBuffers(1, &nativeHandle->GlEBO);
    mStaticMeshNativeHandlePool.Remove(hGpuStaticMesh->NativeHandlePoolID);
    hGpuStaticMesh->NativeHandlePoolID = 0;
    finishUnloadStaticMesh(hGpuStaticMesh);
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::UnloadTexture(GpuTextureHandle hGpuTexture)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hGpuTexture->State != eGpuResourceState::PendingUpload, "Texture is not uploaded yet.");

    if (hGpuTexture.IsNULL())
    {
        return eResourceTransferResult::Failed;
    }
    const TextureNativeHandleGL* nativeHandle = mTextureNativeHandlePool.Get(hGpuTexture->NativeHandlePoolID);
    if (!nativeHandle)
    {
        return eResourceTransferResult::Failed;
    }
    glDeleteTextures(1, &nativeHandle->GlTexture);
    mTextureNativeHandlePool.Remove(hGpuTexture->NativeHandlePoolID);
    hGpuTexture->NativeHandlePoolID = 0;
    finishUnloadTexture(hGpuTexture);
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::UnloadShader(GpuShaderHandle hGpuShader)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hGpuShader->State != eGpuResourceState::PendingUpload, "Shader is not uploaded yet.");

    if (hGpuShader.IsNULL())
    {
        return eResourceTransferResult::Failed;
    }
    const ShaderNativeHandleGL* nativeHandle = mShaderNativeHandlePool.Get(hGpuShader->NativeHandlePoolID);
    if (!nativeHandle)
    {
        return eResourceTransferResult::Failed;
    }
    glDeleteProgram(nativeHandle->GlProgram);
    mShaderNativeHandlePool.Remove(hGpuShader->NativeHandlePoolID);
    hGpuShader->NativeHandlePoolID = 0;
    finishUnloadShader(hGpuShader);
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult RenderingBackendGL::UnloadMorphTargets(
    GpuMorphTargetsHandle hGpuMorphTargets)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hGpuMorphTargets->State != eGpuResourceState::PendingUpload, "Morph targets is not uploaded yet.");

    if (hGpuMorphTargets.IsNULL())
    {
        return eResourceTransferResult::Failed;
    }
    const MorphTargetsNativeHandleGL* nativeHandle =
        mMorphTargetsNativeHandlePool.Get(hGpuMorphTargets->NativeHandlePoolID);
    if (!nativeHandle)
    {
        return eResourceTransferResult::Failed;
    }

    glDeleteBuffers(1, &nativeHandle->GlMorphTargetsSSBO);
    glDeleteBuffers(1, &nativeHandle->GlTargetAttributesSSBO);

    mMorphTargetsNativeHandlePool.Remove(hGpuMorphTargets->NativeHandlePoolID);
    hGpuMorphTargets->NativeHandlePoolID = 0;
    finishUnloadMorphTargets(hGpuMorphTargets);
    return eResourceTransferResult::Success;
}

bool RenderingBackendGL::initializeSamplers()
{
    GLuint materialTextureSampler = 0;
    glCreateSamplers(1, &materialTextureSampler);
    glSamplerParameteri(materialTextureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(materialTextureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(materialTextureSampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(materialTextureSampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (!ASSERT_GL())
    {
        glDeleteSamplers(1, &materialTextureSampler);
        return false;
    }

    GLuint dirLightShadowMapSampler = 0;
    glCreateSamplers(1, &dirLightShadowMapSampler);
    glSamplerParameteri(dirLightShadowMapSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(dirLightShadowMapSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glSamplerParameteri(dirLightShadowMapSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(dirLightShadowMapSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(dirLightShadowMapSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glSamplerParameterfv(dirLightShadowMapSampler, GL_TEXTURE_BORDER_COLOR, borderColor);

    glSamplerParameteri(dirLightShadowMapSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glSamplerParameteri(dirLightShadowMapSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    if (!ASSERT_GL())
    {
        glDeleteSamplers(1, &materialTextureSampler);
        glDeleteSamplers(1, &dirLightShadowMapSampler);
        return false;
    }

    GLuint pointLightShadowMapSampler = 0;
    glCreateSamplers(1, &pointLightShadowMapSampler);
    glSamplerParameteri(pointLightShadowMapSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(pointLightShadowMapSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glSamplerParameteri(pointLightShadowMapSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(pointLightShadowMapSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(pointLightShadowMapSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (!ASSERT_GL())
    {
        glDeleteSamplers(1, &materialTextureSampler);
        glDeleteSamplers(1, &dirLightShadowMapSampler);
        glDeleteSamplers(1, &pointLightShadowMapSampler);
        return false;
    }

    GLuint spotLightShadowMapSampler = 0;
    glCreateSamplers(1, &spotLightShadowMapSampler);
    glSamplerParameteri(spotLightShadowMapSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(spotLightShadowMapSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glSamplerParameteri(spotLightShadowMapSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(spotLightShadowMapSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(spotLightShadowMapSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glSamplerParameterfv(spotLightShadowMapSampler, GL_TEXTURE_BORDER_COLOR, borderColor);

    if (!ASSERT_GL())
    {
        glDeleteSamplers(1, &materialTextureSampler);
        glDeleteSamplers(1, &dirLightShadowMapSampler);
        glDeleteSamplers(1, &pointLightShadowMapSampler);
        glDeleteSamplers(1, &spotLightShadowMapSampler);
        return false;
    }

    GLuint iblSampler = 0;
    glCreateSamplers(1, &iblSampler);
    glSamplerParameteri(iblSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(iblSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(iblSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(iblSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(iblSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (!ASSERT_GL())
    {
        glDeleteSamplers(1, &materialTextureSampler);
        glDeleteSamplers(1, &dirLightShadowMapSampler);
        glDeleteSamplers(1, &pointLightShadowMapSampler);
        glDeleteSamplers(1, &spotLightShadowMapSampler);
        glDeleteSamplers(1, &iblSampler);
        return false;
    }

    mMaterialTextureSampler = materialTextureSampler;
    mDirLightShadowMapSampler = dirLightShadowMapSampler;
    mPointLightShadowMapSampler = pointLightShadowMapSampler;
    mSpotLightShadowMapSampler = spotLightShadowMapSampler;
    mIBLSampler = iblSampler;
    return true;
}

GLenum toGlFunc(IRenderingBackend::eComparisonFunc func)
{
    using eComparisonFunc = IRenderingBackend::eComparisonFunc;

    switch (func)
    {
        case eComparisonFunc::Never:
            return GL_NEVER;
        case eComparisonFunc::Less:
            return GL_LESS;
        case eComparisonFunc::Equal:
            return GL_EQUAL;
        case eComparisonFunc::LEqual:
            return GL_LEQUAL;
        case eComparisonFunc::Greater:
            return GL_GREATER;
        case eComparisonFunc::NotEqual:
            return GL_NOTEQUAL;
        case eComparisonFunc::GEqual:
            return GL_GEQUAL;
        case eComparisonFunc::Always:
            return GL_ALWAYS;
    }

    return GL_ALWAYS;
}

GLenum toGlStencilOp(IRenderingBackend::eStencilOperator op)
{
    using eStencilOperator = IRenderingBackend::eStencilOperator;

    switch (op)
    {
        case eStencilOperator::Keep:
            return GL_KEEP;
        case eStencilOperator::Zero:
            return GL_ZERO;
        case eStencilOperator::Replace:
            return GL_REPLACE;
        case eStencilOperator::IncreaseWrap:
            return GL_INCR_WRAP;
        case eStencilOperator::IncreaseClamp:
            return GL_INCR;
        case eStencilOperator::DecreaseWrap:
            return GL_DECR_WRAP;
        case eStencilOperator::DecreaseClamp:
            return GL_DECR;
        case eStencilOperator::Invert:
            return GL_INVERT;
    }

    return GL_KEEP;
}

bool isStorableTextureFormat(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::B8G8R8A8_UNORM:
        case eTextureFormat::R16_FLOAT:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R16G16B16A16_FLOAT:
        case eTextureFormat::R32_FLOAT:
        case eTextureFormat::R32G32_FLOAT:
        case eTextureFormat::R32G32B32A32_FLOAT:
            return true;

        default:
            return false;
    }
}

GLenum toGlTextureType(eTextureType type, int32_t sampleCount)
{
    switch (type)
    {
        case eTextureType::Texture1D:
            return GL_TEXTURE_1D;
        case eTextureType::Texture2D:
            return sampleCount == 1 ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE;
        case eTextureType::Texture3D:
            return GL_TEXTURE_3D;
        case eTextureType::TextureCubeMap:
            return GL_TEXTURE_CUBE_MAP;
        case eTextureType::Texture1DArray:
            return GL_TEXTURE_1D_ARRAY;
        case eTextureType::Texture2DArray:
            return sampleCount == 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
        case eTextureType::TextureCubeMapArray:
            return GL_TEXTURE_CUBE_MAP_ARRAY;
        default:
            HO_ASSERT(false, "Invalid texture type.");
            return GL_NONE;
    }
}

GLenum toGlInternalFormat(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
            return GL_R8;
        case eTextureFormat::R8G8_UNORM:
            return GL_RG8;
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::B8G8R8A8_UNORM:
            return GL_RGBA8;
        case eTextureFormat::R8_SRGB:
        case eTextureFormat::R8G8_SRGB:
            return GL_SRGB8;
        case eTextureFormat::R8G8B8A8_SRGB:
            return GL_SRGB8_ALPHA8;

        case eTextureFormat::R16_FLOAT:
            return GL_R16F;
        case eTextureFormat::R16G16_FLOAT:
            return GL_RG16F;
        case eTextureFormat::R16G16B16A16_FLOAT:
            return GL_RGBA16F;

        case eTextureFormat::R32_FLOAT:
            return GL_R32F;
        case eTextureFormat::R32G32_FLOAT:
            return GL_RG32F;
        case eTextureFormat::R32G32B32A32_FLOAT:
            return GL_RGBA32F;

        case eTextureFormat::D24_UNORM_S8_UINT:
            return GL_DEPTH24_STENCIL8;
        case eTextureFormat::D32_FLOAT:
            return GL_DEPTH_COMPONENT32F;

        case eTextureFormat::BC1_UNORM:
            return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case eTextureFormat::BC1_SRGB:
            return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
        case eTextureFormat::BC3_UNORM:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case eTextureFormat::BC3_SRGB:
            return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
        case eTextureFormat::BC4_UNORM:
            return GL_COMPRESSED_RED_RGTC1;
        case eTextureFormat::BC4_SNORM:
            return GL_COMPRESSED_SIGNED_RED_RGTC1;
        case eTextureFormat::BC5_UNORM:
            return GL_COMPRESSED_RG_RGTC2;
        case eTextureFormat::BC5_SNORM:
            return GL_COMPRESSED_SIGNED_RG_RGTC2;
        case eTextureFormat::BC6H_UF16:
            return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
        case eTextureFormat::BC6H_SF16:
            return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
        case eTextureFormat::BC7_UNORM:
            return GL_COMPRESSED_RGBA_BPTC_UNORM;
        case eTextureFormat::BC7_SRGB:
            return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;

        case eTextureFormat::None:
        default:
            HO_ASSERT(false, "Invalid texture format.");
            return GL_NONE;
    }
}

GLenum toGlExternalFormat(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8_SRGB:
        case eTextureFormat::R16_FLOAT:
        case eTextureFormat::R32_FLOAT:
            return GL_RED;

        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R32G32_FLOAT:
            return GL_RG;

        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::R8G8B8A8_SRGB:
        case eTextureFormat::R16G16B16A16_FLOAT:
        case eTextureFormat::R32G32B32A32_FLOAT:
            return GL_RGBA;

        case eTextureFormat::B8G8R8A8_UNORM:
            return GL_BGRA;

        case eTextureFormat::D24_UNORM_S8_UINT:
            return GL_DEPTH_STENCIL;

        case eTextureFormat::D32_FLOAT:
            return GL_DEPTH_COMPONENT;

        case eTextureFormat::BC1_UNORM:
            return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case eTextureFormat::BC1_SRGB:
            return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
        case eTextureFormat::BC3_UNORM:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case eTextureFormat::BC3_SRGB:
            return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
        case eTextureFormat::BC4_UNORM:
            return GL_COMPRESSED_RED_RGTC1;
        case eTextureFormat::BC4_SNORM:
            return GL_COMPRESSED_SIGNED_RED_RGTC1;
        case eTextureFormat::BC5_UNORM:
            return GL_COMPRESSED_RG_RGTC2;
        case eTextureFormat::BC5_SNORM:
            return GL_COMPRESSED_SIGNED_RG_RGTC2;
        case eTextureFormat::BC6H_UF16:
            return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
        case eTextureFormat::BC6H_SF16:
            return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
        case eTextureFormat::BC7_UNORM:
            return GL_COMPRESSED_RGBA_BPTC_UNORM;
        case eTextureFormat::BC7_SRGB:
            return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;

        default:
            HO_ASSERT(false, "Invalid or unsupported texture format.");
            return GL_NONE;
    }
}

GLenum toGlExternalType(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::B8G8R8A8_UNORM:
        case eTextureFormat::R8_SRGB:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R8G8B8A8_SRGB:
            return GL_UNSIGNED_BYTE;

        case eTextureFormat::R16_FLOAT:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R16G16B16A16_FLOAT:
            return GL_HALF_FLOAT;

        case eTextureFormat::R32_FLOAT:
        case eTextureFormat::R32G32_FLOAT:
        case eTextureFormat::R32G32B32A32_FLOAT:
        case eTextureFormat::D32_FLOAT:
            return GL_FLOAT;

        case eTextureFormat::D24_UNORM_S8_UINT:
            return GL_UNSIGNED_INT_24_8;

        default:
            HO_ASSERT(false, "Invalid or unsupported texture format.");
            return GL_NONE;
    }
}

GLenum toGlShaderType(eShaderStage stage)
{
    switch (stage)
    {
        case eShaderStage::VertexShader:
            return GL_VERTEX_SHADER;
        case eShaderStage::GeometryShader:
            return GL_GEOMETRY_SHADER;
        case eShaderStage::FragmentShader:
            return GL_FRAGMENT_SHADER;
        case eShaderStage::ComputeShader:
            return GL_COMPUTE_SHADER;
        default:
            HO_ASSERT(false, "Invalid shader stage.");
            return GL_NONE;
    }
}

GLenum toGlAccessType(IRenderingBackend::eResourceAccessType type)
{
    switch (type)
    {
        case IRenderingBackend::eResourceAccessType::ReadOnly:
            return GL_READ_ONLY;
        case IRenderingBackend::eResourceAccessType::WriteOnly:
            return GL_WRITE_ONLY;
        case IRenderingBackend::eResourceAccessType::ReadAndWrite:
            return GL_READ_WRITE;
        default:
            HO_ASSERT(false, "Invalid access type.");
            return GL_NONE;
    }
}

GLbitfield toGlShaderBitfield(eShaderStage stage)
{
    switch (stage)
    {
        case eShaderStage::VertexShader:
            return GL_VERTEX_SHADER_BIT;
        case eShaderStage::GeometryShader:
            return GL_GEOMETRY_SHADER_BIT;
        case eShaderStage::FragmentShader:
            return GL_FRAGMENT_SHADER_BIT;
        case eShaderStage::ComputeShader:
            return GL_COMPUTE_SHADER_BIT;
        default:
            HO_ASSERT(false, "Invalid shader stage.");
            return GL_NONE;
    }
}

// NOLINTBEGIN
// This function no lint for debugging
bool ASSERT_GL()
{
#if defined(DEBUG) || defined(_DEBUG)
    const GLenum glError = glGetError();

    switch (glError)
    {
        case GL_INVALID_ENUM:
            HO_ASSERT(false,
                      "An unacceptable value is specified for an enumerated argument. The offending command is ignored "
                      "and has no other side effect than to set the error flag.");
            return false;
        case GL_INVALID_VALUE:
            HO_ASSERT(false,
                      "A numeric argument is out of range. The offending command is ignored and has no other side "
                      "effect than to set the error flag.");
            return false;
        case GL_INVALID_OPERATION:
            HO_ASSERT(false,
                      "The specified operation is not allowed in the current state. The offending command is ignored "
                      "and has no "
                      "other side effect than to set the error flag.");
            return false;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            HO_ASSERT(false,
                      "The framebuffer object is not complete. The offending command is ignored and has no other side "
                      "effect than to "
                      "set the error flag.");
            return false;
        case GL_OUT_OF_MEMORY:
            HO_ASSERT(false,
                      "There is not enough memory left to execute the command. The state of the GL is undefined, "
                      "except for the "
                      "state of the error flags, after this error is recorded.");
            return false;

        case GL_STACK_UNDERFLOW:
            HO_ASSERT(false,
                      "An attempt has been made to perform an operation that would cause an internal stack to "
                      "underflow.");
            return false;

        case GL_STACK_OVERFLOW:
            HO_ASSERT(
                false,
                "An attempt has been made to perform an operation that would cause an internal stack to overflow.");
            return false;
        default:
            break;
    }
#endif
    return true;
}

// NOLINTEND

GLint RenderingBackendGL::sAlignmentUBO = 0;
GLint RenderingBackendGL::sAlignmentSSBO = 0;
} // namespace ho