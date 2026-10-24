#include "IRenderingBackend.h"

#include "../RenderData/UIViewportData.h"
#include "GpuBufferLayouts.h"
#include "Subsystems/Asset/Assets.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
IRenderingBackend::IRenderingBackend() {}

IRenderingBackend::~IRenderingBackend()
{
    HO_ASSERT(mFrameBufferPool.IsEmpty(),
              "GPU FrameBuffers leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mGpuBufferPool.IsEmpty(), "GPU Buffer leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mGpuStaticMeshPool.IsEmpty(),
              "GPU StaticMeshes leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mGpuMaterialPool.IsEmpty(), "GPU Materials leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mGpuShaderPool.IsEmpty(), "GPU Shaders leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mGpuTexturePool.IsEmpty(), "GPU Textures leaked. Release all resources before destroying the backend.");
    HO_ASSERT(mGpuMorphTargetsPool.IsEmpty(),
              "GPU MorphTargets leaked. Release all resources before destroying the backend.");
}

bool IRenderingBackend::Init()
{
    SetAlphaBlendMode(mCurrentState.AlphaBlendMode);
    SetPolygonMode(mCurrentState.PolygonMode);
    SetCullMode(mCurrentState.CullMode);

    SetDepthTest(mCurrentState.bDepthTestEnabled);
    SetDepthWrite(mCurrentState.bDepthWriteEnabled);
    SetDepthFunc(mCurrentState.DepthFunc);

    SetStencilTest(mCurrentState.bStencilTestEnabled);
    SetStencilWriteMask(mCurrentState.StencilWriteMask);
    SetStencilState(mCurrentState.StencilFunc, mCurrentState.StencilCompareMask);
    SetStencilReference(mCurrentState.StencilReference);
    SetStencilOperator(mCurrentState.StencilFailOp, mCurrentState.DepthFailOp, mCurrentState.DepthPassOp);

    return true;
}

void IRenderingBackend::BeginFrame(uint64_t currentFrame)
{
    HO_ASSERT(!mCurrentState.bFrameBegun, "Frame already begun.");
    mCurrentState.bFrameBegun = true;
    mCurrentState.CurrentFrame = currentFrame;
}

void IRenderingBackend::EndFrame()
{
    HO_ASSERT(mCurrentState.bFrameBegun, "Frame already ended.");
    mCurrentState.bFrameBegun = false;

    // Update current bound resources last used frame count.

    if (mCurrentState.hBoundFrameBuffer.IsValid())
    {
        for (const FrameBuffer::TargetBindingState& colorTargetState :
             mCurrentState.hBoundFrameBuffer->ColorTargetStates)
        {
            if (colorTargetState.hTarget.IsValid())
            {
                colorTargetState.hTarget->LastUsedFrame = mCurrentState.CurrentFrame;
            }
            if (mCurrentState.hBoundFrameBuffer->DepthTargetState.hTarget.IsValid())
            {
                mCurrentState.hBoundFrameBuffer->DepthTargetState.hTarget->LastUsedFrame = mCurrentState.CurrentFrame;
            }
        }
    }

    for (const BufferBindingState& bufferBindingState : mCurrentState.BufferBindingStates)
    {
        if (bufferBindingState.hBoundBuffer.IsValid())
        {
            bufferBindingState.hBoundBuffer->LastUsedFrame = mCurrentState.CurrentFrame;
        }
    }

    if (mCurrentState.hBoundStaticMesh.IsValid())
    {
        mCurrentState.hBoundStaticMesh->LastUsedFrame = mCurrentState.CurrentFrame;
    }
    if (mCurrentState.hBoundMaterial.IsValid())
    {
        mCurrentState.hBoundMaterial->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    for (const GpuTextureHandle& hBoundTexture : mCurrentState.hBoundTextures)
    {
        if (hBoundTexture.IsValid())
        {
            hBoundTexture->LastUsedFrame = mCurrentState.CurrentFrame;
        }
    }
    for (auto& storageTextureBindingState : mCurrentState.StorageTextureBindingStates)
    {
        if (storageTextureBindingState.hBoundTexture.IsValid())
        {
            storageTextureBindingState.hBoundTexture->LastUsedFrame = mCurrentState.CurrentFrame;
        }
    }
    for (const GpuShaderHandle& hBoundShader : mCurrentState.hBoundShaders)
    {
        if (hBoundShader.IsValid())
        {
            hBoundShader->LastUsedFrame = mCurrentState.CurrentFrame;
        }
    }
    if (mCurrentState.hBoundMorphTargets.IsValid())
    {
        mCurrentState.hBoundMorphTargets->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    // Unbind all binding resources.
    clearResourceBindingState();
}

void IRenderingBackend::AttachColorTarget(FrameBufferHandle hFrameBuffer,
                                          eRenderTargetBindingIndex targetIndex,
                                          GpuTextureHandle hTarget)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    AttachColorTargetRange(hFrameBuffer, targetIndex, hTarget, 0, 1, 0);
}

void IRenderingBackend::AttachDepthTarget(FrameBufferHandle hFrameBuffer, GpuTextureHandle hTarget)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    AttachDepthTargetRange(hFrameBuffer, hTarget, 0, 1, 0);
}

void IRenderingBackend::UploadMaterialAttributes(GpuMaterialHandle hMaterial, const MaterialAttributeLayout& attributes)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (hMaterial.IsNULL())
    {
        HO_ASSERT(false, "Invalid gpu material handle. It might be not created by 'prepareUploadMaterial' yet.");
        return;
    }

    if (hMaterial->State == eGpuResourceState::PendingUpload)
    {
        HO_ASSERT(false, "This material is not uploaded yet.");
        return;
    }

    HO_ASSERT(hMaterial->hAttributeBuffer.IsValid() &&
                  hMaterial->hAttributeBuffer->State != eGpuResourceState::PendingUpload,
              "Material was created incompletely.");

    UploadBufferData(hMaterial->hAttributeBuffer, &attributes, static_cast<int32_t>(sizeof(MaterialAttributeLayout)));
}

void IRenderingBackend::BindMaterialAttributes(GpuMaterialHandle hMaterial)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    if (hMaterial.IsNULL())
    {
        HO_ASSERT(false, "Invalid gpu material handle. It might be not created by 'prepareUploadMaterial' yet.");
        return;
    }

    if (hMaterial->State == eGpuResourceState::PendingUpload)
    {
        HO_ASSERT(false, "This material is not uploaded yet.");
        return;
    }

    HO_ASSERT(hMaterial->hAttributeBuffer.IsValid() &&
                  hMaterial->hAttributeBuffer->State != eGpuResourceState::PendingUpload,
              "Material was created incompletely.");

    if (mCurrentState.hBoundMaterial == hMaterial)
    {
        return;
    }

    // Update prev material Last used frame
    if (mCurrentState.hBoundMaterial.IsValid())
    {
        mCurrentState.hBoundMaterial->LastUsedFrame = mCurrentState.CurrentFrame;
    }

    BindBuffer(hMaterial->hAttributeBuffer, eBufferBindingIndex::MaterialAttributes);
    mCurrentState.hBoundMaterial = hMaterial;
}

int32_t IRenderingBackend::EvictStaleResources(uint64_t frameThreshold)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    int32_t evictedCount = 0;

    for (auto& staticMesh : mGpuStaticMeshPool)
    {
        if (!staticMesh.bPersistent && staticMesh.State != eGpuResourceState::PendingUpload &&
            (mCurrentState.CurrentFrame - staticMesh.LastUsedFrame) > frameThreshold)
        {
            UnloadStaticMesh(staticMesh.hThis);
            ++evictedCount;
        }
    }
    for (auto& material : mGpuMaterialPool)
    {
        if (!material.bPersistent && material.State != eGpuResourceState::PendingUpload &&
            (mCurrentState.CurrentFrame - material.LastUsedFrame) > frameThreshold)
        {
            UnloadMaterial(material.hThis);
            ++evictedCount;
        }
    }
    for (auto& texture : mGpuTexturePool)
    {
        if (texture.FrameBufferAttachmentCount == 0 && !texture.bPersistent &&
            texture.State != eGpuResourceState::PendingUpload &&
            (mCurrentState.CurrentFrame - texture.LastUsedFrame) > frameThreshold)
        {
            UnloadTexture(texture.hThis);
            ++evictedCount;
        }
    }
    for (auto& shader : mGpuShaderPool)
    {
        if (!shader.bPersistent && shader.State != eGpuResourceState::PendingUpload &&
            (mCurrentState.CurrentFrame - shader.LastUsedFrame) > frameThreshold)
        {
            UnloadShader(shader.hThis);
            ++evictedCount;
        }
    }
    for (auto& morphTargets : mGpuMorphTargetsPool)
    {
        if (!morphTargets.bPersistent && morphTargets.State != eGpuResourceState::PendingUpload &&
            (mCurrentState.CurrentFrame - morphTargets.LastUsedFrame) > frameThreshold)
        {
            UnloadMorphTargets(morphTargets.hThis);
            ++evictedCount;
        }
    }
    for (auto& buffer : mGpuBufferPool)
    {
        if (!buffer.bPersistent && buffer.State != eGpuResourceState::PendingUpload &&
            (mCurrentState.CurrentFrame - buffer.LastUsedFrame) > frameThreshold)
        {
            UnloadBuffer(buffer.hThis);
            ++evictedCount;
        }
    }
    return evictedCount;
}

GpuBufferHandle IRenderingBackend::PrepareCreateBuffer(int32_t byteSize,
                                                       eGpuBufferUsage usage,
                                                       eGpuBufferAccess access,
                                                       bool bPersistent)
{
    HO_ASSERT(byteSize > 0, "Byte size must be greater than zero.");
    const uint64_t id = mGpuBufferPool.Add(GpuBuffer());
    GpuBufferHandle hNewGpuBuffer = GpuBufferHandle(&mGpuBufferPool, id);

    hNewGpuBuffer->ByteSize = byteSize;
    hNewGpuBuffer->Usage = usage;
    hNewGpuBuffer->Access = access;

    hNewGpuBuffer->LastUsedFrame = 0;
    hNewGpuBuffer->hThis = hNewGpuBuffer;
    hNewGpuBuffer->bPersistent = bPersistent;
    hNewGpuBuffer->State = eGpuResourceState::PendingUpload;
    return hNewGpuBuffer;
}

GpuStaticMeshHandle IRenderingBackend::PrepareUploadStaticMesh(ObjectHandle<RStaticMeshAsset> hStaticMesh,
                                                               bool bPersistent)
{
    if (hStaticMesh.IsValid())
    {
        const uint64_t id = mGpuStaticMeshPool.Add(GpuStaticMesh());
        GpuStaticMeshHandle hNewGpuStaticMesh = GpuStaticMeshHandle(&mGpuStaticMeshPool, id);
        hStaticMesh->hRenderProxy = hNewGpuStaticMesh;

        hNewGpuStaticMesh->hOrigin = hStaticMesh;
        hNewGpuStaticMesh->LastUsedFrame = 0;
        hNewGpuStaticMesh->hThis = hNewGpuStaticMesh;
        hNewGpuStaticMesh->bPersistent = bPersistent;
        hNewGpuStaticMesh->State = eGpuResourceState::PendingUpload;
        return hNewGpuStaticMesh;
    }
    else
    {
        HO_ASSERT(false, "Original asset is invalid.");
        return GpuStaticMeshHandle::sNULL;
    }
}

GpuMaterialHandle IRenderingBackend::PrepareUploadMaterial(ObjectHandle<RMaterialAsset> hMaterial, bool bPersistent)
{
    if (hMaterial.IsValid())
    {
        const uint64_t id = mGpuMaterialPool.Add(GpuMaterial());
        GpuMaterialHandle hNewGpuMaterial = GpuMaterialHandle(&mGpuMaterialPool, id);
        hMaterial->hRenderProxy = hNewGpuMaterial;

        hNewGpuMaterial->PipelineState.AlphaMode = hMaterial->PipelineState.AlphaMode;
        hNewGpuMaterial->PipelineState.AlphaBlendMode = hMaterial->PipelineState.AlphaBlendMode;
        hNewGpuMaterial->PipelineState.bWireframe = hMaterial->PipelineState.bWireframe;
        hNewGpuMaterial->PipelineState.bBackfaceCulling = hMaterial->PipelineState.bBackfaceCulling;

        hNewGpuMaterial->Type = hMaterial->Type;

        hNewGpuMaterial->hAttributeBuffer = PrepareCreateBuffer(static_cast<int32_t>(sizeof(MaterialAttributeLayout)),
                                                                eGpuBufferUsage::Uniform,
                                                                eGpuBufferAccess::Dynamic,
                                                                bPersistent);

        for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
        {
            hNewGpuMaterial->bHasTexture[i] = hMaterial->hTextures[i].IsValid();
        }

        hNewGpuMaterial->hOrigin = hMaterial;
        hNewGpuMaterial->LastUsedFrame = 0;
        hNewGpuMaterial->hThis = hNewGpuMaterial;
        hNewGpuMaterial->bPersistent = bPersistent;
        hNewGpuMaterial->State = eGpuResourceState::PendingUpload;
        return hNewGpuMaterial;
    }
    else
    {
        HO_ASSERT(false, "Original asset is invalid.");
        return GpuMaterialHandle::sNULL;
    }
}

GpuTextureHandle IRenderingBackend::PrepareUploadTexture(ObjectHandle<RTextureAsset> hTexture, bool bPersistent)
{
    if (hTexture.IsValid())
    {
        const uint64_t id = mGpuTexturePool.Add(GpuTexture());
        GpuTextureHandle hNewGpuTexture = GpuTextureHandle(&mGpuTexturePool, id);
        hTexture->hRenderProxy = hNewGpuTexture;

        hNewGpuTexture->Width = hTexture->Width;
        hNewGpuTexture->Height = hTexture->Height;
        hNewGpuTexture->Depth = hTexture->Depth;
        hNewGpuTexture->ArraySize = hTexture->ArraySize;
        hNewGpuTexture->MipLevels = hTexture->MipLevels;
        hNewGpuTexture->Format = hTexture->Format;
        hNewGpuTexture->Type = hTexture->Type;

        hNewGpuTexture->hOrigin = hTexture;
        hNewGpuTexture->LastUsedFrame = 0;
        hNewGpuTexture->hThis = hNewGpuTexture;
        hNewGpuTexture->bPersistent = bPersistent;
        hNewGpuTexture->State = eGpuResourceState::PendingUpload;
        return hNewGpuTexture;
    }
    else
    {
        HO_ASSERT(false, "Original asset is invalid or mismatched with render target creation arguments.");
        return GpuTextureHandle::sNULL;
    }
}

GpuShaderHandle IRenderingBackend::PrepareUploadShader(ObjectHandle<RShaderAsset> hShader, bool bPersistent)
{
    if (hShader.IsValid())
    {
        const uint64_t id = mGpuShaderPool.Add(GpuShader());
        GpuShaderHandle hNewGpuShader = GpuShaderHandle(&mGpuShaderPool, id);
        hShader->hRenderProxy = hNewGpuShader;

        hNewGpuShader->hOrigin = hShader;
        hNewGpuShader->LastUsedFrame = 0;
        hNewGpuShader->hThis = hNewGpuShader;
        hNewGpuShader->bPersistent = bPersistent;
        hNewGpuShader->State = eGpuResourceState::PendingUpload;
        return hNewGpuShader;
    }
    else
    {
        HO_ASSERT(false, "Original asset is invalid.");
        return GpuShaderHandle::sNULL;
    }
}

GpuMorphTargetsHandle IRenderingBackend::PrepareUploadMorphTargets(ObjectHandle<RMorphTargetsAsset> hMorphTargets,
                                                                   bool bPersistent)
{
    if (hMorphTargets.IsValid())
    {
        const uint64_t id = mGpuMorphTargetsPool.Add(GpuMorphTargets());
        GpuMorphTargetsHandle hNewGpuMorphTargets = GpuMorphTargetsHandle(&mGpuMorphTargetsPool, id);
        hMorphTargets->hRenderProxy = hNewGpuMorphTargets;

        hNewGpuMorphTargets->hOrigin = hMorphTargets;
        hNewGpuMorphTargets->LastUsedFrame = 0;
        hNewGpuMorphTargets->hThis = hNewGpuMorphTargets;
        hNewGpuMorphTargets->bPersistent = bPersistent;
        hNewGpuMorphTargets->State = eGpuResourceState::PendingUpload;
        return hNewGpuMorphTargets;
    }
    else
    {
        HO_ASSERT(false, "Original asset is invalid.");
        return GpuMorphTargetsHandle::sNULL;
    }
}

GpuTextureHandle IRenderingBackend::PrepareCreateTexture1D(eTextureFormat format,
                                                           int32_t width,
                                                           int32_t mipLevels,
                                                           bool bPersistent)
{
    HO_ASSERT(format != eTextureFormat::None, "Invalid texture format");
    HO_ASSERT(width > 0 && mipLevels > 0, "Must be greater than zero.");

    const uint64_t id = mGpuTexturePool.Add(GpuTexture());
    GpuTextureHandle hNewGpuTexture = GpuTextureHandle(&mGpuTexturePool, id);

    hNewGpuTexture->Width = width;
    hNewGpuTexture->Height = 1;
    hNewGpuTexture->Depth = 1;
    hNewGpuTexture->ArraySize = 1;
    hNewGpuTexture->MipLevels = mipLevels;
    hNewGpuTexture->Format = format;
    hNewGpuTexture->Type = eTextureType::Texture1D;

    hNewGpuTexture->hOrigin = ObjectHandle<RTextureAsset>::GetNULL();
    hNewGpuTexture->LastUsedFrame = 0;
    hNewGpuTexture->hThis = hNewGpuTexture;
    hNewGpuTexture->bPersistent = bPersistent;
    hNewGpuTexture->State = eGpuResourceState::PendingUpload;
    return hNewGpuTexture;
}

GpuTextureHandle IRenderingBackend::PrepareCreateTexture2D(
    eTextureFormat format, int32_t width, int32_t height, int32_t mipLevels, int32_t sampleCount, bool bPersistent)
{
    HO_ASSERT(format != eTextureFormat::None, "Invalid texture format");
    HO_ASSERT(width > 0 && height > 0 && mipLevels > 0 && sampleCount > 0, "Must be greater than zero.");

    const uint64_t id = mGpuTexturePool.Add(GpuTexture());
    GpuTextureHandle hNewGpuTexture = GpuTextureHandle(&mGpuTexturePool, id);

    hNewGpuTexture->SampleCount = sampleCount;

    hNewGpuTexture->Width = width;
    hNewGpuTexture->Height = height;
    hNewGpuTexture->Depth = 1;
    hNewGpuTexture->ArraySize = 1;
    hNewGpuTexture->MipLevels = mipLevels;
    hNewGpuTexture->Format = format;
    hNewGpuTexture->Type = eTextureType::Texture2D;

    hNewGpuTexture->hOrigin = ObjectHandle<RTextureAsset>::GetNULL();
    hNewGpuTexture->LastUsedFrame = 0;
    hNewGpuTexture->hThis = hNewGpuTexture;
    hNewGpuTexture->bPersistent = bPersistent;
    hNewGpuTexture->State = eGpuResourceState::PendingUpload;
    return hNewGpuTexture;
}

GpuTextureHandle IRenderingBackend::PrepareCreateTexture3D(
    eTextureFormat format, int32_t width, int32_t height, int32_t depth, int32_t mipLevels, bool bPersistent)
{
    HO_ASSERT(format != eTextureFormat::None, "Invalid texture format");
    HO_ASSERT(width > 0 && height > 0 && depth > 0 && mipLevels > 0, "Must be greater than zero.");

    const uint64_t id = mGpuTexturePool.Add(GpuTexture());
    GpuTextureHandle hNewGpuTexture = GpuTextureHandle(&mGpuTexturePool, id);

    hNewGpuTexture->Width = width;
    hNewGpuTexture->Height = height;
    hNewGpuTexture->Depth = depth;
    hNewGpuTexture->ArraySize = 1;
    hNewGpuTexture->MipLevels = mipLevels;
    hNewGpuTexture->Format = format;
    hNewGpuTexture->Type = eTextureType::Texture3D;

    hNewGpuTexture->hOrigin = ObjectHandle<RTextureAsset>::GetNULL();
    hNewGpuTexture->LastUsedFrame = 0;
    hNewGpuTexture->hThis = hNewGpuTexture;
    hNewGpuTexture->bPersistent = bPersistent;
    hNewGpuTexture->State = eGpuResourceState::PendingUpload;
    return hNewGpuTexture;
}

GpuTextureHandle IRenderingBackend::PrepareCreateTextureCubeMap(
    eTextureFormat format, int32_t width, int32_t height, int32_t mipLevels, bool bPersistent)
{
    HO_ASSERT(format != eTextureFormat::None, "Invalid texture format");
    HO_ASSERT(width > 0 && height > 0 && mipLevels > 0, "Must be greater than zero.");

    const uint64_t id = mGpuTexturePool.Add(GpuTexture());
    GpuTextureHandle hNewGpuTexture = GpuTextureHandle(&mGpuTexturePool, id);

    hNewGpuTexture->Width = width;
    hNewGpuTexture->Height = height;
    hNewGpuTexture->Depth = 1;
    hNewGpuTexture->ArraySize = 1;
    hNewGpuTexture->MipLevels = mipLevels;
    hNewGpuTexture->Format = format;
    hNewGpuTexture->Type = eTextureType::TextureCubeMap;

    hNewGpuTexture->hOrigin = ObjectHandle<RTextureAsset>::GetNULL();
    hNewGpuTexture->LastUsedFrame = 0;
    hNewGpuTexture->hThis = hNewGpuTexture;
    hNewGpuTexture->bPersistent = bPersistent;
    hNewGpuTexture->State = eGpuResourceState::PendingUpload;
    return hNewGpuTexture;
}

GpuTextureHandle IRenderingBackend::PrepareCreateTexture1DArray(
    eTextureFormat format, int32_t width, int32_t arraySize, int32_t mipLevels, bool bPersistent)
{
    HO_ASSERT(format != eTextureFormat::None, "Invalid texture format");
    HO_ASSERT(width > 0 && arraySize > 0 && mipLevels > 0, "Must be greater than zero.");

    const uint64_t id = mGpuTexturePool.Add(GpuTexture());
    GpuTextureHandle hNewGpuTexture = GpuTextureHandle(&mGpuTexturePool, id);

    hNewGpuTexture->Width = width;
    hNewGpuTexture->Height = 1;
    hNewGpuTexture->Depth = 1;
    hNewGpuTexture->ArraySize = arraySize;
    hNewGpuTexture->MipLevels = mipLevels;
    hNewGpuTexture->Format = format;
    hNewGpuTexture->Type = eTextureType::Texture1DArray;

    hNewGpuTexture->hOrigin = ObjectHandle<RTextureAsset>::GetNULL();
    hNewGpuTexture->LastUsedFrame = 0;
    hNewGpuTexture->hThis = hNewGpuTexture;
    hNewGpuTexture->bPersistent = bPersistent;
    hNewGpuTexture->State = eGpuResourceState::PendingUpload;
    return hNewGpuTexture;
}

GpuTextureHandle IRenderingBackend::PrepareCreateTexture2DArray(eTextureFormat format,
                                                                int32_t width,
                                                                int32_t height,
                                                                int32_t arraySize,
                                                                int32_t mipLevels,
                                                                int32_t sampleCount,
                                                                bool bPersistent)
{
    HO_ASSERT(format != eTextureFormat::None, "Invalid texture format");
    HO_ASSERT(width > 0 && height > 0 && arraySize > 0 && mipLevels > 0 && sampleCount > 0,
              "Must be greater than zero.");

    const uint64_t id = mGpuTexturePool.Add(GpuTexture());
    GpuTextureHandle hNewGpuTexture = GpuTextureHandle(&mGpuTexturePool, id);

    hNewGpuTexture->SampleCount = sampleCount;

    hNewGpuTexture->Width = width;
    hNewGpuTexture->Height = height;
    hNewGpuTexture->Depth = 1;
    hNewGpuTexture->ArraySize = arraySize;
    hNewGpuTexture->MipLevels = mipLevels;
    hNewGpuTexture->Format = format;
    hNewGpuTexture->Type = eTextureType::Texture2DArray;

    hNewGpuTexture->hOrigin = ObjectHandle<RTextureAsset>::GetNULL();
    hNewGpuTexture->LastUsedFrame = 0;
    hNewGpuTexture->hThis = hNewGpuTexture;
    hNewGpuTexture->bPersistent = bPersistent;
    hNewGpuTexture->State = eGpuResourceState::PendingUpload;
    return hNewGpuTexture;
}

GpuTextureHandle IRenderingBackend::PrepareCreateTextureCubeMapArray(
    eTextureFormat format, int32_t width, int32_t height, int32_t arraySize, int32_t mipLevels, bool bPersistent)
{
    HO_ASSERT(format != eTextureFormat::None, "Invalid texture format");
    HO_ASSERT(width > 0 && height > 0 && arraySize > 0 && mipLevels > 0, "Must be greater than zero.");

    const uint64_t id = mGpuTexturePool.Add(GpuTexture());
    GpuTextureHandle hNewGpuTexture = GpuTextureHandle(&mGpuTexturePool, id);

    hNewGpuTexture->Width = width;
    hNewGpuTexture->Height = height;
    hNewGpuTexture->Depth = 1;
    hNewGpuTexture->ArraySize = arraySize;
    hNewGpuTexture->MipLevels = mipLevels;
    hNewGpuTexture->Format = format;
    hNewGpuTexture->Type = eTextureType::TextureCubeMapArray;

    hNewGpuTexture->hOrigin = ObjectHandle<RTextureAsset>::GetNULL();
    hNewGpuTexture->LastUsedFrame = 0;
    hNewGpuTexture->hThis = hNewGpuTexture;
    hNewGpuTexture->bPersistent = bPersistent;
    hNewGpuTexture->State = eGpuResourceState::PendingUpload;
    return hNewGpuTexture;
}

IRenderingBackend::eResourceTransferResult IRenderingBackend::UploadMaterial(GpuMaterialHandle hPreparedGpuMaterial)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hPreparedGpuMaterial.IsValid(),
              "Invalid gpu material handle. It might be not created by 'prepareUploadMaterial' yet.");
    HO_ASSERT(hPreparedGpuMaterial->State == eGpuResourceState::PendingUpload,
              "PrepareUploadMaterial must be called before upload.");
    HO_ASSERT(hPreparedGpuMaterial->hAttributeBuffer.IsValid() &&
                  hPreparedGpuMaterial->hAttributeBuffer->State == eGpuResourceState::PendingUpload,
              "Material was prepared incompletely. Check 'prepareUploadMaterial' function.");
    if (hPreparedGpuMaterial->hOrigin.IsNULL())
    {
        HO_ASSERT(false, "Invalid material asset. It might be removed from asset system while pending.");
        return eResourceTransferResult::Failed;
    }

    if (CreateBuffer(hPreparedGpuMaterial->hAttributeBuffer) == eResourceTransferResult::Failed)
    {
        return eResourceTransferResult::Failed;
    }

    const ObjectHandle<const RMaterialAsset> hOrigin = hPreparedGpuMaterial->hOrigin;

    MaterialAttributeLayout attributes;
    attributes.Ambient = Vector4(hOrigin->Ambient);
    attributes.Diffuse = Vector4(hOrigin->Diffuse);
    attributes.Specular = Vector4(hOrigin->Specular);
    attributes.Albedo = Vector4(hOrigin->Albedo);
    attributes.Emissive = Vector4(hOrigin->Emissive);
    attributes.Shininess = hOrigin->Shininess;
    attributes.Metallic = hOrigin->Metallic;
    attributes.Roughness = hOrigin->Roughness;
    attributes.IndexOfRefraction = hOrigin->IndexOfRefraction;
    attributes.Opacity = hOrigin->Opacity;
    attributes.AlphaThreshold = hOrigin->AlphaThreshold;
    attributes.EmissiveIntensity = hOrigin->EmissiveIntensity;
    attributes.NormalScale = hOrigin->NormalScale;
    attributes.OcclusionStrength = hOrigin->OcclusionStrength;

    for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
    {
        attributes.UVChannels[i].Value = hOrigin->UVChannels[i];
        const Matrix4x4 uvMatrix(Vector4(hOrigin->UVTransforms[i].GetCol0()),
                                 Vector4(hOrigin->UVTransforms[i].GetCol1()),
                                 Vector4(hOrigin->UVTransforms[i].GetCol2()),
                                 Vector4::sUnitW);
        attributes.UVMatrices[i] = uvMatrix;
        attributes.bHasTexture[i].Value = static_cast<int32_t>(hOrigin->hTextures[i].IsValid());
    }

    if (UploadBufferData(hPreparedGpuMaterial->hAttributeBuffer,
                         &attributes,
                         static_cast<int32_t>(sizeof(MaterialAttributeLayout)),
                         0) == eResourceTransferResult::Failed)
    {
        return eResourceTransferResult::Failed;
    }

    hPreparedGpuMaterial->LastUsedFrame = mCurrentState.CurrentFrame;
    hPreparedGpuMaterial->State = eGpuResourceState::UpToDate;
    return eResourceTransferResult::Success;
}

IRenderingBackend::eResourceTransferResult IRenderingBackend::UnloadMaterial(GpuMaterialHandle hGpuMaterial)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    HO_ASSERT(hGpuMaterial->State != eGpuResourceState::PendingUpload, "Material is not uploaded yet.");
    HO_ASSERT(hGpuMaterial->hAttributeBuffer.IsValid() &&
                  hGpuMaterial->hAttributeBuffer->State != eGpuResourceState::PendingUpload,
              "Material was created incompletely.");
    if (UnloadBuffer(hGpuMaterial->hAttributeBuffer) == eResourceTransferResult::Failed)
    {
        return eResourceTransferResult::Failed;
    }

    finishUnloadMaterial(hGpuMaterial);
    return eResourceTransferResult::Success;
}

void IRenderingBackend::finishUnloadBuffer(GpuBufferHandle hUnloadedGpuBuffer)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (hUnloadedGpuBuffer.IsValid())
    {
        HO_ASSERT(hUnloadedGpuBuffer->State != eGpuResourceState::PendingUpload, "Resource is not uploaded yet.");
        mGpuBufferPool.Remove(hUnloadedGpuBuffer.GetID());
    }
}

void IRenderingBackend::finishUnloadStaticMesh(GpuStaticMeshHandle hUnloadedGpuStaticMesh)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (hUnloadedGpuStaticMesh.IsValid())
    {
        HO_ASSERT(hUnloadedGpuStaticMesh->State != eGpuResourceState::PendingUpload, "Resource is not uploaded yet.");
        mGpuStaticMeshPool.Remove(hUnloadedGpuStaticMesh.GetID());
    }
}

void IRenderingBackend::finishUnloadMaterial(GpuMaterialHandle hUnloadedGpuMaterial)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (hUnloadedGpuMaterial.IsValid())
    {
        HO_ASSERT(hUnloadedGpuMaterial->State != eGpuResourceState::PendingUpload, "Resource is not uploaded yet.");
        // TODO: needs to handle bound textures, shaders on material.
        // Currently, bound textures, shaders will be unloaded on EvictStaleResoruces.
        // What if Its persistent material?
        mGpuMaterialPool.Remove(hUnloadedGpuMaterial.GetID());
    }
}

void IRenderingBackend::finishUnloadTexture(GpuTextureHandle hUnloadedGpuTexture)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (hUnloadedGpuTexture.IsValid())
    {
        HO_ASSERT(hUnloadedGpuTexture->State != eGpuResourceState::PendingUpload, "Resource is not uploaded yet.");
        mGpuTexturePool.Remove(hUnloadedGpuTexture.GetID());
    }
}

void IRenderingBackend::finishUnloadShader(GpuShaderHandle hUnloadedGpuShader)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (hUnloadedGpuShader.IsValid())
    {
        HO_ASSERT(hUnloadedGpuShader->State != eGpuResourceState::PendingUpload, "Resource is not uploaded yet.");
        mGpuShaderPool.Remove(hUnloadedGpuShader.GetID());
    }
}

void IRenderingBackend::finishUnloadMorphTargets(GpuMorphTargetsHandle hUnloadedGpuMorphTargets)
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");

    if (hUnloadedGpuMorphTargets.IsValid())
    {
        HO_ASSERT(hUnloadedGpuMorphTargets->State != eGpuResourceState::PendingUpload, "Resource is not uploaded yet.");
        mGpuMorphTargetsPool.Remove(hUnloadedGpuMorphTargets.GetID());
    }
}

void IRenderingBackend::UnloadAllResources()
{
    HO_ASSERT(mCurrentState.bFrameBegun,
              "All rendering backend APIs must be invoked between BeginFrame() and EndFrame().");
    BindFrameBuffer(FrameBufferHandle::sNULL);
    for (auto& frameBuffer : mFrameBufferPool)
    {
        DestroyFrameBuffer(frameBuffer.hThis);
    }

    for (auto& staticMesh : mGpuStaticMeshPool)
    {
        UnloadStaticMesh(staticMesh.hThis);
    }
    for (auto& material : mGpuMaterialPool)
    {
        UnloadMaterial(material.hThis);
    }
    for (auto& texture : mGpuTexturePool)
    {
        UnloadTexture(texture.hThis);
    }
    for (auto& shader : mGpuShaderPool)
    {
        UnloadShader(shader.hThis);
    }
    for (auto& morphTargets : mGpuMorphTargetsPool)
    {
        UnloadMorphTargets(morphTargets.hThis);
    }

    for (auto& buffer : mGpuBufferPool)
    {
        UnloadBuffer(buffer.hThis);
    }
}

void IRenderingBackend::clearResourceBindingState()
{
    mCurrentState.hBoundFrameBuffer = FrameBufferHandle::sNULL;
    for (BufferBindingState& bufferBindingState : mCurrentState.BufferBindingStates)
    {
        bufferBindingState = BufferBindingState();
    }
    mCurrentState.hBoundStaticMesh = GpuStaticMeshHandle::sNULL;
    mCurrentState.hBoundMaterial = GpuMaterialHandle::sNULL;
    for (GpuTextureHandle& hBoundTexture : mCurrentState.hBoundTextures)
    {
        hBoundTexture = GpuTextureHandle::sNULL;
    }
    for (StorageTextureBindingState& storageTextureBindingState : mCurrentState.StorageTextureBindingStates)
    {
        storageTextureBindingState = StorageTextureBindingState();
    }
    for (GpuShaderHandle& hBoundShader : mCurrentState.hBoundShaders)
    {
        hBoundShader = GpuShaderHandle::sNULL;
    }
    mCurrentState.hBoundMorphTargets = GpuMorphTargetsHandle::sNULL;
}

} // namespace ho