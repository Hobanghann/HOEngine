#include "AssetImportFuncs.h"

#include <half.hpp>
#include <rdo_bc_encoder.h>
#include <shaderc/shaderc.hpp>
#include <stb_image_resize2.h>

#include "Assets.h"
#include "Core/IO/ModelIR.h"
#include "Core/IO/ShaderIR.h"
#include "Core/Log/Logger.h"
#include "Core/Templates/FixedArray.h"

namespace ho
{
namespace importer
{
// ===========================================================================
//  Private Function Declarations
// ===========================================================================

static eMeshPrimitiveType toAssetPrimitiveType(parser::MeshIR::ePrimitiveType type);
[[maybe_unused]] static eAnimExtrapolationMode toAssetExtrapolationMode(parser::AnimationIR::eExtrapolationMode mode);
[[maybe_unused]] static eAnimInterpolationMode toAssetInterpolationMode(parser::AnimationIR::eInterpolationMode mode);
static eMaterialAlphaMode toAssetAlphaMode(parser::MaterialIR::eAlphaMode mode);
static eMaterialAlphaBlendMode toAssetAlphaBlendMode(parser::MaterialIR::eAlphaBlendMode mode);
[[maybe_unused]] static eMaterialTextureUsage toAssetTextureUsage(parser::MaterialIR::eTextureUsage usage);
static parser::MaterialIR::eTextureUsage toIRTextureUsage(eMaterialTextureUsage usage);

static stbir_pixel_layout toStbiFormat(Image::eFormat format);
static bool isLinearImageFormat(Image::eFormat format);
static bool isIntegerImageFormat(Image::eFormat format);

static DXGI_FORMAT toDxgiFormat_rdo_bc(eTextureFormat format);
static bool isUncompressedTextureFormat(eTextureFormat format);
static bool isLinearTextureFormat(eTextureFormat format);
[[maybe_unused]] static bool isIntegerTextureFormat(eTextureFormat format);
static int32_t getTexturePixelBytes(eTextureFormat format);
static int32_t getTextureBlockCount(int32_t length);
static int32_t getTextureBlockBytes(eTextureFormat format);
static int32_t calculateMipLevels(int32_t width, int32_t height);

shaderc_shader_kind toShadercStageGLSL(eShaderStage stage);
// ============================================================================
// NOTE: The implementation of this shader compilation pipeline is based on
// the official Google Shaderc C++ API reference examples.
// Source: shaderc/examples/online-compile/main.cc
// ============================================================================
// Returns GLSL shader source text after preprocessing.
[[maybe_unused]] static std::string preprocessShaderGLSL(const shaderc::Compiler& compiler,
                                                         const std::string& sourceName,
                                                         shaderc_shader_kind kind,
                                                         const std::string& source);
// Compiles a shader to SPIR-V assembly. Returns the assembly text
// as a string.
[[maybe_unused]] static std::string compileFileToAssemblyGLSL(const shaderc::Compiler& compiler,
                                                              const std::string& sourceName,
                                                              shaderc_shader_kind kind,
                                                              const std::string& source,
                                                              bool optimize = false);
// Compiles a shader to a SPIR-V binary. Returns the binary as
// a vector of 32-bit words.
static std::vector<uint32_t> compileFileGLSL(const shaderc::Compiler& compiler,
                                             const std::string& sourceName,
                                             shaderc_shader_kind kind,
                                             const std::string& source,
                                             bool optimize = false);

// ===========================================================================
//  Public Function Definitions
// ===========================================================================

std::unique_ptr<StaticMeshAsset> importStaticMesh(
    const parser::MeshIR& srcMeshIR,
    const std::vector<std::unique_ptr<const parser::MaterialIR>>& pMaterialIRs,
    const parser::SkeletonIR& srcSkeletonIR,
    const parser::SkinIR& srcSkinIR)
{
    std::unique_ptr<StaticMeshAsset> pNewMesh = std::make_unique<StaticMeshAsset>();
    const std::string nameStr =
        srcMeshIR.ResourcePath.RemovedExtension().ToString() + "::StaticMesh::" + srcMeshIR.NameStr;
    pNewMesh->hName = StringHandle(nameStr);
    int32_t allPositionCount = 0;
    int32_t allNormalCount = 0;
    int32_t allTangentCount = 0;
    int32_t allUVCount = 0;
    int32_t allColorCount = 0;
    int32_t allIndexCount = 0;
    for (const auto& subMeshIR : srcMeshIR.SubMeshes)
    {
        allPositionCount += static_cast<int32_t>(subMeshIR.Positions.size());
        allNormalCount += static_cast<int32_t>(subMeshIR.Normals.size());
        allTangentCount += static_cast<int32_t>(subMeshIR.Tangents.size());
        allUVCount += static_cast<int32_t>(subMeshIR.UVs.size());
        allColorCount += static_cast<int32_t>(subMeshIR.Colors.size());
        allIndexCount += static_cast<int32_t>(subMeshIR.Indices.size());
    }

    FixedArray<Vector3> positions(allPositionCount);
    FixedArray<Vector3> normals(allNormalCount);
    FixedArray<Vector4> tangents(allTangentCount);
    FixedArray<std::array<Vector2, StaticMeshAsset::sMaxUVChannel>> uvs(allUVCount);
    FixedArray<std::array<Color32, StaticMeshAsset::sMaxColorChannel>> colors(allColorCount);
    FixedArray<uint32_t> indices(allIndexCount);

    int32_t positionOffset = 0;
    int32_t normalOffset = 0;
    int32_t tangentOffset = 0;
    int32_t uvOffset = 0;
    int32_t colorOffset = 0;
    int32_t indexOffset = 0;
    FixedArray<StaticMeshAsset::SubMesh> subMeshes(static_cast<int32_t>(srcMeshIR.SubMeshes.size()));
    for (int32_t smi = 0; smi < static_cast<int32_t>(srcMeshIR.SubMeshes.size()); ++smi)
    {
        const parser::MeshIR::SubMesh& subMeshIR = srcMeshIR.SubMeshes[smi];

        subMeshes[smi].hName = StringHandle(subMeshIR.NameStr);

        subMeshes[smi].PrimitiveType = toAssetPrimitiveType(subMeshIR.PrimitiveType);

        for (int32_t i = 0; i < static_cast<int32_t>(subMeshIR.Positions.size()); ++i)
        {
            positions[positionOffset + i] = subMeshIR.Positions[i];
        }
        for (int32_t i = 0; i < static_cast<int32_t>(subMeshIR.Normals.size()); ++i)
        {
            normals[normalOffset + i] = subMeshIR.Normals[i];
        }
        for (int32_t i = 0; i < static_cast<int32_t>(subMeshIR.Tangents.size()); ++i)
        {
            tangents[tangentOffset + i] = subMeshIR.Tangents[i];
        }
        for (int32_t i = 0; i < static_cast<int32_t>(subMeshIR.UVs.size()); ++i)
        {
            uvs[uvOffset + i] = subMeshIR.UVs[i];
        }
        for (int32_t i = 0; i < static_cast<int32_t>(subMeshIR.Colors.size()); ++i)
        {
            colors[colorOffset + i] = subMeshIR.Colors[i];
        }
        for (int32_t i = 0; i < static_cast<int32_t>(subMeshIR.Indices.size()); ++i)
        {
            indices[indexOffset + i] = subMeshIR.Indices[i];
        }

        subMeshes[smi].PositionOffset = positionOffset;
        subMeshes[smi].NormalOffset = normalOffset;
        subMeshes[smi].TangentOffset = tangentOffset;
        subMeshes[smi].UVOffset = uvOffset;
        subMeshes[smi].ColorOffset = colorOffset;
        subMeshes[smi].VertexCount = static_cast<int32_t>(subMeshIR.Positions.size());

        subMeshes[smi].IndexOffset = indexOffset;
        subMeshes[smi].IndexCount = static_cast<int32_t>(subMeshIR.Indices.size());

        const parser::MaterialIR& matIR = *pMaterialIRs[srcMeshIR.SubMeshes[smi].RenderMaterialIndex];
        const std::string matNameStr =
            matIR.ResourcePath.RemovedExtension().ToString() + "::Material::" + matIR.NameStr;
        subMeshes[smi].hRenderMaterialName = StringHandle(matNameStr);

        subMeshes[smi].Aabb = subMeshIR.Aabb;
        subMeshes[smi].Sphere = subMeshIR.Sphere;

        positionOffset += static_cast<int32_t>(subMeshIR.Positions.size());
        normalOffset += static_cast<int32_t>(subMeshIR.Normals.size());
        tangentOffset += static_cast<int32_t>(subMeshIR.Tangents.size());
        uvOffset += static_cast<int32_t>(subMeshIR.UVs.size());
        colorOffset += static_cast<int32_t>(subMeshIR.Colors.size());
        indexOffset += static_cast<int32_t>(subMeshIR.Indices.size());
    }

    FixedArray<Transform3D> accTransforms(static_cast<int32_t>(srcSkeletonIR.BoneNameStrs.size()));
    for (int32_t i = 0; i < accTransforms.GetSize(); ++i)
    {
        accTransforms[i] = srcSkeletonIR.LocalTransforms[i];
    }

    for (int32_t i = 0; i < accTransforms.GetSize(); ++i)
    {
        const int32_t parentIdx = srcSkeletonIR.Parents[i];
        if (parentIdx != -1)
        {
            accTransforms[i] = accTransforms[parentIdx] * accTransforms[i];
        }
    }

    for (int32_t i = 0; i < accTransforms.GetSize(); ++i)
    {
        for (int32_t j = 0; j < static_cast<int32_t>(srcSkinIR.BindSubMeshIndices[i].size()); ++j)
        {
            const int32_t targetSubMeshIdx = srcSkinIR.BindSubMeshIndices[i][j];
            subMeshes[targetSubMeshIdx].LocalTransform = accTransforms[i];
        }
    }

    pNewMesh->Positions = std::move(positions);
    pNewMesh->Normals = std::move(normals);
    pNewMesh->Tangents = std::move(tangents);
    pNewMesh->UVs = std::move(uvs);
    pNewMesh->Colors = std::move(colors);
    pNewMesh->Indices = std::move(indices);
    pNewMesh->SubMeshes = std::move(subMeshes);

    pNewMesh->Aabb = srcMeshIR.Aabb;
    pNewMesh->Sphere = srcMeshIR.Sphere;

    return pNewMesh;
}

std::unique_ptr<MaterialAsset> importMaterial(const parser::MaterialIR& srcMaterialIR, eMaterialAssetType type)
{
    std::unique_ptr<MaterialAsset> pNewMaterial = std::make_unique<MaterialAsset>();
    const std::string nameStr =
        srcMaterialIR.ResourcePath.RemovedExtension().ToString() + "::Material::" + srcMaterialIR.NameStr;
    pNewMaterial->hName = StringHandle(nameStr);
    pNewMaterial->Type = type;

    pNewMaterial->PipelineState.AlphaMode = toAssetAlphaMode(srcMaterialIR.AlphaMode);
    pNewMaterial->PipelineState.AlphaBlendMode = toAssetAlphaBlendMode(srcMaterialIR.BlendMode);
    pNewMaterial->PipelineState.bWireframe = srcMaterialIR.bWireframe;
    pNewMaterial->PipelineState.bBackfaceCulling = srcMaterialIR.bBackfaceCulling;

    pNewMaterial->Ambient = srcMaterialIR.Ambient;
    pNewMaterial->Diffuse = srcMaterialIR.Diffuse;
    pNewMaterial->Specular = srcMaterialIR.Specular;
    pNewMaterial->Shininess = srcMaterialIR.Shininess;

    pNewMaterial->Albedo = srcMaterialIR.Albedo;
    pNewMaterial->Metallic = srcMaterialIR.Metallic;
    pNewMaterial->Roughness = srcMaterialIR.Roughness;
    pNewMaterial->IndexOfRefraction = srcMaterialIR.IndexOfRefraction;

    pNewMaterial->Opacity = srcMaterialIR.Opacity;
    pNewMaterial->AlphaThreshold = srcMaterialIR.AlphaThreshold;
    pNewMaterial->Emissive = srcMaterialIR.Emissive;
    pNewMaterial->EmissiveIntensity = srcMaterialIR.EmissiveIntensity;
    pNewMaterial->NormalScale = srcMaterialIR.NormalScale;
    pNewMaterial->OcclusionStrength = srcMaterialIR.OcclusionStrength;

    return pNewMaterial;
}

std::unique_ptr<MaterialAsset> importTexturedMaterial(
    const parser::MaterialIR& srcMaterialIR,
    eMaterialAssetType type,
    const std::vector<std::unique_ptr<const parser::TextureIR>>& pSrcTextureIRs)
{
    std::unique_ptr<MaterialAsset> pNewMaterial = importMaterial(srcMaterialIR, type);

    for (int32_t i = 1; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
    {
        const parser::MaterialIR::eTextureUsage texIrUsage = toIRTextureUsage(static_cast<eMaterialTextureUsage>(i));
        const int32_t texIndex = srcMaterialIR.TextureIRIndices[static_cast<int32_t>(texIrUsage)];
        if (texIndex == -1)
        {
            pNewMaterial->hTextureNames[i] = StringHandle::sNULL;
        }
        else
        {
            const std::string nameStr = pSrcTextureIRs[texIndex]->ResourcePath.RemovedExtension().ToString() +
                                        "::Texture::2D::" + pSrcTextureIRs[texIndex]->NameStr;
            pNewMaterial->hTextureNames[i] = StringHandle(nameStr);
        }
        pNewMaterial->UVChannels[i] = srcMaterialIR.UVChannels[static_cast<int32_t>(texIrUsage)];
        pNewMaterial->UVTransforms[i] = srcMaterialIR.UVTransforms[static_cast<int32_t>(texIrUsage)];
    }

    return pNewMaterial;
}

// Assumes srcTextureIR always uses a 4-channel memory layout (RGBA),
// regardless of the actual color channels or format of the source image.
// TODO: Refactor later. Doing way too much and BC compression logic is heavily duplicated.
std::unique_ptr<TextureAsset> importTexture2D(const parser::TextureIR& srcTextureIR,
                                              eTextureFormat dstFormat,
                                              bool bGenerateMipmap)

{
    if (srcTextureIR.Img.GetBitmap() == nullptr)
    {
        HO_ASSERT(false, "Source image was failed to parse.");
        return nullptr;
    }

    std::unique_ptr<TextureAsset> pNewTexture = std::make_unique<TextureAsset>();
    const std::string nameStr =
        srcTextureIR.ResourcePath.RemovedExtension().ToString() + "::Texture::2D::" + srcTextureIR.NameStr;
    pNewTexture->hName = StringHandle(nameStr);
    pNewTexture->Width = srcTextureIR.Img.GetWidth();
    pNewTexture->Height = srcTextureIR.Img.GetHeight();
    pNewTexture->Depth = 1;
    pNewTexture->ArraySize = 1;
    pNewTexture->MipLevels = bGenerateMipmap ? calculateMipLevels(pNewTexture->Width, pNewTexture->Height) : 1;
    pNewTexture->Format = dstFormat;
    pNewTexture->Type = eTextureType::Texture2D;

    // Construct sub texture layouts
    int32_t totalDataSize = 0;
    int32_t imgWidth = pNewTexture->Width;
    int32_t imgHeight = pNewTexture->Height;
    pNewTexture->Layouts = FixedArray<TextureAsset::SubTextureLayout>(pNewTexture->MipLevels);
    for (int32_t i = 0; i < pNewTexture->MipLevels; ++i)
    {
        pNewTexture->Layouts[i].Offset = totalDataSize;
        if (isUncompressedTextureFormat(dstFormat))
        {
            pNewTexture->Layouts[i].RowPitch = imgWidth * getTexturePixelBytes(dstFormat);
            pNewTexture->Layouts[i].Size = imgHeight * pNewTexture->Layouts[i].RowPitch;
        }
        else
        {
            pNewTexture->Layouts[i].RowPitch = getTextureBlockCount(imgWidth) * getTextureBlockBytes(dstFormat);
            pNewTexture->Layouts[i].Size = getTextureBlockCount(imgHeight) * pNewTexture->Layouts[i].RowPitch;
        }

        totalDataSize += pNewTexture->Layouts[i].Size;
        imgWidth = math::Max(1, static_cast<int32_t>(imgWidth / 2));
        imgHeight = math::Max(1, static_cast<int32_t>(imgHeight / 2));
    }
    pNewTexture->DataBlob = FixedArray<uint8_t>(totalDataSize);

    // Generate compressed mipmaps
    int32_t originImgWidth = pNewTexture->Width;
    int32_t originImgHeight = pNewTexture->Height;
    int32_t resizedImgWidth = originImgWidth;
    int32_t resizedImgHeight = originImgHeight;
    const int32_t imgPixelBytes = isIntegerImageFormat(srcTextureIR.Img.GetFormat()) ? 4 : 16;

    const int32_t texPixelBytes =
        isUncompressedTextureFormat(dstFormat) ? getTexturePixelBytes(dstFormat) : getTextureBlockBytes(dstFormat);
    if (texPixelBytes == 0)
    {
        HO_ASSERT(false, "Invalid texture pixel bytes (0).");
        return nullptr;
    }
    FixedArray<uint8_t> imgBuffers[2] = {FixedArray<uint8_t>(pNewTexture->Width * pNewTexture->Height * imgPixelBytes),
                                         FixedArray<uint8_t>(pNewTexture->Width * pNewTexture->Height * imgPixelBytes)};
    int32_t frontBufferIndex = 0;
    int32_t backBufferIndex = 1;
    // original image is in back buffer initially.
    for (int32_t i = 0; i < originImgWidth * originImgHeight * imgPixelBytes; ++i)
    {
        imgBuffers[backBufferIndex][i] = srcTextureIR.Img.GetBitmap()[i];
    }

    for (int32_t i = 0; i < pNewTexture->MipLevels; ++i)
    {
        const int32_t texWidth = pNewTexture->Layouts[i].RowPitch / texPixelBytes;
        const int32_t texHeight = pNewTexture->Layouts[i].Size / pNewTexture->Layouts[i].RowPitch;
        // Resize original image to half (from back buffer to front buffer)
        const void* pResizeSuccess = nullptr;
        switch (srcTextureIR.Img.GetFormat())
        {
            case Image::eFormat::R8_UNORM:
            case Image::eFormat::R8G8_UNORM:
            case Image::eFormat::R8G8B8_UNORM:
            case Image::eFormat::R8G8B8A8_UNORM:
                pResizeSuccess = stbir_resize_uint8_linear(imgBuffers[backBufferIndex].Data(),
                                                           originImgWidth,
                                                           originImgHeight,
                                                           originImgWidth * imgPixelBytes,
                                                           imgBuffers[frontBufferIndex].Data(),
                                                           resizedImgWidth,
                                                           resizedImgHeight,
                                                           resizedImgWidth * imgPixelBytes,
                                                           toStbiFormat(srcTextureIR.Img.GetFormat()));
                break;
            case Image::eFormat::R8_SRGB:
            case Image::eFormat::R8G8_SRGB:
            case Image::eFormat::R8G8B8_SRGB:
            case Image::eFormat::R8G8B8A8_SRGB:
                pResizeSuccess = stbir_resize_uint8_srgb(imgBuffers[backBufferIndex].Data(),
                                                         originImgWidth,
                                                         originImgHeight,
                                                         originImgWidth * imgPixelBytes,
                                                         imgBuffers[frontBufferIndex].Data(),
                                                         resizedImgWidth,
                                                         resizedImgHeight,
                                                         resizedImgWidth * imgPixelBytes,
                                                         toStbiFormat(srcTextureIR.Img.GetFormat()));
                break;
            case Image::eFormat::R32_FLOAT:
            case Image::eFormat::R32G32_FLOAT:
            case Image::eFormat::R32G32B32_FLOAT:
            case Image::eFormat::R32G32B32A32_FLOAT:
                pResizeSuccess =
                    stbir_resize_float_linear(reinterpret_cast<const float*>(imgBuffers[backBufferIndex].Data()),
                                              originImgWidth,
                                              originImgHeight,
                                              originImgWidth * imgPixelBytes,
                                              reinterpret_cast<float*>(imgBuffers[frontBufferIndex].Data()),
                                              resizedImgWidth,
                                              resizedImgHeight,
                                              resizedImgWidth * imgPixelBytes,
                                              toStbiFormat(srcTextureIR.Img.GetFormat()));
                break;
            default:
                HO_ASSERT(false, "Invalid image format.");
                return nullptr;
        }

        if (!pResizeSuccess)
        {
            HO_ASSERT(false, "Failed to resize image.");
            return nullptr;
        }

        // Compress current mipmap (from front buffer to dataBlob buffer)
        const uint8_t* pSrcImageBufferUint8 = imgBuffers[frontBufferIndex].Data();
        const float* pSrcImageBufferFloat = reinterpret_cast<float*>(imgBuffers[frontBufferIndex].Data());
        switch (dstFormat)
        {
            case eTextureFormat::R8_UNORM:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    uint8_t* pDstTextureBuffer = pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset;
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBuffer + hi * texWidth + wi)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, linear format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::R8G8_UNORM:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    uint8_t* pDstTextureBuffer = pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset;
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBuffer + (hi * texWidth + wi) * 2)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 2)[1] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, linear format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::R8G8B8A8_UNORM:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    uint8_t* pDstTextureBuffer = pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset;
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[1] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[2] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[3] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, linear format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::R8_SRGB:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    !isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    uint8_t* pDstTextureBuffer = pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset;
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBuffer + hi * texWidth + wi)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, srgb format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::R8G8_SRGB:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    !isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    uint8_t* pDstTextureBuffer = pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset;
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBuffer + (hi * texWidth + wi) * 2)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 2)[1] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, srgb format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::R8G8B8A8_SRGB:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    !isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    uint8_t* pDstTextureBuffer = pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset;
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[1] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[2] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[3] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, srgb format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::R16_FLOAT:
                if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    half_float::half* pDstTextureBuffer = reinterpret_cast<half_float::half*>(
                        pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset);
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBuffer + hi * texWidth + wi)[0] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be float, linear format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::R16G16_FLOAT:
                if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    half_float::half* pDstTextureBuffer = reinterpret_cast<half_float::half*>(
                        pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset);
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBuffer + (hi * texWidth + wi) * 2)[0] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 2)[1] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be float, linear format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::R16G16B16A16_FLOAT:
                if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    half_float::half* pDstTextureBuffer = reinterpret_cast<half_float::half*>(
                        pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset);
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[0] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[1] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[2] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                            (pDstTextureBuffer + (hi * texWidth + wi) * 4)[3] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be float, linear format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::R32_FLOAT:
                if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    float* pDstTextureBuffer =
                        reinterpret_cast<float*>(pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset);
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBuffer + hi * texWidth + wi)[0] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be float, linear format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::BC1_UNORM:
            case eTextureFormat::BC3_UNORM:
            case eTextureFormat::BC4_UNORM:
            case eTextureFormat::BC4_SNORM:
            case eTextureFormat::BC5_UNORM:
            case eTextureFormat::BC5_SNORM:
            case eTextureFormat::BC7_UNORM:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    utils::image_u8 rdoSrcImg;
                    rdoSrcImg.init(resizedImgWidth, resizedImgHeight);

                    uint8_t* pRdoSrcImgPixels = reinterpret_cast<uint8_t*>(rdoSrcImg.get_pixels().data());
                    for (int32_t hi = 0; hi < resizedImgHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < resizedImgWidth; ++wi)
                        {
                            (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[0] =
                                (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[0]; // copy r channel.
                            (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[1] =
                                (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[1]; // copy g channel.
                            (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[2] =
                                (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[2]; // copy b channel.
                            (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[3] =
                                (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[3]; // copy a channel.
                        }
                    }

                    rdo_bc::rdo_bc_params params;
                    params.clear();
                    params.m_perceptual = isLinearTextureFormat(dstFormat) ? false : true;
                    params.m_use_bc1_3color_mode = (srcTextureIR.Img.GetFormat() == Image::eFormat::R8G8B8A8_UNORM ||
                                                    srcTextureIR.Img.GetFormat() == Image::eFormat::R8G8B8A8_SRGB);
                    params.m_use_bc1_3color_mode_for_black = false;
                    params.m_bc1_quality_level = 10;
                    params.m_dxgi_format = toDxgiFormat_rdo_bc(dstFormat);
                    params.m_rdo_lambda = 2.0f;
                    params.m_rdo_try_2_matches = false;
                    params.m_rdo_ultrasmooth_block_handling = false;
                    params.m_use_hq_bc345 = false;
                    params.m_bc345_search_rad = 5;
                    params.m_bc7enc_reduce_entropy = true;
                    params.m_rdo_max_threads = std::thread::hardware_concurrency();

                    rdo_bc::rdo_bc_encoder encoder;
                    if (!encoder.init(rdoSrcImg, params))
                    {
                        HO_ASSERT(false, "Failed to initialize rdo_bc encoder.");
                        return nullptr;
                    }

                    if (!encoder.encode())
                    {
                        HO_ASSERT(false, "Failed to encode.");
                        return nullptr;
                    }

                    const uint8_t* pPackedBlocks = reinterpret_cast<const uint8_t*>(encoder.get_blocks());
                    uint8_t* pDstTextureBuffer = pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset;
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            for (int32_t bi = 0; bi < texPixelBytes; ++bi)
                            {
                                (pDstTextureBuffer + (hi * texWidth + wi) * texPixelBytes)[bi] =
                                    (pPackedBlocks + (hi * texWidth + wi) * texPixelBytes)[bi];
                            }
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, linear format. Please check source image format.");
                    return nullptr;
                }
                break;
            case eTextureFormat::BC1_SRGB:
            case eTextureFormat::BC3_SRGB:
            case eTextureFormat::BC7_SRGB:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    !isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    utils::image_u8 rdoSrcImg;
                    rdoSrcImg.init(resizedImgWidth, resizedImgHeight);

                    uint8_t* pRdoSrcImgPixels = reinterpret_cast<uint8_t*>(rdoSrcImg.get_pixels().data());
                    for (int32_t hi = 0; hi < resizedImgHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < resizedImgWidth; ++wi)
                        {
                            (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[0] =
                                (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[0]; // copy r channel.
                            (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[1] =
                                (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[1]; // copy g channel.
                            (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[2] =
                                (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[2]; // copy b channel.
                            (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[3] =
                                (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[3]; // copy a channel.
                        }
                    }

                    rdo_bc::rdo_bc_params params;
                    params.clear();
                    params.m_perceptual = isLinearTextureFormat(dstFormat) ? false : true;
                    params.m_use_bc1_3color_mode = (srcTextureIR.Img.GetFormat() == Image::eFormat::R8G8B8A8_UNORM ||
                                                    srcTextureIR.Img.GetFormat() == Image::eFormat::R8G8B8A8_SRGB);
                    params.m_use_bc1_3color_mode_for_black = false;
                    params.m_bc1_quality_level = 10;
                    params.m_dxgi_format = toDxgiFormat_rdo_bc(dstFormat);
                    params.m_rdo_lambda = 2.0f;
                    params.m_rdo_try_2_matches = false;
                    params.m_rdo_ultrasmooth_block_handling = false;
                    params.m_use_hq_bc345 = false;
                    params.m_bc345_search_rad = 5;
                    params.m_bc7enc_reduce_entropy = true;
                    params.m_rdo_max_threads = std::thread::hardware_concurrency();

                    rdo_bc::rdo_bc_encoder encoder;
                    if (!encoder.init(rdoSrcImg, params))
                    {
                        HO_ASSERT(false, "Failed to initialize rdo_bc encoder.");
                        return nullptr;
                    }

                    if (!encoder.encode())
                    {
                        HO_ASSERT(false, "Failed to encode.");
                        return nullptr;
                    }

                    const uint8_t* pPackedBlocks = reinterpret_cast<const uint8_t*>(encoder.get_blocks());
                    uint8_t* pDstTextureBuffer = pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i].Offset;
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            for (int32_t bi = 0; bi < texPixelBytes; ++bi)
                            {
                                (pDstTextureBuffer + (hi * texWidth + wi) * texPixelBytes)[bi] =
                                    (pPackedBlocks + (hi * texWidth + wi) * texPixelBytes)[bi];
                            }
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, srgb format. Please check source image format.");
                    return nullptr;
                }
                break;

            case eTextureFormat::BC6H_UF16:
            case eTextureFormat::BC6H_SF16:
                // if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                //     isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    HO_ASSERT(false, "Currently not supports BC6 format.");
                    return nullptr;
                }
                // else
                // {
                //     HO_ASSERT(false, "Format mismatch.");
                //     return nullptr;
                // }
                break;
            default:
                HO_ASSERT(false, "Invalid texture format.");
                return nullptr;
        }

        frontBufferIndex = (frontBufferIndex + 1) % 2;
        backBufferIndex = (backBufferIndex + 1) % 2;
        imgBuffers[frontBufferIndex].Fill(0);
        originImgWidth = resizedImgWidth;
        originImgHeight = resizedImgHeight;
        resizedImgWidth = math::Max(1, static_cast<int32_t>(originImgWidth / 2));
        resizedImgHeight = math::Max(1, static_cast<int32_t>(originImgHeight / 2));
    }
    return pNewTexture;
}

// Assumes srcTextureIR always uses a 4-channel memory layout (RGBA),
// regardless of the actual color channels or format of the source image.
// TODO: Refactor later. Doing way too much and BC compression logic is heavily duplicated.
std::unique_ptr<TextureAsset> importTextureCubeMap(const std::string& nameStr,
                                                   const parser::TextureIR& srcPosX,
                                                   const parser::TextureIR& srcNegX,
                                                   const parser::TextureIR& srcPosY,
                                                   const parser::TextureIR& srcNegY,
                                                   const parser::TextureIR& srcPosZ,
                                                   const parser::TextureIR& srcNegZ,
                                                   eTextureFormat dstFormat,
                                                   bool bGenerateMipmap)
{
    if (srcPosX.Img.GetBitmap() == nullptr || srcNegX.Img.GetBitmap() == nullptr ||
        srcPosY.Img.GetBitmap() == nullptr || srcNegY.Img.GetBitmap() == nullptr ||
        srcPosZ.Img.GetBitmap() == nullptr || srcNegZ.Img.GetBitmap() == nullptr)
    {
        HO_ASSERT(false, "Source images were failed to parse.");
        return nullptr;
    }

    const int32_t baseWidth = srcPosX.Img.GetWidth();
    const int32_t baseHeight = srcPosX.Img.GetHeight();
    const Image::eFormat baseFormat = srcPosX.Img.GetFormat();
    const int32_t baseLogicalChannels = srcPosX.Img.GetLogicalChannelCount();
    const int32_t basePhysicalChannels = srcPosX.Img.GetPhysicalChannelCount();

    if (baseWidth != baseHeight)
    {
        HO_ASSERT(false, "Cubemap faces must be square (width == height).");
        return nullptr;
    }

    if (srcNegX.Img.GetWidth() != baseWidth || srcNegX.Img.GetHeight() != baseHeight ||
        srcPosY.Img.GetWidth() != baseWidth || srcPosY.Img.GetHeight() != baseHeight ||
        srcNegY.Img.GetWidth() != baseWidth || srcNegY.Img.GetHeight() != baseHeight ||
        srcPosZ.Img.GetWidth() != baseWidth || srcPosZ.Img.GetHeight() != baseHeight ||
        srcNegZ.Img.GetWidth() != baseWidth || srcNegZ.Img.GetHeight() != baseHeight)
    {
        HO_ASSERT(false, "All 6 faces of a cubemap must have the exact same dimensions.");
        return nullptr;
    }

    if (srcNegX.Img.GetFormat() != baseFormat || srcPosY.Img.GetFormat() != baseFormat ||
        srcNegY.Img.GetFormat() != baseFormat || srcPosZ.Img.GetFormat() != baseFormat ||
        srcNegZ.Img.GetFormat() != baseFormat || srcNegX.Img.GetLogicalChannelCount() != baseLogicalChannels ||
        srcPosY.Img.GetLogicalChannelCount() != baseLogicalChannels ||
        srcNegY.Img.GetLogicalChannelCount() != baseLogicalChannels ||
        srcPosZ.Img.GetLogicalChannelCount() != baseLogicalChannels ||
        srcNegZ.Img.GetLogicalChannelCount() != baseLogicalChannels ||
        srcNegX.Img.GetPhysicalChannelCount() != basePhysicalChannels ||
        srcPosY.Img.GetPhysicalChannelCount() != basePhysicalChannels ||
        srcNegY.Img.GetPhysicalChannelCount() != basePhysicalChannels ||
        srcPosZ.Img.GetPhysicalChannelCount() != basePhysicalChannels ||
        srcNegZ.Img.GetPhysicalChannelCount() != basePhysicalChannels)
    {
        HO_ASSERT(false, "All 6 faces of a cubemap must have the exact same format and channel counts.");
        return nullptr;
    }

    std::unique_ptr<TextureAsset> pNewTexture = std::make_unique<TextureAsset>();
    const std::string texNameStr = srcPosX.ResourcePath.GetParentPath().ToString() + "::Texture::CubeMap::" + nameStr;
    pNewTexture->hName = StringHandle(texNameStr);
    pNewTexture->Width = baseWidth;
    pNewTexture->Height = baseHeight;
    pNewTexture->Depth = 6;
    pNewTexture->ArraySize = 1;
    pNewTexture->MipLevels = bGenerateMipmap ? calculateMipLevels(pNewTexture->Width, pNewTexture->Height) : 1;
    pNewTexture->Format = dstFormat;
    pNewTexture->Type = eTextureType::TextureCubeMap;

    // Construct sub texture layouts
    int32_t totalDataSize = 0;
    int32_t imgWidth = pNewTexture->Width;
    int32_t imgHeight = pNewTexture->Height;
    pNewTexture->Layouts = FixedArray<TextureAsset::SubTextureLayout>(pNewTexture->MipLevels * 6);
    for (int32_t i = 0; i < pNewTexture->MipLevels; ++i)
    {
        for (int32_t face = 0; face < 6; ++face)
        {
            pNewTexture->Layouts[i * 6 + face].Offset = totalDataSize;
            if (isUncompressedTextureFormat(dstFormat))
            {
                pNewTexture->Layouts[i * 6 + face].RowPitch = imgWidth * getTexturePixelBytes(dstFormat);
                pNewTexture->Layouts[i * 6 + face].Size = imgHeight * pNewTexture->Layouts[i * 6 + face].RowPitch;
            }
            else
            {
                pNewTexture->Layouts[i * 6 + face].RowPitch =
                    getTextureBlockCount(imgWidth) * getTextureBlockBytes(dstFormat);
                pNewTexture->Layouts[i * 6 + face].Size =
                    getTextureBlockCount(imgHeight) * pNewTexture->Layouts[i * 6 + face].RowPitch;
            }

            totalDataSize += pNewTexture->Layouts[i * 6 + face].Size;
        }
        imgWidth = math::Max(1, static_cast<int32_t>(imgWidth / 2));
        imgHeight = math::Max(1, static_cast<int32_t>(imgHeight / 2));
    }
    pNewTexture->DataBlob = FixedArray<uint8_t>(totalDataSize);

    // Generate compressed mipmaps
    const parser::TextureIR* const pSrcTexIRs[6] = {&srcPosX, &srcNegX, &srcPosY, &srcNegY, &srcPosZ, &srcNegZ};
    int32_t originImgWidth = pNewTexture->Width;
    int32_t originImgHeight = pNewTexture->Height;
    int32_t resizedImgWidth = originImgWidth;
    int32_t resizedImgHeight = originImgHeight;
    const int32_t imgPixelBytes = isIntegerImageFormat(baseFormat) ? 4 : 16;

    const int32_t texPixelBytes =
        isUncompressedTextureFormat(dstFormat) ? getTexturePixelBytes(dstFormat) : getTextureBlockBytes(dstFormat);
    if (texPixelBytes == 0)
    {
        HO_ASSERT(false, "Invalid texture pixel bytes (0).");
        return nullptr;
    }
    FixedArray<uint8_t> imgBuffers[2][6] = {};
    for (int32_t i = 0; i < 2; ++i)
    {
        for (int32_t j = 0; j < 6; ++j)
        {
            imgBuffers[i][j] = FixedArray<uint8_t>(pNewTexture->Width * pNewTexture->Height * imgPixelBytes);
        }
    }
    int32_t frontBufferIndex = 0;
    int32_t backBufferIndex = 1;

    // original image is in back buffer initially.
    for (int32_t face = 0; face < 6; ++face)
    {
        for (int32_t i = 0; i < originImgWidth * originImgHeight * imgPixelBytes; ++i)
        {
            imgBuffers[backBufferIndex][face][i] = pSrcTexIRs[face]->Img.GetBitmap()[i];
        }
    }

    for (int32_t i = 0; i < pNewTexture->MipLevels; ++i)
    {
        for (int32_t face = 0; face < 6; ++face)
        {
            const int32_t texWidth = pNewTexture->Layouts[i * 6 + face].RowPitch / texPixelBytes;
            const int32_t texHeight =
                pNewTexture->Layouts[i * 6 + face].Size / pNewTexture->Layouts[i * 6 + face].RowPitch;
            // Resize original image to half (from back buffer to front buffer)
            const void* pResizeSuccess = nullptr;
            switch (baseFormat)
            {
                case Image::eFormat::R8_UNORM:
                case Image::eFormat::R8G8_UNORM:
                case Image::eFormat::R8G8B8_UNORM:
                case Image::eFormat::R8G8B8A8_UNORM:
                    pResizeSuccess = stbir_resize_uint8_linear(imgBuffers[backBufferIndex][face].Data(),
                                                               originImgWidth,
                                                               originImgHeight,
                                                               originImgWidth * imgPixelBytes,
                                                               imgBuffers[frontBufferIndex][face].Data(),
                                                               resizedImgWidth,
                                                               resizedImgHeight,
                                                               resizedImgWidth * imgPixelBytes,
                                                               toStbiFormat(baseFormat));
                    break;
                case Image::eFormat::R8_SRGB:
                case Image::eFormat::R8G8_SRGB:
                case Image::eFormat::R8G8B8_SRGB:
                case Image::eFormat::R8G8B8A8_SRGB:
                    pResizeSuccess = stbir_resize_uint8_srgb(imgBuffers[backBufferIndex][face].Data(),
                                                             originImgWidth,
                                                             originImgHeight,
                                                             originImgWidth * imgPixelBytes,
                                                             imgBuffers[frontBufferIndex][face].Data(),
                                                             resizedImgWidth,
                                                             resizedImgHeight,
                                                             resizedImgWidth * imgPixelBytes,
                                                             toStbiFormat(baseFormat));
                    break;
                case Image::eFormat::R32_FLOAT:
                case Image::eFormat::R32G32_FLOAT:
                case Image::eFormat::R32G32B32_FLOAT:
                case Image::eFormat::R32G32B32A32_FLOAT:
                    pResizeSuccess = stbir_resize_float_linear(
                        reinterpret_cast<const float*>(imgBuffers[backBufferIndex][face].Data()),
                        originImgWidth,
                        originImgHeight,
                        originImgWidth * imgPixelBytes,
                        reinterpret_cast<float*>(imgBuffers[frontBufferIndex][face].Data()),
                        resizedImgWidth,
                        resizedImgHeight,
                        resizedImgWidth * imgPixelBytes,
                        toStbiFormat(baseFormat));
                    break;
                default:
                    HO_ASSERT(false, "Invalid image format.");
                    return nullptr;
            }

            if (!pResizeSuccess)
            {
                HO_ASSERT(false, "Failed to resize image.");
                return nullptr;
            }

            // Compress current mipmap (from front buffer to dataBlob buffer)
            const uint8_t* pSrcImageBufferUint8 = imgBuffers[frontBufferIndex][face].Data();
            const float* pSrcImageBufferFloat = reinterpret_cast<float*>(imgBuffers[frontBufferIndex][face].Data());
            switch (dstFormat)
            {
                case eTextureFormat::R8_UNORM:
                    if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        uint8_t* pDstTextureBuffer =
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset;
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBuffer + hi * texWidth + wi)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, linear format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::R8G8_UNORM:
                    if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        uint8_t* pDstTextureBuffer =
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset;
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBuffer + (hi * texWidth + wi) * 2)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 2)[1] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, linear format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::R8G8B8A8_UNORM:
                    if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        uint8_t* pDstTextureBuffer =
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset;
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[1] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[2] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[3] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, linear format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::R8_SRGB:
                    if (isIntegerImageFormat(baseFormat) && !isLinearImageFormat(baseFormat))
                    {
                        uint8_t* pDstTextureBuffer =
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset;
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBuffer + hi * texWidth + wi)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, srgb format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::R8G8_SRGB:
                    if (isIntegerImageFormat(baseFormat) && !isLinearImageFormat(baseFormat))
                    {
                        uint8_t* pDstTextureBuffer =
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset;
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBuffer + (hi * texWidth + wi) * 2)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 2)[1] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, srgb format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::R8G8B8A8_SRGB:
                    if (isIntegerImageFormat(baseFormat) && !isLinearImageFormat(baseFormat))
                    {
                        uint8_t* pDstTextureBuffer =
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset;
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[1] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[2] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[3] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, srgb format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::R16_FLOAT:
                    if (!isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        half_float::half* pDstTextureBuffer = reinterpret_cast<half_float::half*>(
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset);
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBuffer + hi * texWidth + wi)[0] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be float, linear format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::R16G16_FLOAT:
                    if (!isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        half_float::half* pDstTextureBuffer = reinterpret_cast<half_float::half*>(
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset);
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBuffer + (hi * texWidth + wi) * 2)[0] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 2)[1] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be float, linear format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::R16G16B16A16_FLOAT:
                    if (!isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        half_float::half* pDstTextureBuffer = reinterpret_cast<half_float::half*>(
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset);
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[0] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[1] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[2] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                                (pDstTextureBuffer + (hi * texWidth + wi) * 4)[3] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be float, linear format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::R32_FLOAT:
                    if (!isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        float* pDstTextureBuffer = reinterpret_cast<float*>(pNewTexture->DataBlob.Data() +
                                                                            pNewTexture->Layouts[i * 6 + face].Offset);
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBuffer + hi * texWidth + wi)[0] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be float, linear format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::BC1_UNORM:
                case eTextureFormat::BC3_UNORM:
                case eTextureFormat::BC4_UNORM:
                case eTextureFormat::BC4_SNORM:
                case eTextureFormat::BC5_UNORM:
                case eTextureFormat::BC5_SNORM:
                case eTextureFormat::BC7_UNORM:
                    if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        utils::image_u8 rdoSrcImg;
                        rdoSrcImg.init(resizedImgWidth, resizedImgHeight);

                        uint8_t* pRdoSrcImgPixels = reinterpret_cast<uint8_t*>(rdoSrcImg.get_pixels().data());
                        for (int32_t hi = 0; hi < resizedImgHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < resizedImgWidth; ++wi)
                            {
                                (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[0] =
                                    (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[0]; // copy r channel.
                                (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[1] =
                                    (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[1]; // copy g channel.
                                (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[2] =
                                    (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[2]; // copy b channel.
                                (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[3] =
                                    (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[3]; // copy a channel.
                            }
                        }

                        rdo_bc::rdo_bc_params params;
                        params.clear();
                        params.m_perceptual = isLinearTextureFormat(dstFormat) ? false : true;
                        params.m_use_bc1_3color_mode = (baseFormat == Image::eFormat::R8G8B8A8_UNORM ||
                                                        baseFormat == Image::eFormat::R8G8B8A8_SRGB);
                        params.m_use_bc1_3color_mode_for_black = false;
                        params.m_bc1_quality_level = 10;
                        params.m_dxgi_format = toDxgiFormat_rdo_bc(dstFormat);
                        params.m_rdo_lambda = 2.0f;
                        params.m_rdo_try_2_matches = false;
                        params.m_rdo_ultrasmooth_block_handling = false;
                        params.m_use_hq_bc345 = false;
                        params.m_bc345_search_rad = 5;
                        params.m_bc7enc_reduce_entropy = true;
                        params.m_rdo_max_threads = std::thread::hardware_concurrency();

                        rdo_bc::rdo_bc_encoder encoder;
                        if (!encoder.init(rdoSrcImg, params))
                        {
                            HO_ASSERT(false, "Failed to initialize rdo_bc encoder.");
                            return nullptr;
                        }

                        if (!encoder.encode())
                        {
                            HO_ASSERT(false, "Failed to encode.");
                            return nullptr;
                        }

                        const uint8_t* pPackedBlocks = reinterpret_cast<const uint8_t*>(encoder.get_blocks());
                        uint8_t* pDstTextureBuffer =
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset;
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                for (int32_t bi = 0; bi < texPixelBytes; ++bi)
                                {
                                    (pDstTextureBuffer + (hi * texWidth + wi) * texPixelBytes)[bi] =
                                        (pPackedBlocks + (hi * texWidth + wi) * texPixelBytes)[bi];
                                }
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, linear format. Please check source image format.");
                        return nullptr;
                    }
                    break;
                case eTextureFormat::BC1_SRGB:
                case eTextureFormat::BC3_SRGB:
                case eTextureFormat::BC7_SRGB:
                    if (isIntegerImageFormat(baseFormat) && !isLinearImageFormat(baseFormat))
                    {
                        utils::image_u8 rdoSrcImg;
                        rdoSrcImg.init(resizedImgWidth, resizedImgHeight);

                        uint8_t* pRdoSrcImgPixels = reinterpret_cast<uint8_t*>(rdoSrcImg.get_pixels().data());
                        for (int32_t hi = 0; hi < resizedImgHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < resizedImgWidth; ++wi)
                            {
                                (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[0] =
                                    (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[0]; // copy r channel.
                                (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[1] =
                                    (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[1]; // copy g channel.
                                (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[2] =
                                    (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[2]; // copy b channel.
                                (pRdoSrcImgPixels + (hi * resizedImgWidth + wi) * 4)[3] =
                                    (pSrcImageBufferUint8 + (hi * resizedImgWidth + wi) * 4)[3]; // copy a channel.
                            }
                        }

                        rdo_bc::rdo_bc_params params;
                        params.clear();
                        params.m_perceptual = isLinearTextureFormat(dstFormat) ? false : true;
                        params.m_use_bc1_3color_mode = (baseFormat == Image::eFormat::R8G8B8A8_UNORM ||
                                                        baseFormat == Image::eFormat::R8G8B8A8_SRGB);
                        params.m_use_bc1_3color_mode_for_black = false;
                        params.m_bc1_quality_level = 10;
                        params.m_dxgi_format = toDxgiFormat_rdo_bc(dstFormat);
                        params.m_rdo_lambda = 2.0f;
                        params.m_rdo_try_2_matches = false;
                        params.m_rdo_ultrasmooth_block_handling = false;
                        params.m_use_hq_bc345 = false;
                        params.m_bc345_search_rad = 5;
                        params.m_bc7enc_reduce_entropy = true;
                        params.m_rdo_max_threads = std::thread::hardware_concurrency();

                        rdo_bc::rdo_bc_encoder encoder;
                        if (!encoder.init(rdoSrcImg, params))
                        {
                            HO_ASSERT(false, "Failed to initialize rdo_bc encoder.");
                            return nullptr;
                        }

                        if (!encoder.encode())
                        {
                            HO_ASSERT(false, "Failed to encode.");
                            return nullptr;
                        }

                        const uint8_t* pPackedBlocks = reinterpret_cast<const uint8_t*>(encoder.get_blocks());
                        uint8_t* pDstTextureBuffer =
                            pNewTexture->DataBlob.Data() + pNewTexture->Layouts[i * 6 + face].Offset;
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                for (int32_t bi = 0; bi < texPixelBytes; ++bi)
                                {
                                    (pDstTextureBuffer + (hi * texWidth + wi) * texPixelBytes)[bi] =
                                        (pPackedBlocks + (hi * texWidth + wi) * texPixelBytes)[bi];
                                }
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, srgb format. Please check source image format.");
                        return nullptr;
                    }
                    break;

                case eTextureFormat::BC6H_UF16:
                case eTextureFormat::BC6H_SF16:
                    // if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    //     isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                    {
                        HO_ASSERT(false, "Currently not supports BC6 format.");
                        return nullptr;
                    }
                    // else
                    // {
                    //     HO_ASSERT(false, "Format mismatch.");
                    //     return nullptr;
                    // }
                    break;
                default:
                    HO_ASSERT(false, "Invalid texture format.");
                    return nullptr;
            }
        }

        frontBufferIndex = (frontBufferIndex + 1) % 2;
        backBufferIndex = (backBufferIndex + 1) % 2;
        for (int32_t face = 0; face < 6; ++face)
        {
            imgBuffers[frontBufferIndex][face].Fill(0);
        }
        originImgWidth = resizedImgWidth;
        originImgHeight = resizedImgHeight;
        resizedImgWidth = math::Max(1, static_cast<int32_t>(originImgWidth / 2));
        resizedImgHeight = math::Max(1, static_cast<int32_t>(originImgHeight / 2));
    }
    return pNewTexture;
}

std::unique_ptr<ShaderAsset> importShader(const parser::ShaderIR& srcShaderIR,
                                          eShaderStage stage,
                                          uint8_t compatibleMaterialTypes)
{
    std::unique_ptr<ShaderAsset> pNewShader = std::make_unique<ShaderAsset>();
    const std::string name =
        srcShaderIR.ResourcePath.RemovedExtension().ToString() + "::Shader::" + srcShaderIR.NameStr;
    pNewShader->hName = StringHandle(name);
    pNewShader->SourceStr = srcShaderIR.SourceStr;

    const shaderc::Compiler compiler;
    bool bOptimize = true; // NOLINT
#if defined(DEBUG) | defined(_DEBUG)
    bOptimize = false;
#endif
    std::vector<uint32_t> binary =
        compileFileGLSL(compiler, srcShaderIR.NameStr, toShadercStageGLSL(stage), srcShaderIR.SourceStr, bOptimize);
    pNewShader->Binary = FixedArray<uint32_t>(static_cast<int32_t>(binary.size()));
    for (int32_t i = 0; i < static_cast<int32_t>(binary.size()); ++i)
    {
        pNewShader->Binary[i] = binary[i];
    }

    pNewShader->Stage = stage;
    pNewShader->CompatibleMaterialTypes = compatibleMaterialTypes;
    return pNewShader;
}

// ===========================================================================
//  Private Function Definitions
// ===========================================================================

eMeshPrimitiveType toAssetPrimitiveType(parser::MeshIR::ePrimitiveType type)
{
    switch (type)
    {
        case parser::MeshIR::ePrimitiveType::Point:
            return eMeshPrimitiveType::Point;
        case parser::MeshIR::ePrimitiveType::Line:
            return eMeshPrimitiveType::Line;
        case parser::MeshIR::ePrimitiveType::LineStrip:
            return eMeshPrimitiveType::LineStrip;
        case parser::MeshIR::ePrimitiveType::Triangle:
            return eMeshPrimitiveType::Triangle;
        case parser::MeshIR::ePrimitiveType::TriangleStrip:
            return eMeshPrimitiveType::TriangleStrip;
        default:
            HO_ASSERT(false, "Invalid primitive type.");
            return eMeshPrimitiveType::None;
    }
}

eAnimExtrapolationMode toAssetExtrapolationMode(parser::AnimationIR::eExtrapolationMode mode)
{
    switch (mode)
    {
        case parser::AnimationIR::eExtrapolationMode::Default:
            return eAnimExtrapolationMode::Default;
        case parser::AnimationIR::eExtrapolationMode::Constant:
            return eAnimExtrapolationMode::Constant;
        case parser::AnimationIR::eExtrapolationMode::Linear:
            return eAnimExtrapolationMode::Linear;
        case parser::AnimationIR::eExtrapolationMode::Repeat:
            return eAnimExtrapolationMode::Repeat;
        default:
            HO_ASSERT(false, "Invalid extrapolation mode.");
            return eAnimExtrapolationMode::None;
    }
}

eAnimInterpolationMode toAssetInterpolationMode(parser::AnimationIR::eInterpolationMode mode)
{
    switch (mode)
    {
        case parser::AnimationIR::eInterpolationMode::Step:
            return eAnimInterpolationMode::Step;
        case parser::AnimationIR::eInterpolationMode::Linear:
            return eAnimInterpolationMode::Linear;
        case parser::AnimationIR::eInterpolationMode::SphericalLinear:
            return eAnimInterpolationMode::SphericalLinear;
        case parser::AnimationIR::eInterpolationMode::CubicSpline:
            return eAnimInterpolationMode::CubicSpline;
        default:
            HO_ASSERT(false, "Invalid interpolation mode.");
            eAnimInterpolationMode::None;
    }
}

eMaterialAlphaMode toAssetAlphaMode(parser::MaterialIR::eAlphaMode mode)
{
    switch (mode)
    {
        case parser::MaterialIR::eAlphaMode::Opaque:
            return eMaterialAlphaMode::Opaque;
        case parser::MaterialIR::eAlphaMode::Blend:
            return eMaterialAlphaMode::Blend;
        case parser::MaterialIR::eAlphaMode::Mask:
            return eMaterialAlphaMode::Mask;
        default:
            HO_ASSERT(false, "Invalid alpha mode.");
            eMaterialAlphaMode::None;
    }
    return eMaterialAlphaMode::None;
}

eMaterialAlphaBlendMode toAssetAlphaBlendMode(parser::MaterialIR::eAlphaBlendMode mode)
{
    switch (mode)
    {
        case parser::MaterialIR::eAlphaBlendMode::Default:
            return eMaterialAlphaBlendMode::Default;
        case parser::MaterialIR::eAlphaBlendMode::Additive:
            return eMaterialAlphaBlendMode::Additive;
        default:
            HO_ASSERT(false, "Invalid blend mode.");
            return eMaterialAlphaBlendMode::None;
    }
}

eMaterialTextureUsage toAssetTextureUsage(parser::MaterialIR::eTextureUsage usage)
{
    switch (usage)
    {
        case parser::MaterialIR::eTextureUsage::Diffuse:
            return eMaterialTextureUsage::Diffuse;
        case parser::MaterialIR::eTextureUsage::Specular:
            return eMaterialTextureUsage::Specular;
        case parser::MaterialIR::eTextureUsage::Opacity:
            return eMaterialTextureUsage::Opacity;
        case parser::MaterialIR::eTextureUsage::Normal:
            return eMaterialTextureUsage::Normal;
        case parser::MaterialIR::eTextureUsage::Albedo:
            return eMaterialTextureUsage::Albedo;
        case parser::MaterialIR::eTextureUsage::MetallicRoughness:
            return eMaterialTextureUsage::MetallicRoughness;
        case parser::MaterialIR::eTextureUsage::Emissive:
            return eMaterialTextureUsage::Emissive;
        case parser::MaterialIR::eTextureUsage::AmbientOcclusion:
            return eMaterialTextureUsage::AmbientOcclusion;
        default:
            HO_ASSERT(false, "Invalid texture usage.");
            return eMaterialTextureUsage::None;
    }
}

parser::MaterialIR::eTextureUsage toIRTextureUsage(eMaterialTextureUsage usage)
{
    switch (usage)
    {
        case eMaterialTextureUsage::Diffuse:
            return parser::MaterialIR::eTextureUsage::Diffuse;
        case eMaterialTextureUsage::Specular:
            return parser::MaterialIR::eTextureUsage::Specular;
        case eMaterialTextureUsage::Opacity:
            return parser::MaterialIR::eTextureUsage::Opacity;
        case eMaterialTextureUsage::Normal:
            return parser::MaterialIR::eTextureUsage::Normal;
        case eMaterialTextureUsage::Albedo:
            return parser::MaterialIR::eTextureUsage::Albedo;
        case eMaterialTextureUsage::MetallicRoughness:
            return parser::MaterialIR::eTextureUsage::MetallicRoughness;
        case eMaterialTextureUsage::Emissive:
            return parser::MaterialIR::eTextureUsage::Emissive;
        case eMaterialTextureUsage::AmbientOcclusion:
            return parser::MaterialIR::eTextureUsage::AmbientOcclusion;
        default:
            HO_ASSERT(false, "Invalid texture usage.");
            return parser::MaterialIR::eTextureUsage::None;
    }
}

stbir_pixel_layout toStbiFormat(Image::eFormat format)
{
    switch (format)
    {
        case Image::eFormat::R8_UNORM:
        case Image::eFormat::R8G8_UNORM:
        case Image::eFormat::R8G8B8_UNORM:
        case Image::eFormat::R32_FLOAT:
        case Image::eFormat::R32G32_FLOAT:
        case Image::eFormat::R32G32B32_FLOAT:
        case Image::eFormat::R8_SRGB:
        case Image::eFormat::R8G8_SRGB:
        case Image::eFormat::R8G8B8_SRGB:
            return stbir_pixel_layout::STBIR_4CHANNEL;

        case Image::eFormat::R32G32B32A32_FLOAT:
        case Image::eFormat::R8G8B8A8_UNORM:
        case Image::eFormat::R8G8B8A8_SRGB:
            return stbir_pixel_layout::STBIR_RGBA;

        default:
            HO_ASSERT(false, "Invalid image format.");
            return stbir_pixel_layout::STBIR_4CHANNEL;
    }
}

bool isLinearImageFormat(Image::eFormat format)
{
    switch (format)
    {
        case Image::eFormat::R8_UNORM:
        case Image::eFormat::R8G8_UNORM:
        case Image::eFormat::R8G8B8_UNORM:
        case Image::eFormat::R8G8B8A8_UNORM:
        case Image::eFormat::R32_FLOAT:
        case Image::eFormat::R32G32_FLOAT:
        case Image::eFormat::R32G32B32_FLOAT:
        case Image::eFormat::R32G32B32A32_FLOAT:
            return true;
        case Image::eFormat::R8_SRGB:
        case Image::eFormat::R8G8_SRGB:
        case Image::eFormat::R8G8B8_SRGB:
        case Image::eFormat::R8G8B8A8_SRGB:
            return false;
        default:
            HO_ASSERT(false, "Invalid image format.");
            return false;
    }
}

bool isIntegerImageFormat(Image::eFormat format)
{
    switch (format)
    {
        case Image::eFormat::R8_UNORM:
        case Image::eFormat::R8G8_UNORM:
        case Image::eFormat::R8G8B8_UNORM:
        case Image::eFormat::R8G8B8A8_UNORM:
        case Image::eFormat::R8_SRGB:
        case Image::eFormat::R8G8_SRGB:
        case Image::eFormat::R8G8B8_SRGB:
        case Image::eFormat::R8G8B8A8_SRGB:
            return true;
        case Image::eFormat::R32_FLOAT:
        case Image::eFormat::R32G32_FLOAT:
        case Image::eFormat::R32G32B32_FLOAT:
        case Image::eFormat::R32G32B32A32_FLOAT:
            return false;

        default:
            HO_ASSERT(false, "Invalid image format.");
            return false;
    }
}

bool isUncompressedTextureFormat(eTextureFormat format)
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

bool isLinearTextureFormat(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::R16_FLOAT:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R16G16B16A16_FLOAT:
        case eTextureFormat::R32_FLOAT:
        case eTextureFormat::BC1_UNORM:
        case eTextureFormat::BC3_UNORM:
        case eTextureFormat::BC4_UNORM:
        case eTextureFormat::BC4_SNORM:
        case eTextureFormat::BC5_UNORM:
        case eTextureFormat::BC5_SNORM:
        case eTextureFormat::BC6H_UF16:
        case eTextureFormat::BC6H_SF16:
        case eTextureFormat::BC7_UNORM:
            return true;

        case eTextureFormat::R8_SRGB:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R8G8B8A8_SRGB:
        case eTextureFormat::BC1_SRGB:
        case eTextureFormat::BC3_SRGB:
        case eTextureFormat::BC7_SRGB:
            return false;

        default:
            HO_ASSERT(false, "Invalid texture format.");
            return false;
    }
}

DXGI_FORMAT toDxgiFormat_rdo_bc(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::BC1_UNORM:
        case eTextureFormat::BC1_SRGB:
            return DXGI_FORMAT_BC1_UNORM;
        case eTextureFormat::BC3_UNORM:
        case eTextureFormat::BC3_SRGB:
            return DXGI_FORMAT_BC3_UNORM;
        case eTextureFormat::BC4_UNORM:
        case eTextureFormat::BC4_SNORM:
            return DXGI_FORMAT_BC4_UNORM;
        case eTextureFormat::BC5_UNORM:
        case eTextureFormat::BC5_SNORM:
            return DXGI_FORMAT_BC5_UNORM;
        case eTextureFormat::BC6H_UF16:
        case eTextureFormat::BC6H_SF16:
            return DXGI_FORMAT_BC6H_UF16;
        case eTextureFormat::BC7_UNORM:
        case eTextureFormat::BC7_SRGB:
            return DXGI_FORMAT_BC7_UNORM;
        default:
            HO_ASSERT(false, "Invalid texture format.");
            return DXGI_FORMAT_UNKNOWN;
    }
}

bool isIntegerTextureFormat(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8_SRGB:
        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::R8G8B8A8_SRGB:
        case eTextureFormat::BC1_UNORM:
        case eTextureFormat::BC1_SRGB:
        case eTextureFormat::BC3_UNORM:
        case eTextureFormat::BC3_SRGB:
        case eTextureFormat::BC4_UNORM:
        case eTextureFormat::BC4_SNORM:
        case eTextureFormat::BC5_UNORM:
        case eTextureFormat::BC5_SNORM:
        case eTextureFormat::BC7_UNORM:
        case eTextureFormat::BC7_SRGB:
            return true;

        case eTextureFormat::R16_FLOAT:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R16G16B16A16_FLOAT:
        case eTextureFormat::R32_FLOAT:
        case eTextureFormat::BC6H_UF16:
        case eTextureFormat::BC6H_SF16:
            return false;

        case eTextureFormat::None:
        default:
            HO_ASSERT(false, "Invalid or None texture format.");
            return false;
    }
}

int32_t getTexturePixelBytes(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::R8_UNORM:
        case eTextureFormat::R8_SRGB:
            return 1;
        case eTextureFormat::R8G8_UNORM:
        case eTextureFormat::R8G8_SRGB:
        case eTextureFormat::R16_FLOAT:
            return 2;
        case eTextureFormat::R8G8B8A8_UNORM:
        case eTextureFormat::R8G8B8A8_SRGB:
        case eTextureFormat::R16G16_FLOAT:
        case eTextureFormat::R32_FLOAT:
            return 4;
        case eTextureFormat::R16G16B16A16_FLOAT:
            return 8;
        default:
            HO_ASSERT(false, "It's not uncompressed format.");
            return 0;
    }
}

int32_t getTextureBlockCount(int32_t length)
{
    return math::Max(1, (length + 3) / 4);
}

int32_t getTextureBlockBytes(eTextureFormat format)
{
    switch (format)
    {
        case eTextureFormat::BC1_UNORM:
        case eTextureFormat::BC1_SRGB:
            return 8;
        case eTextureFormat::BC3_UNORM:
        case eTextureFormat::BC3_SRGB:
            return 16;
        case eTextureFormat::BC4_UNORM:
        case eTextureFormat::BC4_SNORM:
            return 8;
        case eTextureFormat::BC5_UNORM:
        case eTextureFormat::BC5_SNORM:
            return 16;
        case eTextureFormat::BC6H_UF16:
        case eTextureFormat::BC6H_SF16:
            HO_ASSERT(false, "Currently not supports BC6 format.");
            return 0;
        case eTextureFormat::BC7_UNORM:
        case eTextureFormat::BC7_SRGB:
            return 16;
        default:
            HO_ASSERT(false, "It's not compressed format.");
            return 0;
    }
}

int32_t calculateMipLevels(int32_t width, int32_t height)
{
    return static_cast<int32_t>(math::Floor(log2f(static_cast<float>(math::Max(width, height))))) + 1;
}

shaderc_shader_kind toShadercStageGLSL(eShaderStage stage)
{
    switch (stage)
    {
        case eShaderStage::VertexShader:
            return shaderc_shader_kind::shaderc_glsl_vertex_shader;
        case eShaderStage::FragmentShader:
            return shaderc_shader_kind::shaderc_glsl_fragment_shader;
        default:
            HO_ASSERT(false, "Invalid shader stage.");
            return shaderc_shader_kind::shaderc_glsl_infer_from_source;
    }
}

std::string preprocessShaderGLSL(const shaderc::Compiler& compiler,
                                 const std::string& sourceName,
                                 shaderc_shader_kind kind,
                                 const std::string& source)
{
    shaderc::CompileOptions options;

    // Like -DMY_DEFINE=1
    options.AddMacroDefinition("MY_DEFINE", "1");

    const shaderc::PreprocessedSourceCompilationResult result =
        compiler.PreprocessGlsl(source, kind, sourceName.c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        HO_LOG_ERROR((std::string("Shader Preprocessing Failed:\n") + result.GetErrorMessage()).c_str());
        return "";
    }

    return {result.cbegin(), result.cend()};
}

std::string compileFileToAssemblyGLSL(const shaderc::Compiler& compiler,
                                      const std::string& sourceName,
                                      shaderc_shader_kind kind,
                                      const std::string& source,
                                      bool optimize)
{
    shaderc::CompileOptions options;

    // Like -DMY_DEFINE=1
    options.AddMacroDefinition("MY_DEFINE", "1");
    if (optimize)
    {
        options.SetOptimizationLevel(shaderc_optimization_level_size);
    }

    const shaderc::AssemblyCompilationResult result =
        compiler.CompileGlslToSpvAssembly(source, kind, sourceName.c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        HO_LOG_ERROR((std::string("Shader Assembly Failed:\n") + result.GetErrorMessage()).c_str());
        return "";
    }

    return {result.cbegin(), result.cend()};
}

std::vector<uint32_t> compileFileGLSL(const shaderc::Compiler& compiler,
                                      const std::string& sourceName,
                                      shaderc_shader_kind kind,
                                      const std::string& source,
                                      bool optimize)
{
    shaderc::CompileOptions options;

    // Like -DMY_DEFINE=1
    options.AddMacroDefinition("MY_DEFINE", "1");
    if (optimize)
    {
        options.SetOptimizationLevel(shaderc_optimization_level_size);
    }
    options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

    const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, kind, sourceName.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        HO_LOG_ERROR((std::string("Shader Compilation Failed:\n") + module.GetErrorMessage()).c_str());
        HO_ASSERT(false, "Shader compilation failed.");
        return std::vector<uint32_t>();
    }

    return {module.cbegin(), module.cend()};
}

} // namespace importer
} // namespace ho