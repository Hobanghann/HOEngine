#include "RenderingSystemGL.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include "Core/IO/Path.h"
#include "Core/IO/ResourceParseFuncs.h"
#include "Core/Log/Logger.h"
#include "Platforms/IPlatformApplication.h"
#include "Platforms/Windows/GL/Win32ApplicationGL.h"
#include "Subsystems/Asset/AssetImportFuncs.h"
#include "Subsystems/Asset/Assets.h"

namespace ho
{
static GLenum toGlRenderTargetFormat(IRenderingSystem::eRenderTargetFormat format);
static GLenum toGlInternalFormat(eTextureFormat format);
static GLenum toGlExternalFormat(eTextureFormat format);
static GLenum toGlExternalType(eTextureFormat format);
static bool isUncompressedFormat(eTextureFormat format);
static GLenum toGlTextureType(eTextureType type);
static GLenum toGlShaderType(eShaderStage stage);
[[maybe_unused]] static GLbitfield toGlShaderBitfield(eShaderStage stage);

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

void* RenderingSystemGL::GetRenderTargetNativeHandle(StringHandle hFrameBufferName,
                                                     eRenderTargetType type,
                                                     bool bRequireMultisample) const
{
    auto fbit = mNameToFrameBufferMap.find(hFrameBufferName);

    if (fbit == mNameToFrameBufferMap.end())
    {
        return nullptr;
    }

    const FrameBuffer& frameBuffer = fbit->second;
    HO_ASSERT(frameBuffer.hRenderTargets[mFrontRenderQueueIndex][static_cast<int32_t>(type)].IsValid(),
              "Frame buffer doesn't have such render target.");
    const TextureNativeHandleGL* nativeHandle = nullptr;
    if (bRequireMultisample)
    {
        HO_ASSERT(frameBuffer.SampleCount > 1, "Frame buffer has single sample render targets.");
        nativeHandle =
            mTextureNativeHandlePool.Get(frameBuffer.hRenderTargets[mFrontRenderQueueIndex][static_cast<int32_t>(type)]
                                             .Get()
                                             ->NativeHandlePoolIndex);
    }
    else
    {
        if (frameBuffer.SampleCount > 1)
        {
            HO_ASSERT(type != eRenderTargetType::Depth,
                      "MultiSample Depth Target must be used in rendering system only.");
            nativeHandle = mTextureNativeHandlePool.Get(
                frameBuffer.hResolvedRenderTargets[mFrontRenderQueueIndex][static_cast<int32_t>(type)]
                    .Get()
                    ->NativeHandlePoolIndex);
        }
        else
        {
            nativeHandle = mTextureNativeHandlePool.Get(
                frameBuffer.hRenderTargets[mFrontRenderQueueIndex][static_cast<int32_t>(type)]
                    .Get()
                    ->NativeHandlePoolIndex);
        }
    }

    HO_ASSERT(nativeHandle, "Texture for render target was not created.");
    return reinterpret_cast<void*>(static_cast<uintptr_t>(nativeHandle->GlTexture)); // NOLINT
}

RenderingSystemGL::RenderingSystemGL()
{
    HO_ASSERT(!spInstance, "Rendering system must be created by 'createInstance' function.");
}

bool RenderingSystemGL::init()
{
    if (!spInstance)
    {
        HO_ASSERT(false, "Rendering system is not created.");
        return false;
    }
    else
    {
        // Create default GPU resources.

        // VAO
        glCreateVertexArrays(1, &mGlVAO);

        glVertexArrayAttribFormat(mGlVAO, mPosAttribIndex, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(mGlVAO, mNormalAttribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3));
        glVertexArrayAttribFormat(mGlVAO, mTangentsAttribIndex, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector3));
        glVertexArrayAttribFormat(
            mGlVAO, mUV0AttribIndex, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector3) + sizeof(Vector4));
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
            cancelInitialization();
            return false;
        }

        // Shaders
        Path shaderDirPath(std::string("Subsystems/Rendering/GL/Shaders"));
        shaderDirPath.ResolveProjectPath();

        auto loadShader = [&](const std::string& shaderFileName,
                              GLuint* pGlProgram,
                              eShaderStage stage,
                              eMaterialAssetType compatibleMaterialTypes) -> bool
        {
            const std::unique_ptr<const parser::ShaderIR> pDefaultIR =
                parser::parseShaderFile(shaderDirPath / Path(shaderFileName), shaderFileName);
            const std::unique_ptr<const ShaderAsset> pDefaultAsset =
                importer::importShader(*pDefaultIR, stage, static_cast<uint8_t>(compatibleMaterialTypes));
            const GLuint glShader = glCreateShader(toGlShaderType(pDefaultAsset->Stage));
            if (!ASSERT_GL())
            {
                glDeleteShader(glShader);
                return false;
            }
            glShaderBinary(1,
                           &glShader,
                           GL_SHADER_BINARY_FORMAT_SPIR_V,
                           pDefaultAsset->Binary.Data(),
                           static_cast<GLsizei>(pDefaultAsset->Binary.GetSize() * sizeof(uint32_t)));
            if (!ASSERT_GL())
            {
                glDeleteShader(glShader);
                return false;
            }

            glSpecializeShader(glShader, "main", 0, nullptr, nullptr);
            if (!ASSERT_GL())
            {
                glDeleteShader(glShader);
                return false;
            }

            GLint isCompiled = 0;
            glGetShaderiv(glShader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE)
            {
                HO_ASSERT(false, "Shader is not compiled.");
                glDeleteShader(glShader);
                return false;
            }

            *pGlProgram = glCreateProgram();

            glProgramParameteri(*pGlProgram, GL_PROGRAM_SEPARABLE, GL_TRUE);

            glAttachShader(*pGlProgram, glShader);
            if (!ASSERT_GL())
            {
                glDeleteShader(glShader);
                return false;
            }

            glLinkProgram(*pGlProgram);

            GLint isLinked = 0;
            glGetProgramiv(*pGlProgram, GL_LINK_STATUS, &isLinked);
            if (isLinked == GL_FALSE)
            {
                std::string glLogInfo;
                GLint glLogSize = 0;
                glGetProgramiv(*pGlProgram, GL_INFO_LOG_LENGTH, &glLogSize);
                glLogInfo.resize(glLogSize);
                glGetProgramInfoLog(*pGlProgram, glLogSize, nullptr, glLogInfo.data());
                HO_LOG_ERROR((std::string("Shader Link Failed:\n") + glLogInfo).c_str());
                HO_ASSERT(false, "Failed to link shader.");
                glDeleteShader(glShader);
                *pGlProgram = 0;
                return false;
            }

            glDetachShader(*pGlProgram, glShader);
            glDeleteShader(glShader);
            return true;
        };

        if (!loadShader("Default.vert",
                        &mGlDefaultVS,
                        eShaderStage::VertexShader,
                        eMaterialAssetType::Legacy | eMaterialAssetType::StandardLit))
        {
            cancelInitialization();
            return false;
        }

        if (!loadShader(
                "DefaultPhong.frag", &mGlDefaultPhongFS, eShaderStage::FragmentShader, eMaterialAssetType::Legacy))
        {
            cancelInitialization();
            return false;
        }

        if (!loadShader("DefaultPhongMasked.frag",
                        &mGlDefaultPhongMaskedFS,
                        eShaderStage::FragmentShader,
                        eMaterialAssetType::Legacy))
        {
            cancelInitialization();
            return false;
        }

        if (!loadShader(
                "DefaultPBR.frag", &mGlDefaultPbrFS, eShaderStage::FragmentShader, eMaterialAssetType::StandardLit))
        {
            cancelInitialization();
            return false;
        }

        if (!loadShader("DefaultPBRMasked.frag",
                        &mGlDefaultPbrMaskedFS,
                        eShaderStage::FragmentShader,
                        eMaterialAssetType::StandardLit))
        {
            cancelInitialization();
            return false;
        }

        if (!loadShader(
                "DefaultSkyMap.vert", &mGlDefaultSkyMapVS, eShaderStage::VertexShader, eMaterialAssetType::None))
        {
            cancelInitialization();
            return false;
        }

        if (!loadShader(
                "DefaultSkyMap.frag", &mGlDefaultSkyMapFS, eShaderStage::FragmentShader, eMaterialAssetType::None))
        {
            cancelInitialization();
            return false;
        }

        if (!loadShader(
                "WorldGizmo.vert", &mGlDefaultWorldGizmoVS, eShaderStage::VertexShader, eMaterialAssetType::None))
        {
            cancelInitialization();
            return false;
        }

        if (!loadShader(
                "WorldGizmo.frag", &mGlDefaultWorldGizmoFS, eShaderStage::FragmentShader, eMaterialAssetType::None))
        {
            cancelInitialization();
            return false;
        }

        // Cubemap
        const std::unique_ptr<const parser::TextureIR> p2x2Black = parser::parseTextureFile(
            Path(std::string("Subsystems/Rendering/BuiltInAssets/2x2Black.png")).ResolvedProjectPath(),
            "DefaultCubemap",
            true);
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mGlDefaultCubeMap);
        glTextureStorage2D(mGlDefaultCubeMap, 1, GL_RGBA8, p2x2Black->Img.GetWidth(), p2x2Black->Img.GetHeight());
        for (int32_t face = 0; face < 6; ++face)
        {
            glTextureSubImage3D(mGlDefaultCubeMap,
                                0,
                                0,
                                0,
                                face,
                                p2x2Black->Img.GetWidth(),
                                p2x2Black->Img.GetHeight(),
                                1,
                                GL_RGBA,
                                GL_UNSIGNED_BYTE,
                                p2x2Black->Img.GetBitmap());
        }
        glTextureParameteri(mGlDefaultCubeMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(mGlDefaultCubeMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(mGlDefaultCubeMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(mGlDefaultCubeMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(mGlDefaultCubeMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // UBOs
        glCreateBuffers(1, &mGlMatrixUBO);
        glNamedBufferData(mGlMatrixUBO, sizeof(MatrixLayout), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, mMatrixUboIndex, mGlMatrixUBO);
        if (!ASSERT_GL())
        {
            cancelInitialization();
            return false;
        }

        glCreateBuffers(1, &mGlLightUBO);
        glNamedBufferData(mGlLightUBO, sizeof(LightLayout), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, mLightUboIndex, mGlLightUBO);
        if (!ASSERT_GL())
        {
            cancelInitialization();
            return false;
        }

        glCreateBuffers(1, &mGlGeneralPurposeUBO);
        glNamedBufferData(mGlGeneralPurposeUBO, sizeof(GeneralPurposeLayout), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, mGeneralPurposeUboIndex, mGlGeneralPurposeUBO);
        if (!ASSERT_GL())
        {
            cancelInitialization();
            return false;
        }

        // Set initial GL state
        glEnable(GL_MULTISAMPLE);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        return true;
    }
}

void RenderingSystemGL::SetVSync(bool bEnabled)
{
    if (mbVSyncEnabled.TestNotEqual(bEnabled))
    {
#if defined(_WIN32)
        static_cast<Win32ApplicationGL&>(IPlatformApplication::GetInstance()).SetVSync(bEnabled);
#elif defined(__linux__)
#error "Linux environment is currently not supported."
#endif
        mbVSyncEnabled.Set(bEnabled);
    }
}

void RenderingSystemGL::releaseAllResources()
{
    flush();
    // Unload all GPU resources
    for (auto& nativeHandle : mStaticMeshNativeHandlePool.GetMutableRawPool())
    {
        if (nativeHandle.has_value())
        {
            glDeleteBuffers(1, &nativeHandle.value().GlVbo);
            glDeleteBuffers(1, &nativeHandle.value().GlEbo);
        }
    }
    for (auto& nativeHandle : mMaterialNativeHandlePool.GetMutableRawPool())
    {
        if (nativeHandle.has_value())
        {
            glDeleteBuffers(1, &nativeHandle.value().GlUbo);
        }
    }
    for (auto& nativeHandle : mTextureNativeHandlePool.GetMutableRawPool())
    {
        if (nativeHandle.has_value())
        {
            glDeleteTextures(1, &nativeHandle.value().GlTexture);
        }
    }
    for (auto& nativeHandle : mShaderNativeHandlePool.GetMutableRawPool())
    {
        if (nativeHandle.has_value())
        {
            glDeleteProgram(nativeHandle.value().GlProgram);
        }
    }

    for (auto& it : mNameToGlFboMap)
    {
        glDeleteFramebuffers(2, it.second.FBO);
        glDeleteFramebuffers(2, it.second.ResolvedFBO);
    }

    mNameToGlFboMap.clear();

    glDeleteVertexArrays(1, &mGlVAO);
    glDeleteProgramPipelines(1, &mGlProgramPipeline);
    glDeleteProgram(mGlDefaultVS);
    glDeleteProgram(mGlDefaultPhongFS);
    glDeleteProgram(mGlDefaultPhongMaskedFS);
    glDeleteProgram(mGlDefaultPbrFS);
    glDeleteProgram(mGlDefaultPbrMaskedFS);
    glDeleteProgram(mGlDefaultSkyMapVS);
    glDeleteProgram(mGlDefaultSkyMapFS);
    glDeleteProgram(mGlDefaultWorldGizmoVS);
    glDeleteProgram(mGlDefaultWorldGizmoFS);
    glDeleteTextures(1, &mGlDefaultCubeMap);
    glDeleteBuffers(1, &mGlMatrixUBO);
    glDeleteBuffers(1, &mGlLightUBO);
    glDeleteBuffers(1, &mGlGeneralPurposeUBO);

    mStaticMeshNativeHandlePool.Clear();
    mMaterialNativeHandlePool.Clear();
    mTextureNativeHandlePool.Clear();
    mShaderNativeHandlePool.Clear();
}

void RenderingSystemGL::applyMaterial(GpuMaterialHandle hGpuMaterial)
{
    const GpuMaterial* pMaterial = hGpuMaterial.Get();

    if (pMaterial->PipelineState.AlphaMode == eMaterialAlphaMode::Blend)
    {
        switch (pMaterial->PipelineState.AlphaBlendMode)
        {
            case eMaterialAlphaBlendMode::Default:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case eMaterialAlphaBlendMode::Additive:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
            default:
                break;
        }
    }

    if (pMaterial->PipelineState.bWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (pMaterial->PipelineState.bBackfaceCulling)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    glBindBufferBase(GL_UNIFORM_BUFFER,
                     mMaterialUboIndex,
                     mMaterialNativeHandlePool.Get(hGpuMaterial.Get()->NativeHandlePoolIndex)->GlUbo);

    for (int32_t i = 1; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
    {
        if (pMaterial->TextureNativeHandlePoolIndices[i] != 0)
        {
            glBindTextureUnit(i, mTextureNativeHandlePool.Get(pMaterial->TextureNativeHandlePoolIndices[i])->GlTexture);
        }
    }

    const GLuint glVS =
        pMaterial->ShaderNativeHandlePoolIndices[static_cast<int32_t>(eShaderStage::VertexShader)] != 0
            ? mShaderNativeHandlePool
                  .Get(pMaterial->ShaderNativeHandlePoolIndices[static_cast<int32_t>(eShaderStage::VertexShader)])
                  ->GlProgram
            : mGlDefaultVS;

    glUseProgramStages(mGlProgramPipeline, GL_VERTEX_SHADER_BIT, glVS);

    GLuint glFS = 0;
    if (pMaterial->ShaderNativeHandlePoolIndices[static_cast<int32_t>(eShaderStage::FragmentShader)] != 0)
    {
        glFS = mShaderNativeHandlePool
                   .Get(pMaterial->ShaderNativeHandlePoolIndices[static_cast<int32_t>(eShaderStage::FragmentShader)])
                   ->GlProgram;
    }
    else
    {
        switch (hGpuMaterial.Get()->Type)
        {
            case eMaterialAssetType::Legacy:
                if (hGpuMaterial.Get()->PipelineState.AlphaMode == eMaterialAlphaMode::Mask)
                {
                    glFS = mGlDefaultPhongMaskedFS;
                }
                else
                {
                    glFS = mGlDefaultPhongFS;
                }
                break;
            case eMaterialAssetType::StandardLit:
                if (hGpuMaterial.Get()->PipelineState.AlphaMode == eMaterialAlphaMode::Mask)
                {
                    glFS = mGlDefaultPbrMaskedFS;
                }
                else
                {
                    glFS = mGlDefaultPbrFS;
                }
                break;
            default:
                HO_ASSERT(false, "Invalid material format.");
        }
    }

    glUseProgramStages(mGlProgramPipeline, GL_FRAGMENT_SHADER_BIT, glFS);
}

void RenderingSystemGL::executeDrawCommand(const DrawCommand& command)
{
    glNamedBufferSubData(mGlMatrixUBO, 0, sizeof(Matrix4x4), &command.WorldMat);
    Matrix4x4 invWorldMat = command.WorldMat.Inverse();
    glNamedBufferSubData(mGlMatrixUBO, sizeof(Matrix4x4), sizeof(Matrix4x4), &invWorldMat);

    const StaticMeshNativeHandleGL* meshNativehandle =
        mStaticMeshNativeHandlePool.Get(command.hGpuStaticMesh.Get()->NativeHandlePoolIndex);
    glVertexArrayVertexBuffer(mGlVAO,
                              mAttribBindingIndex,
                              meshNativehandle->GlVbo,
                              static_cast<GLintptr>(command.VertexOffset * sizeof(Vertex)),
                              sizeof(Vertex));
    glVertexArrayElementBuffer(mGlVAO, meshNativehandle->GlEbo);

    glDrawElements(GL_TRIANGLES,
                   command.IndexCount,
                   GL_UNSIGNED_INT,
                   reinterpret_cast<void*>(command.IndexOffset * sizeof(uint32_t))); // NOLINT

    ASSERT_GL();
}

void RenderingSystemGL::executeSkyMapPass(const GpuTexture* pSkyMap)
{
    HO_ASSERT(pSkyMap, "Sky map is null.");

    glBindTextureUnit(static_cast<int32_t>(eMaterialTextureUsage::Last),
                      mTextureNativeHandlePool.Get(pSkyMap->NativeHandlePoolIndex)->GlTexture);

    glUseProgramStages(mGlProgramPipeline, GL_VERTEX_SHADER_BIT, mGlDefaultSkyMapVS);
    glUseProgramStages(mGlProgramPipeline, GL_FRAGMENT_SHADER_BIT, mGlDefaultSkyMapFS);

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    ASSERT_GL();
}

void RenderingSystemGL::executeInWorldDebugDrawPass()
{
    glUseProgramStages(mGlProgramPipeline, GL_VERTEX_SHADER_BIT, mGlDefaultWorldGizmoVS);
    glUseProgramStages(mGlProgramPipeline, GL_FRAGMENT_SHADER_BIT, mGlDefaultWorldGizmoFS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_BLEND);

    ASSERT_GL();
}

void RenderingSystemGL::executeOverlayDebugDrawPass() {}

void RenderingSystemGL::prepareExecuteRenderQueue()
{
    glBindVertexArray(mGlVAO);
    glBindProgramPipeline(mGlProgramPipeline);
}

void RenderingSystemGL::prepareExecuteRenderPass(const RenderPass& pass)
{
    HO_ASSERT(pass.pFrameBuffer, "Render pass must have target frame buffer.");
    auto it = mNameToGlFboMap.find(pass.pFrameBuffer->hName);
    HO_ASSERT(it != mNameToGlFboMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

    glBindFramebuffer(GL_FRAMEBUFFER, it->second.FBO[mBackRenderQueueIndex]);
    glViewport(pass.Viewport.X, pass.Viewport.Y, pass.Viewport.Width, pass.Viewport.Height);

    ASSERT_GL();

    for (int32_t i = 0; i < static_cast<int32_t>(eRenderTargetType::Last); ++i)
    {
        if (pass.bClearTarget[i])
        {
            if (i == static_cast<int32_t>(eRenderTargetType::Depth))
            {
                glClearBufferfv(GL_DEPTH, 0, &pass.pFrameBuffer->ClearDepth);
            }
            else
            {
                glClearBufferfv(GL_COLOR, i, pass.pFrameBuffer->ClearColors[i].Data);
            }
        }
    }

    glNamedBufferSubData(mGlGeneralPurposeUBO, 0, sizeof(float) * 3, &pass.WorldCameraPos);
    glNamedBufferSubData(mGlMatrixUBO, sizeof(Matrix4x4) * 2, sizeof(Matrix4x4), &pass.ViewMat);
    glNamedBufferSubData(mGlMatrixUBO, sizeof(Matrix4x4) * 3, sizeof(Matrix4x4), &pass.ProjMat);
    glNamedBufferSubData(mGlLightUBO, 0, sizeof(LightLayout), &pass.Lights);

    ASSERT_GL();

    if (pass.pSkyMap)
    {
        glBindTextureUnit(static_cast<int32_t>(eMaterialTextureUsage::Last),
                          mTextureNativeHandlePool.Get(pass.pSkyMap->NativeHandlePoolIndex)->GlTexture);
    }
    else
    {
        glBindTextureUnit(static_cast<int32_t>(eMaterialTextureUsage::Last), mGlDefaultCubeMap);
    }

    if (pass.pIrradianceMap)
    {
        glBindTextureUnit(static_cast<int32_t>(eMaterialTextureUsage::Last) + 1,
                          mTextureNativeHandlePool.Get(pass.pIrradianceMap->NativeHandlePoolIndex)->GlTexture);
    }
    else
    {
        glBindTextureUnit(static_cast<int32_t>(eMaterialTextureUsage::Last) + 1, mGlDefaultCubeMap);
    }
    ASSERT_GL();
}

void RenderingSystemGL::prepareExecuteOpaqueDrawCommands()
{
    glDepthMask(GL_TRUE);

    glDisable(GL_BLEND);

    ASSERT_GL();
}

void RenderingSystemGL::prepareExecuteMaskedDrawCommands()
{
    glDepthMask(GL_TRUE);

    glDisable(GL_BLEND);

    ASSERT_GL();
}

void RenderingSystemGL::prepareExecuteBlendedDrawCommands()
{
    glDepthMask(GL_FALSE);

    glEnable(GL_BLEND);

    ASSERT_GL();
}

void RenderingSystemGL::finishExecuteRenderPass(const RenderPass& pass)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ASSERT_GL();
}

void RenderingSystemGL::finishExecuteRenderQueue() {}

void RenderingSystemGL::resolveFrameBuffer(const FrameBuffer& frameBuffer)
{
    if (frameBuffer.SampleCount == 1)
    {
        return;
    }

    auto glFBO = mNameToGlFboMap.find(frameBuffer.hName);
    HO_ASSERT(glFBO != mNameToGlFboMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");

    for (int32_t i = 0; i < static_cast<int32_t>(eRenderTargetType::Last); ++i)
    {
        if (i != static_cast<int32_t>(eRenderTargetType::Depth) &&
            frameBuffer.hRenderTargets[mBackRenderQueueIndex][i].IsValid())
        {
            HO_ASSERT(frameBuffer.hResolvedRenderTargets[mBackRenderQueueIndex][i].IsValid(),
                      "Render target for resolving was not created. Frame buffer might be created incompletely.");
            glNamedFramebufferReadBuffer(glFBO->second.FBO[mBackRenderQueueIndex], GL_COLOR_ATTACHMENT0 + i);
            glNamedFramebufferDrawBuffer(glFBO->second.ResolvedFBO[mBackRenderQueueIndex], GL_COLOR_ATTACHMENT0 + i);
            glBlitNamedFramebuffer(glFBO->second.FBO[mBackRenderQueueIndex],
                                   glFBO->second.ResolvedFBO[mBackRenderQueueIndex],
                                   0,
                                   0,
                                   frameBuffer.Width,
                                   frameBuffer.Height,
                                   0,
                                   0,
                                   frameBuffer.Width,
                                   frameBuffer.Height,
                                   GL_COLOR_BUFFER_BIT,
                                   GL_NEAREST);
        }
    }
}

void RenderingSystemGL::renderUI()
{
    const RenderQueue& queue = mRenderQueues[mBackRenderQueueIndex];

    for (int i = 0; i < queue.ActiveViewportCount; ++i)
    {
        const UIViewportData& viewportData = queue.UIViewports[i];

        const IPlatformWindow* pWindow = static_cast<const IPlatformWindow*>(viewportData.pTargetWindow);

        pWindow->ActivateContext();

        ImGui_ImplOpenGL3_RenderDrawData(viewportData.pDrawData.get());

        glFlush();

        pWindow->Present();
    }

    IPlatformApplication::GetInstance().GetMainWindow()->ActivateContext();
}

void RenderingSystemGL::flush()
{
    glFlush();
}

bool RenderingSystemGL::createFrameBuffer(const FrameBufferDesc& frameBufferDesc)
{
    if (!validateFrameBufferDesc(frameBufferDesc))
    {
        HO_ASSERT(false, "Invalid frame buffer description.");
        return false;
    }

    GLuint glFBO[2] = {};
    glCreateFramebuffers(2, glFBO);
    if (!ASSERT_GL())
    {
        return false;
    }
    GLuint glResolvedFBO[2] = {};
    if (frameBufferDesc.SampleCount > 1)
    {
        glCreateFramebuffers(2, glResolvedFBO);
        if (!ASSERT_GL())
        {
            glDeleteFramebuffers(2, glFBO);
            return false;
        }
    }

    FrameBuffer newFrameBuffer;
    newFrameBuffer.hName = frameBufferDesc.hName;
    newFrameBuffer.Width = frameBufferDesc.Width;
    newFrameBuffer.Height = frameBufferDesc.Height;
    newFrameBuffer.SampleCount = frameBufferDesc.SampleCount;

    GLint maxSamples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    HO_ASSERT(frameBufferDesc.SampleCount <= maxSamples,
              "Required sample count exceeds current hardware's maximum sample count limits. The hardware maximum "
              "sample count will be used instead.");

    GLuint glRenderTargets[2][static_cast<int32_t>(eRenderTargetType::Last)] = {};
    GLuint glResolvedRenderTargets[2][static_cast<int32_t>(eRenderTargetType::Last)] = {};
    for (int32_t targetIdx = 0; targetIdx < static_cast<int32_t>(eRenderTargetType::Last); ++targetIdx)
    {
        for (int32_t bufferIdx = 0; bufferIdx < 2; ++bufferIdx)
        {
            if (frameBufferDesc.TargetDesc[targetIdx].Format != eRenderTargetFormat::None)
            {
                if (frameBufferDesc.SampleCount > 1)
                {
                    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &glRenderTargets[bufferIdx][targetIdx]);
                    if (targetIdx != static_cast<int32_t>(eRenderTargetType::Depth))
                    {
                        glCreateTextures(GL_TEXTURE_2D, 1, &glResolvedRenderTargets[bufferIdx][targetIdx]);
                    }
                }
                else
                {
                    glCreateTextures(GL_TEXTURE_2D, 1, &glRenderTargets[bufferIdx][targetIdx]);
                }
                if (!ASSERT_GL())
                {
                    cancelFrameBufferCreation(glFBO, glResolvedFBO, glRenderTargets, glResolvedRenderTargets);
                    return false;
                }
            }
        }
    }

    for (int32_t targetIdx = 0; targetIdx < static_cast<int32_t>(eRenderTargetType::Last); ++targetIdx)
    {
        for (int32_t bufferIdx = 0; bufferIdx < 2; ++bufferIdx)
        {
            if (frameBufferDesc.TargetDesc[targetIdx].Format != eRenderTargetFormat::None)
            {
                if (frameBufferDesc.SampleCount > 1)
                {
                    glTextureStorage2DMultisample(glRenderTargets[bufferIdx][targetIdx],
                                                  math::Min(frameBufferDesc.SampleCount, maxSamples),
                                                  toGlRenderTargetFormat(frameBufferDesc.TargetDesc[targetIdx].Format),
                                                  frameBufferDesc.Width,
                                                  frameBufferDesc.Height,
                                                  GL_TRUE);
                    if (targetIdx != static_cast<int32_t>(eRenderTargetType::Depth))
                    {
                        glTextureStorage2D(glResolvedRenderTargets[bufferIdx][targetIdx],
                                           1,
                                           toGlRenderTargetFormat(frameBufferDesc.TargetDesc[targetIdx].Format),
                                           frameBufferDesc.Width,
                                           frameBufferDesc.Height);
                    }
                }
                else
                {
                    glTextureStorage2D(glRenderTargets[bufferIdx][targetIdx],
                                       1,
                                       toGlRenderTargetFormat(frameBufferDesc.TargetDesc[targetIdx].Format),
                                       frameBufferDesc.Width,
                                       frameBufferDesc.Height);
                }
                if (!ASSERT_GL())
                {
                    cancelFrameBufferCreation(glFBO, glResolvedFBO, glRenderTargets, glResolvedRenderTargets);
                    return false;
                }

                if (targetIdx == static_cast<int32_t>(eRenderTargetType::Depth))
                {
                    switch (frameBufferDesc.TargetDesc[targetIdx].Format)
                    {
                        case IRenderingSystem::eRenderTargetFormat::D24_UNORM_S8_UINT:
                            glNamedFramebufferTexture(glFBO[bufferIdx],
                                                      GL_DEPTH_STENCIL_ATTACHMENT,
                                                      glRenderTargets[bufferIdx][targetIdx],
                                                      0);
                            break;
                        case IRenderingSystem::eRenderTargetFormat::D32_FLOAT:
                            glNamedFramebufferTexture(
                                glFBO[bufferIdx], GL_DEPTH_ATTACHMENT, glRenderTargets[bufferIdx][targetIdx], 0);
                            break;
                        default:
                            HO_ASSERT(false, "Invalid depth target format.");
                            cancelFrameBufferCreation(glFBO, glResolvedFBO, glRenderTargets, glResolvedRenderTargets);
                            return false;
                    }
                }
                else
                {
                    glNamedFramebufferTexture(
                        glFBO[bufferIdx], GL_COLOR_ATTACHMENT0 + targetIdx, glRenderTargets[bufferIdx][targetIdx], 0);
                    if (frameBufferDesc.SampleCount > 1)
                    {
                        glNamedFramebufferTexture(glResolvedFBO[bufferIdx],
                                                  GL_COLOR_ATTACHMENT0 + targetIdx,
                                                  glResolvedRenderTargets[bufferIdx][targetIdx],
                                                  0);
                    }
                }

                if (!ASSERT_GL())
                {
                    cancelFrameBufferCreation(glFBO, glResolvedFBO, glRenderTargets, glResolvedRenderTargets);
                    return false;
                }

                GpuTextureHandle hGpuRenderTarget = prepareUploadTexture(TextureHandle::sNULL, true);

                TextureNativeHandleGL nativeHandle;
                nativeHandle.GlTexture = glRenderTargets[bufferIdx][targetIdx];
                hGpuRenderTarget.Get()->NativeHandlePoolIndex = mTextureNativeHandlePool.Add(nativeHandle);

                newFrameBuffer.hRenderTargets[bufferIdx][targetIdx] = hGpuRenderTarget;

                if (frameBufferDesc.SampleCount > 1 && targetIdx != static_cast<int32_t>(eRenderTargetType::Depth))
                {
                    GpuTextureHandle hGpuResolvedRenderTarget = prepareUploadTexture(TextureHandle::sNULL, true);

                    TextureNativeHandleGL nativeHandleResolved;
                    nativeHandleResolved.GlTexture = glResolvedRenderTargets[bufferIdx][targetIdx];
                    hGpuResolvedRenderTarget.Get()->NativeHandlePoolIndex =
                        mTextureNativeHandlePool.Add(nativeHandleResolved);

                    newFrameBuffer.hResolvedRenderTargets[bufferIdx][targetIdx] = hGpuResolvedRenderTarget;
                }
            }
        }
    }

    for (int32_t targetIdx = 0; targetIdx < static_cast<int32_t>(eRenderTargetType::Last); ++targetIdx)
    {
        if (targetIdx == static_cast<int32_t>(eRenderTargetType::Depth))
        {
            newFrameBuffer.ClearDepth = frameBufferDesc.TargetDesc[targetIdx].ClearDepth;
            newFrameBuffer.ClearStencil = frameBufferDesc.TargetDesc[targetIdx].ClearStencil;
        }
        else
        {
            newFrameBuffer.ClearColors[targetIdx] = Vector4(frameBufferDesc.TargetDesc[targetIdx].ClearColor);
        }
    }

    for (int32_t bufferIdx = 0; bufferIdx < 2; ++bufferIdx)
    {
        if (glCheckNamedFramebufferStatus(glFBO[bufferIdx], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            HO_ASSERT(false, "Framebuffer is incomplete.");
            cancelFrameBufferCreation(glFBO, glResolvedFBO, glRenderTargets, glResolvedRenderTargets);
            for (int32_t targetIdx = 0; targetIdx < static_cast<int32_t>(eRenderTargetType::Last); ++targetIdx)
            {
                if (newFrameBuffer.hRenderTargets[bufferIdx][targetIdx].IsValid())
                {
                    mTextureNativeHandlePool.Remove(
                        newFrameBuffer.hRenderTargets[bufferIdx][targetIdx].Get()->NativeHandlePoolIndex);
                    mGpuTexturePool.Remove(newFrameBuffer.hRenderTargets[bufferIdx][targetIdx].GetIndex());
                }
            }
            return false;
        }
        if (frameBufferDesc.SampleCount > 1)
        {
            if (glCheckNamedFramebufferStatus(glResolvedFBO[bufferIdx], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                HO_ASSERT(false, "ResolvedFramebuffer is incomplete.");
                cancelFrameBufferCreation(glFBO, glResolvedFBO, glRenderTargets, glResolvedRenderTargets);
                for (int32_t targetIdx = 0; targetIdx < static_cast<int32_t>(eRenderTargetType::Last); ++targetIdx)
                {
                    if (newFrameBuffer.hResolvedRenderTargets[bufferIdx][targetIdx].IsValid())
                    {
                        mTextureNativeHandlePool.Remove(
                            newFrameBuffer.hResolvedRenderTargets[bufferIdx][targetIdx].Get()->NativeHandlePoolIndex);
                        mGpuTexturePool.Remove(newFrameBuffer.hResolvedRenderTargets[bufferIdx][targetIdx].GetIndex());
                    }
                }
                return false;
            }
        }
    }

    mNameToGlFboMap.insert({frameBufferDesc.hName, GlFBO{{glFBO[0], glFBO[1]}, {glResolvedFBO[0], glResolvedFBO[1]}}});
    mNameToFrameBufferMap.insert({frameBufferDesc.hName, newFrameBuffer});
    return true;
}

bool RenderingSystemGL::destroyFrameBuffer(StringHandle hFrameBufferName)
{
    auto fbit = mNameToFrameBufferMap.find(hFrameBufferName);
    if (fbit == mNameToFrameBufferMap.end())
    {
        return false;
    }
    FrameBuffer& frameBuffer = fbit->second;
    for (int32_t targetIdx = 0; targetIdx < static_cast<int32_t>(eRenderTargetType::Last); ++targetIdx)
    {
        for (int32_t bufferIdx = 0; bufferIdx < 2; ++bufferIdx)
        {
            if (frameBuffer.hRenderTargets[bufferIdx][targetIdx].IsValid())
            {
                const TextureNativeHandleGL* nativeHandle = mTextureNativeHandlePool.Get(
                    frameBuffer.hRenderTargets[bufferIdx][targetIdx].Get()->NativeHandlePoolIndex);
                HO_ASSERT(nativeHandle,
                          "Render target's native handle is null. Frame buffer might be created incompletely.");
                glDeleteTextures(1, &nativeHandle->GlTexture);
                mTextureNativeHandlePool.Remove(
                    frameBuffer.hRenderTargets[bufferIdx][targetIdx].Get()->NativeHandlePoolIndex);
                mGpuTexturePool.Remove(frameBuffer.hRenderTargets[bufferIdx][targetIdx].GetIndex());
            }
            if (frameBuffer.hResolvedRenderTargets[bufferIdx][targetIdx].IsValid())
            {
                const TextureNativeHandleGL* nativeHandle = mTextureNativeHandlePool.Get(
                    frameBuffer.hResolvedRenderTargets[bufferIdx][targetIdx].Get()->NativeHandlePoolIndex);
                HO_ASSERT(nativeHandle,
                          "Render target's native handle is null. Frame buffer might be created incompletely.");
                glDeleteTextures(1, &nativeHandle->GlTexture);
                mTextureNativeHandlePool.Remove(
                    frameBuffer.hResolvedRenderTargets[bufferIdx][targetIdx].Get()->NativeHandlePoolIndex);
                mGpuTexturePool.Remove(frameBuffer.hResolvedRenderTargets[bufferIdx][targetIdx].GetIndex());
            }
        }
    }

    mNameToFrameBufferMap.erase(hFrameBufferName);

    auto glit = mNameToGlFboMap.find(hFrameBufferName);
    HO_ASSERT(glit != mNameToGlFboMap.end(),
              "GL id for frame buffer was not inserted. Frame buffer might be created incompletely.");
    glDeleteFramebuffers(2, glit->second.FBO);
    glDeleteFramebuffers(2, glit->second.ResolvedFBO);
    mNameToGlFboMap.erase(hFrameBufferName);
    return true;
}

// TODO: Currently, all vertex attributes are always packed and uploaded interleaved
//       regardless of their actual presence in the asset.
//       Need to optimize vertex buffer generation to pack and upload only the active attributes.
IRenderingSystem::eResourceTransferResult RenderingSystemGL::uploadStaticMesh(
    GpuStaticMeshHandle hPreparedGpuStaticMesh, uint64_t currentFrame)
{
    HO_ASSERT(hPreparedGpuStaticMesh.IsValid(),
              "Invalid gpu static mesh handle. It might be not created by 'prepareUploadStatucMesh' yet.");
    if (hPreparedGpuStaticMesh.Get()->hOrigin.IsNULL())
    {
        HO_ASSERT(false, "Invalid static mesh asset. It might be removed from asset system while pending.");
        return eResourceTransferResult::Failed;
    }
    const StaticMeshAsset* pOrigin = hPreparedGpuStaticMesh.Get()->hOrigin.Get();

    const int32_t vertexCount = pOrigin->Positions.GetSize();

    FixedArray<Vertex> vertexBuffer(vertexCount);

    for (int32_t i = 0; i < vertexCount; ++i)
    {
        vertexBuffer[i].Position = pOrigin->Positions[i];
        if (i < pOrigin->Normals.GetSize())
        {
            vertexBuffer[i].Normal = pOrigin->Normals[i];
        }
        if (i < pOrigin->Tangents.GetSize())
        {
            vertexBuffer[i].Tangent = pOrigin->Tangents[i];
        }
        if (i < pOrigin->UVs.GetSize())
        {
            vertexBuffer[i].UV0 = pOrigin->UVs[i][0];
            vertexBuffer[i].UV1 = pOrigin->UVs[i][1];
        }
        if (i < pOrigin->Colors.GetSize())
        {
            vertexBuffer[i].Color0[0] = pOrigin->Colors[i][0].R;
            vertexBuffer[i].Color0[1] = pOrigin->Colors[i][0].G;
            vertexBuffer[i].Color0[2] = pOrigin->Colors[i][0].B;
            vertexBuffer[i].Color0[3] = pOrigin->Colors[i][0].A;
        }
    }

    GLuint glVBO = 0;
    glCreateBuffers(1, &glVBO);
    glNamedBufferData(
        glVBO, static_cast<GLsizeiptr>(sizeof(Vertex) * vertexCount), vertexBuffer.Data(), GL_STATIC_DRAW);
    if (!ASSERT_GL())
    {
        glDeleteBuffers(1, &glVBO);
        return eResourceTransferResult::Failed;
    }
    GLuint glEBO = 0;
    glCreateBuffers(1, &glEBO);
    glNamedBufferData(glEBO,
                      static_cast<GLsizeiptr>(sizeof(int32_t) * pOrigin->Indices.GetSize()),
                      pOrigin->Indices.Data(),
                      GL_STATIC_DRAW);
    if (!ASSERT_GL())
    {
        glDeleteBuffers(1, &glVBO);
        glDeleteBuffers(1, &glEBO);
        return eResourceTransferResult::Failed;
    }
    StaticMeshNativeHandleGL nativeHandle;
    nativeHandle.GlVbo = glVBO;
    nativeHandle.GlEbo = glEBO;
    hPreparedGpuStaticMesh.Get()->NativeHandlePoolIndex = mStaticMeshNativeHandlePool.Add(nativeHandle);
    hPreparedGpuStaticMesh.Get()->LastUsedFrame = currentFrame;
    hPreparedGpuStaticMesh.Get()->bPendingUpload = false;
    return eResourceTransferResult::Success;
}

IRenderingSystem::eResourceTransferResult RenderingSystemGL::uploadMaterial(GpuMaterialHandle hPreparedGpuMaterial,
                                                                            uint64_t currentFrame)
{
    HO_ASSERT(hPreparedGpuMaterial.IsValid(),
              "Invalid gpu material handle. It might be not created by 'prepareUploadMaterial' yet.");

    if (hPreparedGpuMaterial.Get()->hOrigin.IsNULL())
    {
        HO_ASSERT(false, "Invalid material asset. It might be removed from asset system while pending.");
        return eResourceTransferResult::Failed;
    }

    const MaterialAsset* pOrigin = hPreparedGpuMaterial.Get()->hOrigin.Get();

    for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
    {
        if (hPreparedGpuMaterial.Get()->hGpuTextures[i].IsNULL())
        {
            continue;
        }
        if (hPreparedGpuMaterial.Get()->hGpuTextures[i].Get()->bPendingUpload)
        {
            return eResourceTransferResult::NeedsRetry;
        }
        else
        {
            hPreparedGpuMaterial.Get()->TextureNativeHandlePoolIndices[i] =
                hPreparedGpuMaterial.Get()->hGpuTextures[i].Get()->NativeHandlePoolIndex;
        }
    }
    for (int32_t i = 0; i < static_cast<int32_t>(eShaderStage::Last); ++i)
    {
        if (hPreparedGpuMaterial.Get()->hGpuShaders[i].IsNULL())
        {
            continue;
        }
        if (hPreparedGpuMaterial.Get()->hGpuShaders[i].Get()->bPendingUpload)
        {
            return eResourceTransferResult::NeedsRetry;
        }
        else
        {
            hPreparedGpuMaterial.Get()->ShaderNativeHandlePoolIndices[i] =
                hPreparedGpuMaterial.Get()->hGpuShaders[i].Get()->NativeHandlePoolIndex;
        }
    }

    GpuMaterial::AttributeLayout attributes;
    attributes.Ambient = Vector4(pOrigin->Ambient);
    attributes.Diffuse = Vector4(pOrigin->Diffuse);
    attributes.Specular = Vector4(pOrigin->Specular);
    attributes.Albedo = Vector4(pOrigin->Albedo);
    attributes.Emissive = Vector4(pOrigin->Emissive);
    attributes.Shininess = pOrigin->Shininess;
    attributes.Metallic = pOrigin->Metallic;
    attributes.Roughness = pOrigin->Roughness;
    attributes.IndexOfRefraction = pOrigin->IndexOfRefraction;
    attributes.Opacity = pOrigin->Opacity;
    attributes.AlphaThreshold = pOrigin->AlphaThreshold;
    attributes.EmissiveIntensity = pOrigin->EmissiveIntensity;
    attributes.NormalScale = pOrigin->NormalScale;
    attributes.OcclusionStrength = pOrigin->OcclusionStrength;

    for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
    {
        attributes.UVChannels[i].Value = pOrigin->UVChannels[i];
        const Matrix4x4 uvMatrix(Vector4(pOrigin->UVTransforms[i].GetCol0()),
                                 Vector4(pOrigin->UVTransforms[i].GetCol1()),
                                 Vector4(pOrigin->UVTransforms[i].GetCol2()),
                                 Vector4::sUnitW);
        attributes.UVMatrices[i] = uvMatrix;
        attributes.bHasTexture[i].Value = static_cast<int32_t>(pOrigin->hTextures[i].IsValid());
    }

    GLuint glUBO;
    glCreateBuffers(1, &glUBO);
    glNamedBufferData(glUBO, sizeof(GpuMaterial::AttributeLayout), &attributes, GL_STATIC_DRAW);
    if (!ASSERT_GL())
    {
        glDeleteBuffers(1, &glUBO);
        return eResourceTransferResult::Failed;
    }
    MaterialNativeHandleGL nativeHandle;
    nativeHandle.GlUbo = glUBO;
    hPreparedGpuMaterial.Get()->NativeHandlePoolIndex = mMaterialNativeHandlePool.Add(nativeHandle);
    hPreparedGpuMaterial.Get()->LastUsedFrame = currentFrame;
    hPreparedGpuMaterial.Get()->bPendingUpload = false;
    return eResourceTransferResult::Success;
}

IRenderingSystem::eResourceTransferResult RenderingSystemGL::uploadTexture(GpuTextureHandle hPreparedGpuTexture,
                                                                           uint64_t currentFrame)
{
    HO_ASSERT(hPreparedGpuTexture.IsValid(),
              "Invalid gpu texture handle. It might be not created by 'prepareUploadTexture' yet.");

    if (hPreparedGpuTexture.Get()->hOrigin.IsNULL())
    {
        HO_ASSERT(false, "Invalid texture asset. It might be removed from asset system while pending.");
        return eResourceTransferResult::Failed;
    }

    const TextureAsset* pOrigin = hPreparedGpuTexture.Get()->hOrigin.Get();

    GLuint glTexture;
    glCreateTextures(toGlTextureType(pOrigin->Type), 1, &glTexture);

    if (!ASSERT_GL())
    {
        return eResourceTransferResult::Failed;
    }

    switch (pOrigin->Type)
    {
        case eTextureType::Texture1D:
            glTextureStorage1D(glTexture, pOrigin->MipLevels, toGlInternalFormat(pOrigin->Format), pOrigin->Width);
            break;
        case eTextureType::Texture2D:
        case eTextureType::TextureCubeMap:
            glTextureStorage2D(
                glTexture, pOrigin->MipLevels, toGlInternalFormat(pOrigin->Format), pOrigin->Width, pOrigin->Height);
            break;
        case eTextureType::Texture3D:
            glTextureStorage3D(glTexture,
                               pOrigin->MipLevels,
                               toGlInternalFormat(pOrigin->Format),
                               pOrigin->Width,
                               pOrigin->Height,
                               pOrigin->Depth);
            break;
        case eTextureType::Texture1DArray:
            glTextureStorage2D(
                glTexture, pOrigin->MipLevels, toGlInternalFormat(pOrigin->Format), pOrigin->Width, pOrigin->ArraySize);
            break;
        case eTextureType::Texture2DArray:
            glTextureStorage3D(glTexture,
                               pOrigin->MipLevels,
                               toGlInternalFormat(pOrigin->Format),
                               pOrigin->Width,
                               pOrigin->Height,
                               pOrigin->ArraySize);
            break;
        case eTextureType::TextureCubeMapArray:
            glTextureStorage3D(glTexture,
                               pOrigin->MipLevels,
                               toGlInternalFormat(pOrigin->Format),
                               pOrigin->Width,
                               pOrigin->Height,
                               pOrigin->ArraySize * 6);
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

    for (int32_t i = 0; i < pOrigin->MipLevels; ++i)
    {
        const int32_t mipWidth = math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(pOrigin->Width) >> i));
        const int32_t mipHeight = math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(pOrigin->Height) >> i));
        const int32_t mipDepth = math::Max(1, static_cast<int32_t>(pOrigin->Depth >> i));

        if (isUncompressedFormat(pOrigin->Format))
        {
            switch (pOrigin->Type)
            {
                case eTextureType::Texture1D:
                    glTextureSubImage1D(glTexture,
                                        i,
                                        0,
                                        mipWidth,
                                        toGlExternalFormat(pOrigin->Format),
                                        toGlExternalType(pOrigin->Format),
                                        pOrigin->DataBlob.Data() + pOrigin->Layouts[i].Offset);
                    break;
                case eTextureType::Texture2D:
                    glTextureSubImage2D(glTexture,
                                        i,
                                        0,
                                        0,
                                        mipWidth,
                                        mipHeight,
                                        toGlExternalFormat(pOrigin->Format),
                                        toGlExternalType(pOrigin->Format),
                                        pOrigin->DataBlob.Data() + pOrigin->Layouts[i].Offset);
                    break;
                case eTextureType::TextureCubeMap:
                    glTextureSubImage3D(glTexture,
                                        i,
                                        0,
                                        0,
                                        0,
                                        mipWidth,
                                        mipHeight,
                                        6,
                                        toGlExternalFormat(pOrigin->Format),
                                        toGlExternalType(pOrigin->Format),
                                        pOrigin->DataBlob.Data() + pOrigin->Layouts[i * 6].Offset);
                    break;
                case eTextureType::Texture3D:
                    glTextureSubImage3D(glTexture,
                                        i,
                                        0,
                                        0,
                                        0,
                                        mipWidth,
                                        mipHeight,
                                        mipDepth,
                                        toGlExternalFormat(pOrigin->Format),
                                        toGlExternalType(pOrigin->Format),
                                        pOrigin->DataBlob.Data() + pOrigin->Layouts[i].Offset);
                    break;
                case eTextureType::Texture1DArray:
                    for (int32_t arrIdx = 0; arrIdx < pOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * pOrigin->MipLevels) + i;
                        glTextureSubImage2D(glTexture,
                                            i,
                                            0,
                                            arrIdx,
                                            mipWidth,
                                            1,
                                            toGlExternalFormat(pOrigin->Format),
                                            toGlExternalType(pOrigin->Format),
                                            pOrigin->DataBlob.Data() + pOrigin->Layouts[linearIdx].Offset);
                    }
                    break;
                case eTextureType::Texture2DArray:
                    for (int32_t arrIdx = 0; arrIdx < pOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * pOrigin->MipLevels) + i;
                        glTextureSubImage3D(glTexture,
                                            i,
                                            0,
                                            0,
                                            arrIdx,
                                            mipWidth,
                                            mipHeight,
                                            1,
                                            toGlExternalFormat(pOrigin->Format),
                                            toGlExternalType(pOrigin->Format),
                                            pOrigin->DataBlob.Data() + pOrigin->Layouts[linearIdx].Offset);
                    }
                    break;
                case eTextureType::TextureCubeMapArray:
                    for (int32_t arrIdx = 0; arrIdx < pOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * pOrigin->MipLevels * 6) + (i * 6);

                        glTextureSubImage3D(glTexture,
                                            i,
                                            0,
                                            0,
                                            arrIdx * 6,
                                            mipWidth,
                                            mipHeight,
                                            6,
                                            toGlExternalFormat(pOrigin->Format),
                                            toGlExternalType(pOrigin->Format),
                                            pOrigin->DataBlob.Data() + pOrigin->Layouts[linearIdx].Offset);
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
            switch (pOrigin->Type)
            {
                case eTextureType::Texture1D:
                    glCompressedTextureSubImage1D(glTexture,
                                                  i,
                                                  0,
                                                  mipWidth,
                                                  toGlExternalFormat(pOrigin->Format),
                                                  pOrigin->Layouts[i].Size,
                                                  pOrigin->DataBlob.Data() + pOrigin->Layouts[i].Offset);
                    break;
                case eTextureType::Texture2D:
                    glCompressedTextureSubImage2D(glTexture,
                                                  i,
                                                  0,
                                                  0,
                                                  mipWidth,
                                                  mipHeight,
                                                  toGlExternalFormat(pOrigin->Format),
                                                  pOrigin->Layouts[i].Size,
                                                  pOrigin->DataBlob.Data() + pOrigin->Layouts[i].Offset);
                    break;
                case eTextureType::TextureCubeMap:
                    glCompressedTextureSubImage3D(glTexture,
                                                  i,
                                                  0,
                                                  0,
                                                  0,
                                                  mipWidth,
                                                  mipHeight,
                                                  6,
                                                  toGlExternalFormat(pOrigin->Format),
                                                  pOrigin->Layouts[i * 6].Size * 6,
                                                  pOrigin->DataBlob.Data() + pOrigin->Layouts[i * 6].Offset);
                    break;
                case eTextureType::Texture3D:
                    glCompressedTextureSubImage3D(glTexture,
                                                  i,
                                                  0,
                                                  0,
                                                  0,
                                                  mipWidth,
                                                  mipHeight,
                                                  mipDepth,
                                                  toGlExternalFormat(pOrigin->Format),
                                                  pOrigin->Layouts[i * 6].Size,
                                                  pOrigin->DataBlob.Data() + pOrigin->Layouts[i].Offset);
                    break;
                case eTextureType::Texture1DArray:
                    for (int32_t arrIdx = 0; arrIdx < pOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * pOrigin->MipLevels) + i;
                        glCompressedTextureSubImage2D(glTexture,
                                                      i,
                                                      0,
                                                      arrIdx,
                                                      mipWidth,
                                                      1,
                                                      toGlExternalFormat(pOrigin->Format),
                                                      pOrigin->Layouts[linearIdx].Size,
                                                      pOrigin->DataBlob.Data() + pOrigin->Layouts[linearIdx].Offset);
                    }
                    break;
                case eTextureType::Texture2DArray:
                    for (int32_t arrIdx = 0; arrIdx < pOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * pOrigin->MipLevels) + i;
                        glCompressedTextureSubImage3D(glTexture,
                                                      i,
                                                      0,
                                                      0,
                                                      arrIdx,
                                                      mipWidth,
                                                      mipHeight,
                                                      1,
                                                      toGlExternalFormat(pOrigin->Format),
                                                      pOrigin->Layouts[linearIdx].Size,
                                                      pOrigin->DataBlob.Data() + pOrigin->Layouts[linearIdx].Offset);
                    }
                    break;
                case eTextureType::TextureCubeMapArray:
                    for (int32_t arrIdx = 0; arrIdx < pOrigin->ArraySize; ++arrIdx)
                    {
                        const int32_t linearIdx = (arrIdx * pOrigin->MipLevels * 6) + (i * 6);

                        glCompressedTextureSubImage3D(glTexture,
                                                      i,
                                                      0,
                                                      0,
                                                      arrIdx * 6,
                                                      mipWidth,
                                                      mipHeight,
                                                      6,
                                                      toGlExternalFormat(pOrigin->Format),
                                                      pOrigin->Layouts[linearIdx].Size * 6,
                                                      pOrigin->DataBlob.Data() + pOrigin->Layouts[linearIdx].Offset);
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
    glTextureParameteri(glTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(glTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(glTexture, GL_TEXTURE_MAX_LEVEL, pOrigin->MipLevels - 1);

    if (pOrigin->Type == eTextureType::TextureCubeMap || pOrigin->Type == eTextureType::TextureCubeMapArray)
    {
        glTextureParameteri(glTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(glTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(glTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    if (!ASSERT_GL())
    {
        glDeleteTextures(1, &glTexture);
        return eResourceTransferResult::Failed;
    }

    TextureNativeHandleGL nativeHandle;
    nativeHandle.GlTexture = glTexture;
    hPreparedGpuTexture.Get()->NativeHandlePoolIndex = mTextureNativeHandlePool.Add(nativeHandle);
    hPreparedGpuTexture.Get()->LastUsedFrame = currentFrame;
    hPreparedGpuTexture.Get()->bPendingUpload = false;
    return eResourceTransferResult::Success;
}

IRenderingSystem::eResourceTransferResult RenderingSystemGL::uploadShader(GpuShaderHandle hPreparedGpuShader,
                                                                          uint64_t currentFrame)
{
    HO_ASSERT(hPreparedGpuShader.IsValid(),
              "Invalid gpu shader handle. It might be not created by 'prepareUploadShader' yet.");

    if (hPreparedGpuShader.Get()->hOrigin.IsNULL())
    {
        HO_ASSERT(false, "Invalid shader asset. It might be removed from asset system while pending.");
        return eResourceTransferResult::Failed;
    }

    const ShaderAsset* pOrigin = hPreparedGpuShader.Get()->hOrigin.Get();

    const GLuint glShader = glCreateShader(toGlShaderType(pOrigin->Stage));
    if (!ASSERT_GL())
    {
        glDeleteShader(glShader);
        return eResourceTransferResult::Failed;
    }
    glShaderBinary(1,
                   &glShader,
                   GL_SHADER_BINARY_FORMAT_SPIR_V,
                   pOrigin->Binary.Data(),
                   static_cast<GLsizei>(pOrigin->Binary.GetSize() * sizeof(uint32_t)));
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
        HO_LOG_ERROR((std::string("Shader Link Failed:\n") + glLogInfo).c_str());
        HO_ASSERT(false, "Failed to link shader.");
        glDeleteShader(glShader);
        glDeleteProgram(glProgram);
        return eResourceTransferResult::Failed;
    }

    glDetachShader(glProgram, glShader);
    glDeleteShader(glShader);

    ShaderNativeHandleGL nativeHandle;
    nativeHandle.GlProgram = glProgram;
    hPreparedGpuShader.Get()->NativeHandlePoolIndex = mShaderNativeHandlePool.Add(nativeHandle);
    hPreparedGpuShader.Get()->LastUsedFrame = currentFrame;
    hPreparedGpuShader.Get()->bPendingUpload = false;
    return eResourceTransferResult::Success;
}

IRenderingSystem::eResourceTransferResult RenderingSystemGL::unloadStaticMesh(GpuStaticMeshHandle hGpuStaticMesh)
{
    if (hGpuStaticMesh.IsNULL())
    {
        return eResourceTransferResult::Failed;
    }
    const StaticMeshNativeHandleGL* nativeHandle =
        mStaticMeshNativeHandlePool.Get(hGpuStaticMesh.Get()->NativeHandlePoolIndex);
    if (!nativeHandle)
    {
        return eResourceTransferResult::Failed;
    }
    glDeleteBuffers(1, &nativeHandle->GlVbo);
    glDeleteBuffers(1, &nativeHandle->GlEbo);
    mStaticMeshNativeHandlePool.Remove(hGpuStaticMesh.Get()->NativeHandlePoolIndex);
    hGpuStaticMesh.Get()->NativeHandlePoolIndex = 0;
    finishUnloadStaticMesh(hGpuStaticMesh);
    return eResourceTransferResult::Success;
}

IRenderingSystem::eResourceTransferResult RenderingSystemGL::unloadMaterial(GpuMaterialHandle hGpuMaterial)
{
    if (hGpuMaterial.IsNULL())
    {
        return eResourceTransferResult::Failed;
    }
    const MaterialNativeHandleGL* nativeHandle =
        mMaterialNativeHandlePool.Get(hGpuMaterial.Get()->NativeHandlePoolIndex);
    if (!nativeHandle)
    {
        return eResourceTransferResult::Failed;
    }
    glDeleteBuffers(1, &nativeHandle->GlUbo);
    mMaterialNativeHandlePool.Remove(hGpuMaterial.Get()->NativeHandlePoolIndex);
    hGpuMaterial.Get()->NativeHandlePoolIndex = 0;
    finishUnloadMaterial(hGpuMaterial);
    return eResourceTransferResult::Success;
}

IRenderingSystem::eResourceTransferResult RenderingSystemGL::unloadTexture(GpuTextureHandle hGpuTexture)
{
    if (hGpuTexture.IsNULL())
    {
        return eResourceTransferResult::Failed;
    }
    const TextureNativeHandleGL* nativeHandle = mTextureNativeHandlePool.Get(hGpuTexture.Get()->NativeHandlePoolIndex);
    if (!nativeHandle)
    {
        return eResourceTransferResult::Failed;
    }
    glDeleteTextures(1, &nativeHandle->GlTexture);
    mTextureNativeHandlePool.Remove(hGpuTexture.Get()->NativeHandlePoolIndex);
    hGpuTexture.Get()->NativeHandlePoolIndex = 0;
    finishUnloadTexture(hGpuTexture);
    return eResourceTransferResult::Success;
}

IRenderingSystem::eResourceTransferResult RenderingSystemGL::unloadShader(GpuShaderHandle hGpuShader)
{
    if (hGpuShader.IsNULL())
    {
        return eResourceTransferResult::Failed;
    }
    const ShaderNativeHandleGL* nativeHandle = mShaderNativeHandlePool.Get(hGpuShader.Get()->NativeHandlePoolIndex);
    if (!nativeHandle)
    {
        return eResourceTransferResult::Failed;
    }
    glDeleteProgram(nativeHandle->GlProgram);
    mShaderNativeHandlePool.Remove(hGpuShader.Get()->NativeHandlePoolIndex);
    hGpuShader.Get()->NativeHandlePoolIndex = 0;
    finishUnloadShader(hGpuShader);
    return eResourceTransferResult::Success;
}

void RenderingSystemGL::cancelInitialization()
{
    glDeleteVertexArrays(1, &mGlVAO);
    glDeleteProgramPipelines(1, &mGlProgramPipeline);
    glDeleteProgram(mGlDefaultVS);
    glDeleteProgram(mGlDefaultPhongFS);
    glDeleteProgram(mGlDefaultPhongMaskedFS);
    glDeleteProgram(mGlDefaultPbrFS);
    glDeleteProgram(mGlDefaultPbrMaskedFS);
    glDeleteProgram(mGlDefaultSkyMapVS);
    glDeleteProgram(mGlDefaultSkyMapFS);
    glDeleteProgram(mGlDefaultWorldGizmoVS);
    glDeleteProgram(mGlDefaultWorldGizmoFS);
    glDeleteTextures(1, &mGlDefaultCubeMap);
    glDeleteBuffers(1, &mGlMatrixUBO);
    glDeleteBuffers(1, &mGlLightUBO);
    glDeleteBuffers(1, &mGlGeneralPurposeUBO);
}

void RenderingSystemGL::cancelFrameBufferCreation(
    GLuint* glFBO,
    GLuint* glResolvedFBO,
    GLuint (*pGlRenderTargets)[static_cast<int32_t>(eRenderTargetType::Last)],
    GLuint (*pGlResolvedRenderTargets)[static_cast<int32_t>(eRenderTargetType::Last)])
{
    if (pGlRenderTargets)
    {
        glDeleteTextures(static_cast<int32_t>(eRenderTargetType::Last), pGlRenderTargets[0]);
        glDeleteTextures(static_cast<int32_t>(eRenderTargetType::Last), pGlRenderTargets[1]);
    }
    if (pGlResolvedRenderTargets)
    {
        glDeleteTextures(static_cast<int32_t>(eRenderTargetType::Last), pGlResolvedRenderTargets[0]);
        glDeleteTextures(static_cast<int32_t>(eRenderTargetType::Last), pGlResolvedRenderTargets[1]);
    }
    glDeleteFramebuffers(2, glFBO);
    glDeleteFramebuffers(2, glResolvedFBO);
}

GLenum toGlRenderTargetFormat(IRenderingSystem::eRenderTargetFormat format)
{
    switch (format)
    {
        case IRenderingSystem::eRenderTargetFormat::R8G8B8A8_UNORM:
        case IRenderingSystem::eRenderTargetFormat::B8G8R8A8_UNORM:
            return GL_RGBA8;
        case IRenderingSystem::eRenderTargetFormat::R16G16B16A16_FLOAT:
            return GL_RGBA16F;
        case IRenderingSystem::eRenderTargetFormat::R32_FLOAT:
            return GL_R32F;
        case IRenderingSystem::eRenderTargetFormat::R16G16_FLOAT:
            return GL_RG16F;
        case IRenderingSystem::eRenderTargetFormat::R32G32B32A32_FLOAT:
            return GL_RGBA32F;
        case IRenderingSystem::eRenderTargetFormat::D24_UNORM_S8_UINT:
            return GL_DEPTH24_STENCIL8;
        case IRenderingSystem::eRenderTargetFormat::D32_FLOAT:
            return GL_DEPTH_COMPONENT32F;
        case IRenderingSystem::eRenderTargetFormat::None:
        default:
            HO_ASSERT(false, "Invalid render target format.");
            return GL_NONE;
    }
}

GLenum toGlTextureType(eTextureType type)
{
    switch (type)
    {
        case eTextureType::Texture1D:
            return GL_TEXTURE_1D;
        case eTextureType::Texture2D:
            return GL_TEXTURE_2D;
        case eTextureType::Texture3D:
            return GL_TEXTURE_3D;
        case eTextureType::TextureCubeMap:
            return GL_TEXTURE_CUBE_MAP;
        case eTextureType::Texture1DArray:
            return GL_TEXTURE_1D_ARRAY;
        case eTextureType::Texture2DArray:
            return GL_TEXTURE_2D_ARRAY;
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
            return GL_RG;
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::R8G8B8A8_SRGB:
        case eTextureFormat::R16G16B16A16_FLOAT:
            return GL_RGBA;
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
        case eTextureFormat::R8_SRGB:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R8G8B8A8_SRGB:
            return GL_UNSIGNED_BYTE;
        case eTextureFormat::R16_FLOAT:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R16G16B16A16_FLOAT:
            return GL_HALF_FLOAT;
        case eTextureFormat::R32_FLOAT:
            return GL_FLOAT;
        default:
            HO_ASSERT(false, "Invalid or unsupported texture format.");
            return GL_NONE;
    }
}

bool isUncompressedFormat(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::R8_SRGB:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R8G8B8A8_SRGB:
        case eTextureFormat::R16_FLOAT:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R16G16B16A16_FLOAT:
        case eTextureFormat::R32_FLOAT:
            return true;
        case eTextureFormat::BC1_UNORM:
        case eTextureFormat::BC1_SRGB:
        case eTextureFormat::BC3_UNORM:
        case eTextureFormat::BC3_SRGB:
        case eTextureFormat::BC4_UNORM:
        case eTextureFormat::BC4_SNORM:
        case eTextureFormat::BC5_UNORM:
        case eTextureFormat::BC5_SNORM:
        case eTextureFormat::BC6H_UF16:
        case eTextureFormat::BC6H_SF16:
        case eTextureFormat::BC7_UNORM:
        case eTextureFormat::BC7_SRGB:
            return false;
        default:
            HO_ASSERT(false, "Invalid texture format.");
            return false;
    }
}

GLenum toGlShaderType(eShaderStage stage)
{
    switch (stage)
    {
        case eShaderStage::VertexShader:
            return GL_VERTEX_SHADER;
        case eShaderStage::FragmentShader:
            return GL_FRAGMENT_SHADER;
        default:
            HO_ASSERT(false, "Invalid shader stage.");
            return GL_NONE;
    }
}

GLbitfield toGlShaderBitfield(eShaderStage stage)
{
    switch (stage)
    {
        case eShaderStage::VertexShader:
            return GL_VERTEX_SHADER_BIT;
        case eShaderStage::FragmentShader:
            return GL_FRAGMENT_SHADER_BIT;
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
            HO_ASSERT(
                false,
                "An attempt has been made to perform an operation that would cause an internal stack to underflow.");
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

} // namespace ho