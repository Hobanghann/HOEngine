#include "AssetImportFuncs.h"

#include <half.hpp>
#include <ispc_texcomp.h>
#include <shaderc/shaderc.hpp>
#include <stb_image_resize2.h>

#include "Assets.h"
#include "Core/IO/ModelIR.h"
#include "Core/IO/ShaderIR.h"
#include "Core/Log/Logger.h"
#include "Core/Templates/FixedArray.h"
#include "TextureUtils.h"

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

static bool isLinearImageFormat(Image::eFormat format);
static bool isIntegerImageFormat(Image::eFormat format);

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
                                             std::string* pOutErrorMsg,
                                             bool optimize = false);

// ===========================================================================
//  Public Function Definitions
// ===========================================================================

bool importStaticMesh(ObjectHandle<RStaticMeshAsset> hOutAsset,
                      const parser::MeshIR& srcMeshIR,
                      const std::vector<std::unique_ptr<const parser::MaterialIR>>& pMaterialIRs,
                      const parser::SkeletonIR& srcSkeletonIR,
                      const parser::SkinIR& srcSkinIR)
{
    if (hOutAsset.IsNULL())
    {
        return false;
    }

    int32_t allPositionCount = 0;
    int32_t allNormalCount = 0;
    int32_t allTangentCount = 0;
    int32_t allUVCount = 0;
    int32_t allColorCount = 0;
    int32_t allIndexCount = 0;
    for (const auto& subMeshIR : srcMeshIR.SubMeshes)
    {
        const int32_t currVertexCount = static_cast<int32_t>(subMeshIR.Positions.size());

        allPositionCount += currVertexCount;

        if (!subMeshIR.Normals.empty())
        {
            HO_ASSERT(currVertexCount == static_cast<int32_t>(subMeshIR.Normals.size()),
                      "Vertex count mismatch between positions and normals in submesh.");
            allNormalCount += static_cast<int32_t>(subMeshIR.Normals.size());
        }

        if (!subMeshIR.Tangents.empty())
        {
            HO_ASSERT(currVertexCount == static_cast<int32_t>(subMeshIR.Tangents.size()),
                      "Vertex count mismatch between positions and tangents in submesh.");
            allTangentCount += static_cast<int32_t>(subMeshIR.Tangents.size());
        }

        if (!subMeshIR.UVs.empty())
        {
            HO_ASSERT(currVertexCount == static_cast<int32_t>(subMeshIR.UVs.size()),
                      "Vertex count mismatch between positions and UVs in submesh.");
            allUVCount += static_cast<int32_t>(subMeshIR.UVs.size());
        }

        if (!subMeshIR.Colors.empty())
        {
            HO_ASSERT(currVertexCount == static_cast<int32_t>(subMeshIR.Colors.size()),
                      "Vertex count mismatch between positions and colors in submesh.");
            allColorCount += static_cast<int32_t>(subMeshIR.Colors.size());
        }

        allIndexCount += static_cast<int32_t>(subMeshIR.Indices.size());
    }

    FixedArray<Vector3> positions(allPositionCount);
    FixedArray<Vector3> normals(allNormalCount);
    FixedArray<Vector4> tangents(allTangentCount);
    FixedArray<std::array<Vector2, RStaticMeshAsset::sMaxUVChannel>> uvs(allUVCount);
    FixedArray<std::array<Color32, RStaticMeshAsset::sMaxColorChannel>> colors(allColorCount);
    FixedArray<uint32_t> indices(allIndexCount);

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

    int32_t allTransformCount = 0;
    for (int32_t i = 0; i < static_cast<int32_t>(srcSkinIR.BindSubMeshIndices.size()); ++i)
    {
        allTransformCount += static_cast<int32_t>(srcSkinIR.BindSubMeshIndices[i].size());
    }
    FixedArray<Transform3D> localTransforms(allTransformCount);

    int32_t currVertexCount = 0;
    int32_t positionOffset = 0;
    int32_t normalOffset = 0;
    int32_t tangentOffset = 0;
    int32_t uvOffset = 0;
    int32_t colorOffset = 0;
    int32_t indexOffset = 0;
    int32_t localTransformOffset = 0;
    FixedArray<RStaticMeshAsset::SubMesh> subMeshes(static_cast<int32_t>(srcMeshIR.SubMeshes.size()));
    for (int32_t smi = 0; smi < static_cast<int32_t>(srcMeshIR.SubMeshes.size()); ++smi)
    {
        const parser::MeshIR::SubMesh& subMeshIR = srcMeshIR.SubMeshes[smi];
        subMeshes[smi].hName = StringHandle(subMeshIR.NameStr);
        subMeshes[smi].PrimitiveType = toAssetPrimitiveType(subMeshIR.PrimitiveType);
        currVertexCount = static_cast<int32_t>(subMeshIR.Positions.size());

        // Positions
        HO_ASSERT(!subMeshIR.Positions.empty(), "Submesh can't be empty.");
        subMeshes[smi].PositionOffset = positionOffset;
        std::memcpy((positions.Data() + positionOffset), subMeshIR.Positions.data(), sizeof(Vector3) * currVertexCount);
        positionOffset += currVertexCount;

        // Normals
        subMeshes[smi].NormalOffset = -1;
        if (!subMeshIR.Normals.empty())
        {
            subMeshes[smi].NormalOffset = normalOffset;
            std::memcpy(normals.Data() + normalOffset, subMeshIR.Normals.data(), sizeof(Vector3) * currVertexCount);
            normalOffset += currVertexCount;
        }

        // Tangents
        subMeshes[smi].TangentOffset = -1;
        if (!subMeshIR.Tangents.empty())
        {
            subMeshes[smi].TangentOffset = tangentOffset;
            std::memcpy(tangents.Data() + tangentOffset, subMeshIR.Tangents.data(), sizeof(Vector4) * currVertexCount);
            tangentOffset += currVertexCount;
        }

        // UVs
        subMeshes[smi].UVOffset = -1;
        if (!subMeshIR.UVs.empty())
        {
            subMeshes[smi].UVOffset = uvOffset;
            std::memcpy(uvs.Data() + uvOffset,
                        subMeshIR.UVs.data(),
                        sizeof(std::array<Vector2, RStaticMeshAsset::sMaxUVChannel>) * currVertexCount);
            uvOffset += currVertexCount;
        }

        // Colors
        subMeshes[smi].ColorOffset = -1;
        if (!subMeshIR.Colors.empty())
        {
            subMeshes[smi].ColorOffset = colorOffset;
            std::memcpy(colors.Data() + colorOffset,
                        subMeshIR.Colors.data(),
                        sizeof(std::array<Color32, RStaticMeshAsset::sMaxColorChannel>) * currVertexCount);
            colorOffset += currVertexCount;
        }

        // Indices
        subMeshes[smi].IndexOffset = indexOffset;
        std::memcpy(
            indices.Data() + indexOffset, subMeshIR.Indices.data(), sizeof(uint32_t) * subMeshIR.Indices.size());
        indexOffset += static_cast<int32_t>(subMeshIR.Indices.size());

        subMeshes[smi].VertexCount = currVertexCount;
        subMeshes[smi].IndexCount = static_cast<int32_t>(subMeshIR.Indices.size());

        // Transform
        int32_t localTransformCount = 0;
        for (int32_t ti = 0; ti < accTransforms.GetSize(); ++ti)
        {
            for (int32_t bsmi = 0; bsmi < static_cast<int32_t>(srcSkinIR.BindSubMeshIndices[ti].size()); ++bsmi)
            {
                if (smi == srcSkinIR.BindSubMeshIndices[ti][bsmi])
                {
                    localTransforms[localTransformOffset + localTransformCount] = accTransforms[ti];
                    ++localTransformCount;
                }
            }
        }
        subMeshes[smi].LocalTransformOffset = localTransformOffset;
        subMeshes[smi].LocalTransformCount = localTransformCount;
        localTransformOffset += localTransformCount;

        // Material
        HO_ASSERT(subMeshIR.RenderMaterialIndex < static_cast<int32_t>(pMaterialIRs.size()),
                  "Material index out of bounds.");
        const parser::MaterialIR& matIR = *pMaterialIRs[subMeshIR.RenderMaterialIndex];
        const std::string matNameStr = matIR.ResourcePath.GetParentPath().ToString() + "::Material::" + matIR.NameStr;
        subMeshes[smi].hRenderMaterialName = StringHandle(matNameStr);

        subMeshes[smi].AabbVolume = AABB::FromPositions(subMeshIR.Positions.data(), currVertexCount);
        subMeshes[smi].SphereVolume = Sphere::FromPositionsRitter(subMeshIR.Positions.data(), currVertexCount);
    }

    hOutAsset->Positions = std::move(positions);
    hOutAsset->Normals = std::move(normals);
    hOutAsset->Tangents = std::move(tangents);
    hOutAsset->UVs = std::move(uvs);
    hOutAsset->Colors = std::move(colors);
    hOutAsset->Indices = std::move(indices);
    hOutAsset->LocalTransforms = std::move(localTransforms);
    hOutAsset->SubMeshes = std::move(subMeshes);

    size_t totalWorldVertexCount = 0;
    for (int32_t smi = 0; smi < hOutAsset->SubMeshes.GetSize(); ++smi)
    {
        totalWorldVertexCount +=
            static_cast<size_t>(hOutAsset->SubMeshes[smi].VertexCount) * hOutAsset->SubMeshes[smi].LocalTransformCount;
    }

    std::vector<Vector3> worldPositions;
    worldPositions.reserve(totalWorldVertexCount);

    for (int32_t smi = 0; smi < hOutAsset->SubMeshes.GetSize(); ++smi)
    {
        for (int32_t lti = 0; lti < hOutAsset->SubMeshes[smi].LocalTransformCount; ++lti)
        {
            for (int32_t pi = 0; pi < hOutAsset->SubMeshes[smi].VertexCount; ++pi)
            {
                worldPositions.push_back(
                    hOutAsset->LocalTransforms[hOutAsset->SubMeshes[smi].LocalTransformOffset + lti].Transform(
                        hOutAsset->Positions[hOutAsset->SubMeshes[smi].PositionOffset + pi]));
            }
        }
    }

    hOutAsset->AabbVolume = AABB::FromPositions(worldPositions.data(), static_cast<int32_t>(worldPositions.size()));
    hOutAsset->SphereVolume =
        Sphere::FromPositionsRitter(worldPositions.data(), static_cast<int32_t>(worldPositions.size()));

    return true;
}

bool importMorphTargets(ObjectHandle<RMorphTargetsAsset> hOutAsset, const parser::MeshIR& srcMeshIR)
{
    if (hOutAsset.IsNULL())
    {
        return false;
    }

    int32_t currVertexCount = 0;
    int32_t allHasMorphSubMeshCount = 0;
    int32_t allPositionCount = 0;
    int32_t allNormalCount = 0;
    int32_t allTangentCount = 0;
    for (const auto& subMeshIR : srcMeshIR.SubMeshes)
    {
        if (!subMeshIR.MorphTargets.empty())
        {
            ++allHasMorphSubMeshCount;
            currVertexCount = static_cast<int32_t>(subMeshIR.Positions.size());
            for (const auto& morphTargetIR : subMeshIR.MorphTargets)
            {
                if (currVertexCount != static_cast<int32_t>(morphTargetIR.Positions.size()))
                {
                    HO_ASSERT(false, "Vertex count mismatch between submesh and morph target.");
                    return false;
                }
                allPositionCount += static_cast<int32_t>(morphTargetIR.Positions.size());
                if (!morphTargetIR.Normals.empty())
                {
                    if (currVertexCount != static_cast<int32_t>(morphTargetIR.Normals.size()))
                    {
                        HO_ASSERT(false, "Vertex count mismatch between morph target's position and normal.");
                        return false;
                    }
                    allNormalCount += static_cast<int32_t>(morphTargetIR.Normals.size());
                }
                if (!morphTargetIR.Tangents.empty())
                {
                    if (currVertexCount != static_cast<int32_t>(morphTargetIR.Tangents.size()))
                    {
                        HO_ASSERT(false, "Vertex count mismatch between morph target's position and tangent.");
                        return false;
                    }
                    allTangentCount += static_cast<int32_t>(morphTargetIR.Tangents.size());
                }
            }
        }
    }

    if (allHasMorphSubMeshCount == 0)
    {
        return false;
    }

    FixedArray<RMorphTargetsAsset::MorphTargets> subMeshMorphTargets(static_cast<int32_t>(allHasMorphSubMeshCount));
    FixedArray<Vector3> positions(allPositionCount);
    FixedArray<Vector3> normals(allNormalCount);
    FixedArray<Vector4> tangents(allTangentCount);

    int32_t subMeshMorphTargetsOffset = 0;
    int32_t positionOffset = 0;
    int32_t normalOffset = 0;
    int32_t tangentOffset = 0;
    for (int32_t smi = 0; smi < static_cast<int32_t>(srcMeshIR.SubMeshes.size()); ++smi)
    {
        const parser::MeshIR::SubMesh& subMeshIR = srcMeshIR.SubMeshes[smi];

        if (!subMeshIR.MorphTargets.empty())
        {
            currVertexCount = static_cast<int32_t>(subMeshIR.Positions.size());

            subMeshMorphTargets[subMeshMorphTargetsOffset].SubMeshIndex = smi;
            subMeshMorphTargets[subMeshMorphTargetsOffset].VertexCount = currVertexCount;

            FixedArray<RMorphTargetsAsset::MorphTarget> targets(static_cast<int32_t>(subMeshIR.MorphTargets.size()));
            for (int32_t mti = 0; mti < static_cast<int32_t>(subMeshIR.MorphTargets.size()); ++mti)
            {
                const parser::MeshIR::MorphTarget& morphTargetIR = subMeshIR.MorphTargets[mti];

                targets[mti].hName = StringHandle(morphTargetIR.NameStr);

                targets[mti].PositionOffset = positionOffset;
                std::memcpy(positions.Data() + positionOffset,
                            morphTargetIR.Positions.data(),
                            sizeof(Vector3) * currVertexCount);
                positionOffset += currVertexCount;

                targets[mti].NormalOffset = -1;
                if (!morphTargetIR.Normals.empty())
                {
                    targets[mti].NormalOffset = normalOffset;
                    std::memcpy(
                        normals.Data() + normalOffset, morphTargetIR.Normals.data(), sizeof(Vector3) * currVertexCount);
                    normalOffset += currVertexCount;
                }

                targets[mti].TangentOffset = -1;
                if (!morphTargetIR.Tangents.empty())
                {
                    targets[mti].TangentOffset = tangentOffset;
                    std::memcpy(tangents.Data() + tangentOffset,
                                morphTargetIR.Tangents.data(),
                                sizeof(Vector4) * currVertexCount);
                    tangentOffset += currVertexCount;
                }

                targets[mti].Weight = morphTargetIR.Weight;
            }

            subMeshMorphTargets[subMeshMorphTargetsOffset].Targets = std::move(targets);

            ++subMeshMorphTargetsOffset;
        }
    }

    hOutAsset->SubMeshMorphTargets = std::move(subMeshMorphTargets);
    hOutAsset->Positions = std::move(positions);
    hOutAsset->Normals = std::move(normals);
    hOutAsset->Tangents = std::move(tangents);

    return true;
}

bool importMorphAnimation(ObjectHandle<RMorphAnimationAsset> hOutAsset, const parser::AnimationIR& srcAnimIR)
{
    hOutAsset->Duration = srcAnimIR.Duration;

    FixedArray<RMorphAnimationAsset::MorphTrack> morphTracks(static_cast<int32_t>(srcAnimIR.MorphTargetTracks.size()));
    for (int32_t ti = 0; ti < static_cast<int32_t>(srcAnimIR.MorphTargetTracks.size()); ++ti)
    {
        const parser::AnimationIR::MorphTargetTrack& morphTrackIR = srcAnimIR.MorphTargetTracks[ti];

        FixedArray<int32_t> subMeshIndices(static_cast<int32_t>(morphTrackIR.SubMeshIndices.size()));
        std::memcpy(subMeshIndices.Data(),
                    morphTrackIR.SubMeshIndices.data(),
                    sizeof(int32_t) * morphTrackIR.SubMeshIndices.size());
        morphTracks[ti].SubmeshIndices = std::move(subMeshIndices);

        FixedArray<RMorphAnimationAsset::MorphKey> keySequence(static_cast<int32_t>(morphTrackIR.KeySequence.size()));
        for (int32_t ki = 0; ki < static_cast<int32_t>(morphTrackIR.KeySequence.size()); ki++)
        {
            const parser::AnimationIR::MorphKey& keyIR = morphTrackIR.KeySequence[ki];

            keySequence[ki].Time = keyIR.Time;

            FixedArray<int32_t> morphTargetIndices(static_cast<int32_t>(keyIR.MorphTargetIndices.size()));
            std::memcpy(morphTargetIndices.Data(),
                        keyIR.MorphTargetIndices.data(),
                        sizeof(int32_t) * keyIR.MorphTargetIndices.size());
            keySequence[ki].MorphTargetIndices = std::move(morphTargetIndices);

            FixedArray<float> weights(static_cast<int32_t>(keyIR.Weights.size()));
            std::memcpy(weights.Data(), keyIR.Weights.data(), sizeof(int32_t) * keyIR.Weights.size());
            keySequence[ki].Weights = std::move(weights);
        }
        morphTracks[ti].KeySequence = std::move(keySequence);
    }

    hOutAsset->Tracks = std::move(morphTracks);
    return true;
}

bool importMaterial(ObjectHandle<RMaterialAsset> hOutAsset,
                    const parser::MaterialIR& srcMaterialIR,
                    eMaterialAssetType type)
{
    hOutAsset->Type = type;

    hOutAsset->PipelineState.AlphaMode = toAssetAlphaMode(srcMaterialIR.AlphaMode);
    hOutAsset->PipelineState.AlphaBlendMode = toAssetAlphaBlendMode(srcMaterialIR.BlendMode);
    hOutAsset->PipelineState.bWireframe = srcMaterialIR.bWireframe;
    hOutAsset->PipelineState.bBackfaceCulling = srcMaterialIR.bBackfaceCulling;

    hOutAsset->Ambient = srcMaterialIR.Ambient;
    hOutAsset->Diffuse = srcMaterialIR.Diffuse;
    hOutAsset->Specular = srcMaterialIR.Specular;
    hOutAsset->Shininess = srcMaterialIR.Shininess;

    hOutAsset->Albedo = srcMaterialIR.Albedo;
    hOutAsset->Metallic = srcMaterialIR.Metallic;
    hOutAsset->Roughness = srcMaterialIR.Roughness;
    hOutAsset->IndexOfRefraction = srcMaterialIR.IndexOfRefraction;

    hOutAsset->Opacity = srcMaterialIR.Opacity;
    hOutAsset->AlphaThreshold = srcMaterialIR.AlphaThreshold;
    hOutAsset->Emissive = srcMaterialIR.Emissive;
    hOutAsset->EmissiveIntensity = srcMaterialIR.EmissiveIntensity;
    hOutAsset->NormalScale = srcMaterialIR.NormalScale;
    hOutAsset->OcclusionStrength = srcMaterialIR.OcclusionStrength;

    return true;
}

bool importTexturedMaterial(ObjectHandle<RMaterialAsset> hOutAsset,
                            const parser::MaterialIR& srcMaterialIR,
                            eMaterialAssetType type,
                            const std::vector<std::unique_ptr<const parser::TextureIR>>& pSrcTextureIRs)
{
    if (!importMaterial(hOutAsset, srcMaterialIR, type))
    {
        return false;
    }

    for (int32_t i = 1; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
    {
        const parser::MaterialIR::eTextureUsage texIrUsage = toIRTextureUsage(static_cast<eMaterialTextureUsage>(i));
        const int32_t texIndex = srcMaterialIR.TextureIRIndices[static_cast<int32_t>(texIrUsage)];
        if (texIndex == -1)
        {
            hOutAsset->hTextureNames[i] = StringHandle::sNULL;
        }
        else
        {
            const std::string nameStr = pSrcTextureIRs[texIndex]->ResourcePath.GetParentPath().ToString() +
                                        "::Texture2D::" + pSrcTextureIRs[texIndex]->NameStr;
            hOutAsset->hTextureNames[i] = StringHandle(nameStr);
        }
        hOutAsset->UVChannels[i] = srcMaterialIR.UVChannels[static_cast<int32_t>(texIrUsage)];
        hOutAsset->UVTransforms[i] = srcMaterialIR.UVTransforms[static_cast<int32_t>(texIrUsage)];
    }

    return true;
}

// Assumes srcTextureIR always uses a 4-channel memory layout (RGBA),
// regardless of the actual color channels or format of the source image.
// TODO: Refactor later. Doing way too much and BC compression logic is heavily duplicated.
bool importTexture2D(ObjectHandle<RTextureAsset> hOutAsset,
                     const parser::TextureIR& srcTextureIR,
                     eTextureFormat dstFormat,
                     bool bGenerateMipmap)

{
    if (srcTextureIR.Img.GetBitmap() == nullptr)
    {
        HO_ASSERT(false, "Source image was failed to parse.");
        return false;
    }

    hOutAsset->Width = srcTextureIR.Img.GetWidth();
    hOutAsset->Height = srcTextureIR.Img.GetHeight();
    hOutAsset->Depth = 1;
    hOutAsset->ArraySize = 1;
    hOutAsset->MipLevels = bGenerateMipmap ? texutils::calculateMipLevels(hOutAsset->Width, hOutAsset->Height) : 1;
    hOutAsset->Format = dstFormat;
    hOutAsset->Type = eTextureType::Texture2D;

    // Construct sub texture layouts
    int32_t totalDataSize = 0;
    hOutAsset->Layouts = FixedArray<RTextureAsset::SubTextureLayout>(hOutAsset->MipLevels);
    for (int32_t mipLevel = 0; mipLevel < hOutAsset->MipLevels; ++mipLevel)
    {
        const int32_t mipWidth =
            math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(srcTextureIR.Img.GetWidth()) >> mipLevel));
        const int32_t mipHeight =
            math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(srcTextureIR.Img.GetHeight()) >> mipLevel));
        hOutAsset->Layouts[mipLevel].Offset = totalDataSize;
        if (texutils::isUncompressedTextureFormat(dstFormat))
        {
            hOutAsset->Layouts[mipLevel].RowPitch = mipWidth * texutils::getTexturePixelBytes(dstFormat);
            hOutAsset->Layouts[mipLevel].Size = mipHeight * hOutAsset->Layouts[mipLevel].RowPitch;
        }
        else
        {
            hOutAsset->Layouts[mipLevel].RowPitch =
                texutils::getTextureBlockCount(mipWidth) * texutils::getTextureBlockBytes(dstFormat);
            hOutAsset->Layouts[mipLevel].Size =
                texutils::getTextureBlockCount(mipHeight) * hOutAsset->Layouts[mipLevel].RowPitch;
        }

        totalDataSize += hOutAsset->Layouts[mipLevel].Size;
    }
    hOutAsset->DataBlob = FixedArray<uint8_t>(totalDataSize);

    // Generate compressed mipmaps
    int32_t originImgWidth = hOutAsset->Width;
    int32_t originImgHeight = hOutAsset->Height;
    int32_t resizedImgWidth = originImgWidth;
    int32_t resizedImgHeight = originImgHeight;
    int32_t imgPixelBytes = 0;
    switch (dstFormat)
    {
        case eTextureFormat::BC4_SNORM:
        case eTextureFormat::BC4_UNORM:
            if (!(isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                  isLinearImageFormat(srcTextureIR.Img.GetFormat())))
            {
                HO_ASSERT(false, "Source image must be linear integer format. Please check source image format.");
                return false;
            }
            imgPixelBytes = 1;
            break;
        case eTextureFormat::BC5_SNORM:
        case eTextureFormat::BC5_UNORM:
            if (!(isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                  isLinearImageFormat(srcTextureIR.Img.GetFormat())))
            {
                HO_ASSERT(false, "Source image must be linear integer format. Please check source image format.");
                return false;
            }
            imgPixelBytes = 2;
            break;
        default:
            imgPixelBytes = isIntegerImageFormat(srcTextureIR.Img.GetFormat()) ? 4 : 16;
    }

    const int32_t texPixelBytes = texutils::isUncompressedTextureFormat(dstFormat)
                                      ? texutils::getTexturePixelBytes(dstFormat)
                                      : texutils::getTextureBlockBytes(dstFormat);
    if (texPixelBytes == 0)
    {
        HO_ASSERT(false, "Invalid texture pixel bytes (0).");
        return false;
    }
    FixedArray<uint8_t> imgBuffers[2] = {FixedArray<uint8_t>(hOutAsset->Width * hOutAsset->Height * imgPixelBytes),
                                         FixedArray<uint8_t>(hOutAsset->Width * hOutAsset->Height * imgPixelBytes)};
    int32_t frontBufferIndex = 0;
    int32_t backBufferIndex = 1;

    // original image is in back buffer initially.
    const int32_t pixelStride = isIntegerImageFormat(srcTextureIR.Img.GetFormat()) ? 4 : 16;

    for (int32_t i = 0; i < originImgWidth * originImgHeight; ++i)
    {
        const uint8_t* pSrcPixel = srcTextureIR.Img.GetBitmap() + (i * pixelStride);
        for (int32_t byteIdx = 0; byteIdx < imgPixelBytes; ++byteIdx)
        {
            imgBuffers[backBufferIndex][i * imgPixelBytes + byteIdx] = pSrcPixel[byteIdx];
        }
    }

    stbir_pixel_layout layout;
    switch (dstFormat)
    {
        case eTextureFormat::BC4_SNORM:
        case eTextureFormat::BC4_UNORM:
            layout = STBIR_1CHANNEL;
            break;
        case eTextureFormat::BC5_SNORM:
        case eTextureFormat::BC5_UNORM:
            layout = STBIR_2CHANNEL;
            break;
        default:
            layout = STBIR_4CHANNEL;
    }
    for (int32_t mipLevel = 0; mipLevel < hOutAsset->MipLevels; ++mipLevel)
    {
        const int32_t texWidth = hOutAsset->Layouts[mipLevel].RowPitch / texPixelBytes;
        const int32_t texHeight = hOutAsset->Layouts[mipLevel].Size / hOutAsset->Layouts[mipLevel].RowPitch;
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
                                                           layout);
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
                                                         layout);
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
                                              layout);
                break;
            default:
                HO_ASSERT(false, "Invalid image format.");
                return false;
        }

        if (!pResizeSuccess)
        {
            HO_ASSERT(false, "Failed to resize image.");
            return false;
        }

        // Compress current mipmap (from front buffer to dataBlob buffer)
        const uint8_t* pSrcImageBufferUint8 = imgBuffers[frontBufferIndex].Data();
        const float* pSrcImageBufferFloat = reinterpret_cast<float*>(imgBuffers[frontBufferIndex].Data());
        uint8_t* pDstTextureBufferUint8 = hOutAsset->DataBlob.Data() + hOutAsset->Layouts[mipLevel].Offset;
        half_float::half* pDstTextureBufferHalfFloat =
            reinterpret_cast<half_float::half*>(hOutAsset->DataBlob.Data() + hOutAsset->Layouts[mipLevel].Offset);
        float* pDstTextureBufferFloat =
            reinterpret_cast<float*>(hOutAsset->DataBlob.Data() + hOutAsset->Layouts[mipLevel].Offset);
        switch (dstFormat)
        {
            case eTextureFormat::R8_UNORM:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBufferUint8 + hi * texWidth + wi)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, linear format. Please check source image format.");
                    return false;
                }
                break;
            case eTextureFormat::R8G8_UNORM:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 2)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 2)[1] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, linear format. Please check source image format.");
                    return false;
                }
                break;
            case eTextureFormat::R8G8B8A8_UNORM:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[1] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[2] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[3] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, linear format. Please check source image format.");
                    return false;
                }
                break;
            case eTextureFormat::R8_SRGB:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    !isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBufferUint8 + hi * texWidth + wi)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, srgb format. Please check source image format.");
                    return false;
                }
                break;
            case eTextureFormat::R8G8_SRGB:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    !isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 2)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 2)[1] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, srgb format. Please check source image format.");
                    return false;
                }
                break;
            case eTextureFormat::R8G8B8A8_SRGB:
                if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    !isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[0] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[1] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[2] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                            (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[3] =
                                (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be integer, srgb format. Please check source image format.");
                    return false;
                }
                break;
            case eTextureFormat::R16_FLOAT:
                if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBufferHalfFloat + hi * texWidth + wi)[0] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be float, linear format. Please check source image format.");
                    return false;
                }
                break;
            case eTextureFormat::R16G16_FLOAT:
                if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 2)[0] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 2)[1] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be float, linear format. Please check source image format.");
                    return false;
                }
                break;
            case eTextureFormat::R16G16B16A16_FLOAT:
                if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 4)[0] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 4)[1] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 4)[2] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                            (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 4)[3] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be float, linear format. Please check source image format.");
                    return false;
                }
                break;
            case eTextureFormat::R32_FLOAT:
                if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                    isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                {
                    for (int32_t hi = 0; hi < texHeight; ++hi)
                    {
                        for (int32_t wi = 0; wi < texWidth; ++wi)
                        {
                            (pDstTextureBufferFloat + hi * texWidth + wi)[0] =
                                (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                        }
                    }
                }
                else
                {
                    HO_ASSERT(false, "Source image must be float, linear format. Please check source image format.");
                    return false;
                }
                break;

            // Add padding to source image.
            case eTextureFormat::BC1_UNORM:
            case eTextureFormat::BC3_UNORM:
            case eTextureFormat::BC4_UNORM:
            case eTextureFormat::BC4_SNORM:
            case eTextureFormat::BC5_UNORM:
            case eTextureFormat::BC5_SNORM:
            case eTextureFormat::BC7_UNORM:
            case eTextureFormat::BC1_SRGB:
            case eTextureFormat::BC3_SRGB:
            case eTextureFormat::BC7_SRGB:
            case eTextureFormat::BC6H_UF16:
            case eTextureFormat::BC6H_SF16:
            {
                const bool bIsBC6H = (dstFormat == eTextureFormat::BC6H_UF16 || dstFormat == eTextureFormat::BC6H_SF16);

                const uint8_t* pSrcImageBuffer =
                    bIsBC6H ? reinterpret_cast<const uint8_t*>(pSrcImageBufferFloat) : pSrcImageBufferUint8;

                const int32_t paddedWidth = math::Max(4, ((resizedImgWidth + 3) / 4) * 4);
                const int32_t paddedHeight = math::Max(4, ((resizedImgHeight + 3) / 4) * 4);
                const int32_t paddedStride = paddedWidth * imgPixelBytes;

                const uint8_t* pFinalSrcPixels = pSrcImageBuffer;

                FixedArray<uint8_t> paddedBuffer;

                if (paddedWidth != resizedImgWidth || paddedHeight != resizedImgHeight)
                {
                    paddedBuffer = FixedArray<uint8_t>(paddedWidth * paddedHeight * imgPixelBytes);

                    for (int32_t y = 0; y < paddedHeight; ++y)
                    {
                        const int32_t clampY = math::Min(y, resizedImgHeight - 1);
                        for (int32_t x = 0; x < paddedWidth; ++x)
                        {
                            const int32_t clampX = math::Min(x, resizedImgWidth - 1);

                            const uint8_t* pSrcPixel =
                                pSrcImageBuffer + (clampY * resizedImgWidth + clampX) * imgPixelBytes;
                            uint8_t* pDstPixel = paddedBuffer.Data() + (y * paddedWidth + x) * imgPixelBytes;

                            memcpy(pDstPixel, pSrcPixel, imgPixelBytes);
                        }
                    }
                    pFinalSrcPixels = paddedBuffer.Data();
                }

                rgba_surface surface{};
                surface.ptr = const_cast<uint8_t*>(pFinalSrcPixels); // NOLINT
                if (paddedWidth != resizedImgWidth || paddedHeight != resizedImgHeight)
                {
                    surface.width = paddedWidth;
                    surface.height = paddedHeight;
                    surface.stride = paddedStride;
                }
                else
                {
                    surface.width = resizedImgWidth;
                    surface.height = resizedImgHeight;
                    surface.stride = resizedImgWidth * imgPixelBytes;
                }

                switch (dstFormat)
                {
                    case eTextureFormat::BC1_UNORM:
                        if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                            isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                        {
                            CompressBlocksBC1(&surface, pDstTextureBufferUint8);
                        }
                        else
                        {
                            HO_ASSERT(false,
                                      "Source image must be linear integer format. Please check source image format.");
                            return false;
                        }
                        break;
                    case eTextureFormat::BC1_SRGB:
                        if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                            !isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                        {
                            CompressBlocksBC1(&surface, pDstTextureBufferUint8);
                        }
                        else
                        {
                            HO_ASSERT(false,
                                      "Source image must be sRGB integer format. Please check source image format.");
                            return false;
                        }
                        break;
                    case eTextureFormat::BC3_UNORM:
                        if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                            isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                        {
                            CompressBlocksBC3(&surface, pDstTextureBufferUint8);
                        }
                        else
                        {
                            HO_ASSERT(false,
                                      "Source image must be linear integer format. Please check source image format.");
                            return false;
                        }
                        break;
                    case eTextureFormat::BC3_SRGB:
                        if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                            !isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                        {
                            CompressBlocksBC3(&surface, pDstTextureBufferUint8);
                        }
                        else
                        {
                            HO_ASSERT(false,
                                      "Source image must be sRGB integer format. Please check source image format.");
                            return false;
                        }
                        break;
                    case eTextureFormat::BC4_UNORM:
                    case eTextureFormat::BC4_SNORM:
                        if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                            isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                        {
                            CompressBlocksBC4(&surface, pDstTextureBufferUint8);
                        }
                        else
                        {
                            HO_ASSERT(false,
                                      "Source image must be linear integer format. Please check source image format.");
                            return false;
                        }
                        break;
                    case eTextureFormat::BC5_UNORM:
                    case eTextureFormat::BC5_SNORM:
                        if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                            isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                        {
                            CompressBlocksBC5(&surface, pDstTextureBufferUint8);
                        }
                        else
                        {
                            HO_ASSERT(false,
                                      "Source image must be linear integer format. Please check source image format.");
                            return false;
                        }
                        break;
                    case eTextureFormat::BC6H_UF16:
                    case eTextureFormat::BC6H_SF16:
                        if (!isIntegerImageFormat(srcTextureIR.Img.GetFormat()))
                        {
                            // Convert to half-float first
                            FixedArray<half_float::half> halfFloatBuffer(surface.width * surface.height * 4);

                            const float* pSrcFloat = reinterpret_cast<const float*>(surface.ptr);
                            for (int32_t p = 0; p < surface.width * surface.height * 4; ++p)
                            {
                                halfFloatBuffer[p] = half_float::half(pSrcFloat[p]);
                            }

                            surface.ptr = reinterpret_cast<uint8_t*>(halfFloatBuffer.Data());
                            surface.stride = surface.width * 8;

                            // Compress
                            bc6h_enc_settings bc6hSettings{};
                            GetProfile_bc6h_basic(&bc6hSettings);
                            CompressBlocksBC6H(&surface, pDstTextureBufferUint8, &bc6hSettings);
                        }
                        else
                        {
                            HO_ASSERT(false, "Source image must be float format. Please check source image format.");
                            return false;
                        }
                        break;
                    case eTextureFormat::BC7_UNORM:
                        if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                            isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                        {
                            bc7_enc_settings bc7Settings{};
                            if (srcTextureIR.Img.GetLogicalChannelCount() < 4)
                            {
                                GetProfile_basic(&bc7Settings);
                            }
                            else
                            {
                                GetProfile_alpha_basic(&bc7Settings);
                            }

                            CompressBlocksBC7(&surface, pDstTextureBufferUint8, &bc7Settings);
                        }
                        else
                        {
                            HO_ASSERT(false,
                                      "Source image must be linear integer format. Please check source image format.");
                            return false;
                        }
                        break;
                    case eTextureFormat::BC7_SRGB:
                        if (isIntegerImageFormat(srcTextureIR.Img.GetFormat()) &&
                            !isLinearImageFormat(srcTextureIR.Img.GetFormat()))
                        {
                            bc7_enc_settings bc7Settings{};
                            if (srcTextureIR.Img.GetLogicalChannelCount() < 4)
                            {
                                GetProfile_basic(&bc7Settings);
                            }
                            else
                            {
                                GetProfile_alpha_basic(&bc7Settings);
                            }

                            CompressBlocksBC7(&surface, pDstTextureBufferUint8, &bc7Settings);
                        }
                        else
                        {
                            HO_ASSERT(false,
                                      "Source image must be sRGB integer format. Please check source image format.");
                            return false;
                        }
                        break;
                    default:
                        HO_ASSERT(false, "Invalid texture format.");
                        return false;
                }
            }
            break;
            default:
                HO_ASSERT(false, "Invalid texture format.");
                return false;
        }

        frontBufferIndex = (frontBufferIndex + 1) % 2;
        backBufferIndex = (backBufferIndex + 1) % 2;
        imgBuffers[frontBufferIndex].Fill(0);
        originImgWidth = resizedImgWidth;
        originImgHeight = resizedImgHeight;
        resizedImgWidth = math::Max(1, static_cast<int32_t>(originImgWidth / 2));
        resizedImgHeight = math::Max(1, static_cast<int32_t>(originImgHeight / 2));
    }

    return true;
}

// Assumes srcTextureIR always uses a 4-channel memory layout (RGBA),
// regardless of the actual color channels or format of the source image.
// TODO: Refactor later. Doing way too much and BC compression logic is heavily duplicated.
bool importTextureCubemap(ObjectHandle<RTextureAsset> hOutAsset,
                          const std::string& nameStr,
                          const parser::TextureIR& srcPosX,
                          const parser::TextureIR& srcNegX,
                          const parser::TextureIR& srcPosY,
                          const parser::TextureIR& srcNegY,
                          const parser::TextureIR& srcPosZ,
                          const parser::TextureIR& srcNegZ,
                          eTextureFormat dstFormat,
                          bool bGenerateMipmap)
{
    // Validate parameters
    if (srcPosX.Img.GetBitmap() == nullptr || srcNegX.Img.GetBitmap() == nullptr ||
        srcPosY.Img.GetBitmap() == nullptr || srcNegY.Img.GetBitmap() == nullptr ||
        srcPosZ.Img.GetBitmap() == nullptr || srcNegZ.Img.GetBitmap() == nullptr)
    {
        HO_ASSERT(false, "Source images were failed to parse.");
        return false;
    }

    const int32_t baseWidth = srcPosX.Img.GetWidth();
    const int32_t baseHeight = srcPosX.Img.GetHeight();
    const Image::eFormat baseFormat = srcPosX.Img.GetFormat();
    const int32_t baseLogicalChannels = srcPosX.Img.GetLogicalChannelCount();
    const int32_t basePhysicalChannels = srcPosX.Img.GetPhysicalChannelCount();

    if (baseWidth != baseHeight)
    {
        HO_ASSERT(false, "Cubemap faces must be square (width == height).");
        return false;
    }

    if (srcNegX.Img.GetWidth() != baseWidth || srcNegX.Img.GetHeight() != baseHeight ||
        srcPosY.Img.GetWidth() != baseWidth || srcPosY.Img.GetHeight() != baseHeight ||
        srcNegY.Img.GetWidth() != baseWidth || srcNegY.Img.GetHeight() != baseHeight ||
        srcPosZ.Img.GetWidth() != baseWidth || srcPosZ.Img.GetHeight() != baseHeight ||
        srcNegZ.Img.GetWidth() != baseWidth || srcNegZ.Img.GetHeight() != baseHeight)
    {
        HO_ASSERT(false, "All 6 faces of a cubemap must have the exact same dimensions.");
        return false;
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
        return false;
    }

    hOutAsset->Width = baseWidth;
    hOutAsset->Height = baseHeight;
    hOutAsset->Depth = 1;
    hOutAsset->ArraySize = 1;
    hOutAsset->MipLevels = bGenerateMipmap ? texutils::calculateMipLevels(hOutAsset->Width, hOutAsset->Height) : 1;
    hOutAsset->Format = dstFormat;
    hOutAsset->Type = eTextureType::TextureCubeMap;

    // Construct sub texture layouts
    int32_t totalDataSize = 0;
    hOutAsset->Layouts = FixedArray<RTextureAsset::SubTextureLayout>(hOutAsset->MipLevels);
    for (int32_t mipLevel = 0; mipLevel < hOutAsset->MipLevels; ++mipLevel)
    {
        const int32_t mipWidth = math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(baseWidth) >> mipLevel));
        const int32_t mipHeight = math::Max(1, static_cast<int32_t>(static_cast<uint32_t>(baseHeight) >> mipLevel));
        hOutAsset->Layouts[mipLevel].Offset = totalDataSize;
        if (texutils::isUncompressedTextureFormat(dstFormat))
        {
            hOutAsset->Layouts[mipLevel].RowPitch = mipWidth * texutils::getTexturePixelBytes(dstFormat);
            hOutAsset->Layouts[mipLevel].Size = 6 * mipHeight * hOutAsset->Layouts[mipLevel].RowPitch;
        }
        else
        {
            hOutAsset->Layouts[mipLevel].RowPitch =
                texutils::getTextureBlockCount(mipWidth) * texutils::getTextureBlockBytes(dstFormat);
            hOutAsset->Layouts[mipLevel].Size =
                6 * texutils::getTextureBlockCount(mipHeight) * hOutAsset->Layouts[mipLevel].RowPitch;
        }

        totalDataSize += hOutAsset->Layouts[mipLevel].Size;
    }
    hOutAsset->DataBlob = FixedArray<uint8_t>(totalDataSize);

    // Generate compressed mipmaps
    const parser::TextureIR* const pSrcTexIRs[6] = {&srcPosX, &srcNegX, &srcPosY, &srcNegY, &srcPosZ, &srcNegZ};
    int32_t originImgWidth = hOutAsset->Width;
    int32_t originImgHeight = hOutAsset->Height;
    int32_t resizedImgWidth = originImgWidth;
    int32_t resizedImgHeight = originImgHeight;
    int32_t imgPixelBytes = 0;
    switch (dstFormat)
    {
        case eTextureFormat::BC4_SNORM:
        case eTextureFormat::BC4_UNORM:
            if (!(isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat)))
            {
                HO_ASSERT(false, "Source image must be linear integer format. Please check source image format.");
                return false;
            }
            imgPixelBytes = 1;
            break;
        case eTextureFormat::BC5_SNORM:
        case eTextureFormat::BC5_UNORM:
            if (!(isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat)))
            {
                HO_ASSERT(false, "Source image must be linear integer format. Please check source image format.");
                return false;
            }
            imgPixelBytes = 2;
            break;
        default:
            imgPixelBytes = isIntegerImageFormat(baseFormat) ? 4 : 16;
    }

    const int32_t texPixelBytes = texutils::isUncompressedTextureFormat(dstFormat)
                                      ? texutils::getTexturePixelBytes(dstFormat)
                                      : texutils::getTextureBlockBytes(dstFormat);
    if (texPixelBytes == 0)
    {
        HO_ASSERT(false, "Invalid texture pixel bytes (0).");
        return false;
    }
    FixedArray<uint8_t> imgBuffers[2][6] = {};
    for (int32_t i = 0; i < 2; ++i)
    {
        for (int32_t j = 0; j < 6; ++j)
        {
            imgBuffers[i][j] = FixedArray<uint8_t>(hOutAsset->Width * hOutAsset->Height * imgPixelBytes);
        }
    }
    int32_t frontBufferIndex = 0;
    int32_t backBufferIndex = 1;

    // original image is in back buffer initially.
    const int32_t pixelStride = isIntegerImageFormat(baseFormat) ? 4 : 16;

    for (int32_t face = 0; face < 6; ++face)
    {
        for (int32_t i = 0; i < originImgWidth * originImgHeight; ++i)
        {
            const uint8_t* pSrcPixel = pSrcTexIRs[face]->Img.GetBitmap() + (i * pixelStride);
            for (int32_t byteIdx = 0; byteIdx < imgPixelBytes; ++byteIdx)
            {
                imgBuffers[backBufferIndex][face][i * imgPixelBytes + byteIdx] = pSrcPixel[byteIdx];
            }
        }
    }

    stbir_pixel_layout layout;
    switch (dstFormat)
    {
        case eTextureFormat::BC4_SNORM:
        case eTextureFormat::BC4_UNORM:
            layout = STBIR_1CHANNEL;
            break;
        case eTextureFormat::BC5_SNORM:
        case eTextureFormat::BC5_UNORM:
            layout = STBIR_2CHANNEL;
            break;
        default:
            layout = STBIR_4CHANNEL;
    }

    for (int32_t mipLevel = 0; mipLevel < hOutAsset->MipLevels; ++mipLevel)
    {
        const int32_t texWidth = hOutAsset->Layouts[mipLevel].RowPitch / texPixelBytes;
        const int32_t texHeight = hOutAsset->Layouts[mipLevel].Size / hOutAsset->Layouts[mipLevel].RowPitch;
        for (int32_t face = 0; face < 6; ++face)
        {
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
                                                               layout);
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
                                                             layout);
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
                        layout);
                    break;
                default:
                    HO_ASSERT(false, "Invalid image format.");
                    return false;
            }

            if (!pResizeSuccess)
            {
                HO_ASSERT(false, "Failed to resize image.");
                return false;
            }

            // Compress current mipmap (from front buffer to dataBlob buffer)
            const uint8_t* pSrcImageBufferUint8 = imgBuffers[frontBufferIndex][face].Data();
            const float* pSrcImageBufferFloat = reinterpret_cast<float*>(imgBuffers[frontBufferIndex][face].Data());
            uint8_t* pDstTextureBufferUint8 = hOutAsset->DataBlob.Data() + hOutAsset->Layouts[mipLevel].Offset +
                                              face * (hOutAsset->Layouts[mipLevel].Size / 6);
            half_float::half* pDstTextureBufferHalfFloat =
                reinterpret_cast<half_float::half*>(hOutAsset->DataBlob.Data() + hOutAsset->Layouts[mipLevel].Offset +
                                                    face * (hOutAsset->Layouts[mipLevel].Size / 6));
            float* pDstTextureBufferFloat =
                reinterpret_cast<float*>(hOutAsset->DataBlob.Data() + hOutAsset->Layouts[mipLevel].Offset +
                                         face * (hOutAsset->Layouts[mipLevel].Size / 6));
            switch (dstFormat)
            {
                case eTextureFormat::R8_UNORM:
                    if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBufferUint8 + hi * texWidth + wi)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, linear format. Please check source image format.");
                        return false;
                    }
                    break;
                case eTextureFormat::R8G8_UNORM:
                    if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 2)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 2)[1] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, linear format. Please check source image format.");
                        return false;
                    }
                    break;
                case eTextureFormat::R8G8B8A8_UNORM:
                    if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[1] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[2] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[3] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, linear format. Please check source image format.");
                        return false;
                    }
                    break;
                case eTextureFormat::R8_SRGB:
                    if (isIntegerImageFormat(baseFormat) && !isLinearImageFormat(baseFormat))
                    {
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBufferUint8 + hi * texWidth + wi)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, srgb format. Please check source image format.");
                        return false;
                    }
                    break;
                case eTextureFormat::R8G8_SRGB:
                    if (isIntegerImageFormat(baseFormat) && !isLinearImageFormat(baseFormat))
                    {
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 2)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 2)[1] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, srgb format. Please check source image format.");
                        return false;
                    }
                    break;
                case eTextureFormat::R8G8B8A8_SRGB:
                    if (isIntegerImageFormat(baseFormat) && !isLinearImageFormat(baseFormat))
                    {
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[0] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[1] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[2] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                                (pDstTextureBufferUint8 + (hi * texWidth + wi) * 4)[3] =
                                    (pSrcImageBufferUint8 + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be integer, srgb format. Please check source image format.");
                        return false;
                    }
                    break;
                case eTextureFormat::R16_FLOAT:
                    if (!isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBufferHalfFloat + hi * texWidth + wi)[0] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be float, linear format. Please check source image format.");
                        return false;
                    }
                    break;
                case eTextureFormat::R16G16_FLOAT:
                    if (!isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 2)[0] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 2)[1] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be float, linear format. Please check source image format.");
                        return false;
                    }
                    break;
                case eTextureFormat::R16G16B16A16_FLOAT:
                    if (!isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 4)[0] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                                (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 4)[1] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[1]; // copy g channel.
                                (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 4)[2] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[2]; // copy b channel.
                                (pDstTextureBufferHalfFloat + (hi * texWidth + wi) * 4)[3] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[3]; // copy a channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be float, linear format. Please check source image format.");
                        return false;
                    }
                    break;
                case eTextureFormat::R32_FLOAT:
                    if (!isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                    {
                        for (int32_t hi = 0; hi < texHeight; ++hi)
                        {
                            for (int32_t wi = 0; wi < texWidth; ++wi)
                            {
                                (pDstTextureBufferFloat + hi * texWidth + wi)[0] =
                                    (pSrcImageBufferFloat + (hi * texWidth + wi) * 4)[0]; // copy r channel.
                            }
                        }
                    }
                    else
                    {
                        HO_ASSERT(false,
                                  "Source image must be float, linear format. Please check source image format.");
                        return false;
                    }
                    break;

                    // Add padding to source image
                case eTextureFormat::BC1_UNORM:
                case eTextureFormat::BC3_UNORM:
                case eTextureFormat::BC4_UNORM:
                case eTextureFormat::BC4_SNORM:
                case eTextureFormat::BC5_UNORM:
                case eTextureFormat::BC5_SNORM:
                case eTextureFormat::BC7_UNORM:
                case eTextureFormat::BC1_SRGB:
                case eTextureFormat::BC3_SRGB:
                case eTextureFormat::BC7_SRGB:
                case eTextureFormat::BC6H_UF16:
                case eTextureFormat::BC6H_SF16:
                {
                    const bool bIsBC6H =
                        (dstFormat == eTextureFormat::BC6H_UF16 || dstFormat == eTextureFormat::BC6H_SF16);

                    const uint8_t* pSrcImageBuffer =
                        bIsBC6H ? reinterpret_cast<const uint8_t*>(pSrcImageBufferFloat) : pSrcImageBufferUint8;

                    const int32_t paddedWidth = math::Max(4, ((resizedImgWidth + 3) / 4) * 4);
                    const int32_t paddedHeight = math::Max(4, ((resizedImgHeight + 3) / 4) * 4);
                    const int32_t paddedStride = paddedWidth * imgPixelBytes;

                    const uint8_t* pFinalSrcPixels = pSrcImageBuffer;

                    FixedArray<uint8_t> paddedBuffer;

                    if (paddedWidth != resizedImgWidth || paddedHeight != resizedImgHeight)
                    {
                        paddedBuffer = FixedArray<uint8_t>(paddedWidth * paddedHeight * imgPixelBytes);

                        for (int32_t y = 0; y < paddedHeight; ++y)
                        {
                            const int32_t clampY = math::Min(y, resizedImgHeight - 1);
                            for (int32_t x = 0; x < paddedWidth; ++x)
                            {
                                const int32_t clampX = math::Min(x, resizedImgWidth - 1);

                                const uint8_t* pSrcPixel =
                                    pSrcImageBuffer + (clampY * resizedImgWidth + clampX) * imgPixelBytes;
                                uint8_t* pDstPixel = paddedBuffer.Data() + (y * paddedWidth + x) * imgPixelBytes;

                                memcpy(pDstPixel, pSrcPixel, imgPixelBytes);
                            }
                        }
                        pFinalSrcPixels = paddedBuffer.Data();
                    }

                    rgba_surface surface{};
                    surface.ptr = const_cast<uint8_t*>(pFinalSrcPixels); // NOLINT
                    if (paddedWidth != resizedImgWidth || paddedHeight != resizedImgHeight)
                    {
                        surface.width = paddedWidth;
                        surface.height = paddedHeight;
                        surface.stride = paddedStride;
                    }
                    else
                    {
                        surface.width = resizedImgWidth;
                        surface.height = resizedImgHeight;
                        surface.stride = resizedImgWidth * imgPixelBytes;
                    }

                    switch (dstFormat)
                    {
                        case eTextureFormat::BC1_UNORM:
                            if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                            {
                                CompressBlocksBC1(&surface, pDstTextureBufferUint8);
                            }
                            else
                            {
                                HO_ASSERT(
                                    false,
                                    "Source image must be linear integer format. Please check source image format.");
                                return false;
                            }
                            break;
                        case eTextureFormat::BC1_SRGB:
                            if (isIntegerImageFormat(baseFormat) && !isLinearImageFormat(baseFormat))
                            {
                                CompressBlocksBC1(&surface, pDstTextureBufferUint8);
                            }
                            else
                            {
                                HO_ASSERT(
                                    false,
                                    "Source image must be sRGB integer format. Please check source image format.");
                                return false;
                            }
                            break;
                        case eTextureFormat::BC3_UNORM:
                            if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                            {
                                CompressBlocksBC3(&surface, pDstTextureBufferUint8);
                            }
                            else
                            {
                                HO_ASSERT(
                                    false,
                                    "Source image must be linear integer format. Please check source image format.");
                                return false;
                            }
                            break;
                        case eTextureFormat::BC3_SRGB:
                            if (isIntegerImageFormat(baseFormat) && !isLinearImageFormat(baseFormat))
                            {
                                CompressBlocksBC3(&surface, pDstTextureBufferUint8);
                            }
                            else
                            {
                                HO_ASSERT(
                                    false,
                                    "Source image must be sRGB integer format. Please check source image format.");
                                return false;
                            }
                            break;
                        case eTextureFormat::BC4_UNORM:
                        case eTextureFormat::BC4_SNORM:
                            if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                            {
                                CompressBlocksBC4(&surface, pDstTextureBufferUint8);
                            }
                            else
                            {
                                HO_ASSERT(
                                    false,
                                    "Source image must be linear integer format. Please check source image format.");
                                return false;
                            }
                            break;
                        case eTextureFormat::BC5_UNORM:
                        case eTextureFormat::BC5_SNORM:
                            if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                            {
                                CompressBlocksBC5(&surface, pDstTextureBufferUint8);
                            }
                            else
                            {
                                HO_ASSERT(
                                    false,
                                    "Source image must be linenar integer format. Please check source image format.");
                                return false;
                            }
                            break;
                        case eTextureFormat::BC6H_UF16:
                        case eTextureFormat::BC6H_SF16:
                            if (!isIntegerImageFormat(baseFormat))
                            {
                                // Convert to half-float first
                                FixedArray<half_float::half> halfFloatBuffer(surface.width * surface.height * 4);

                                const float* pSrcFloat = reinterpret_cast<const float*>(surface.ptr);
                                for (int32_t p = 0; p < surface.width * surface.height * 4; ++p)
                                {
                                    halfFloatBuffer[p] = half_float::half(pSrcFloat[p]);
                                }

                                surface.ptr = reinterpret_cast<uint8_t*>(halfFloatBuffer.Data());
                                surface.stride = surface.width * 8;

                                // Compress
                                bc6h_enc_settings bc6hSettings{};
                                GetProfile_bc6h_basic(&bc6hSettings);
                                CompressBlocksBC6H(&surface, pDstTextureBufferUint8, &bc6hSettings);
                            }
                            else
                            {
                                HO_ASSERT(false,
                                          "Source image must be float format. Please check source image format.");
                                return false;
                            }
                            break;
                        case eTextureFormat::BC7_UNORM:
                            if (isIntegerImageFormat(baseFormat) && isLinearImageFormat(baseFormat))
                            {
                                bc7_enc_settings bc7Settings{};
                                if (baseLogicalChannels < 4)
                                {
                                    GetProfile_basic(&bc7Settings);
                                }
                                else
                                {
                                    GetProfile_alpha_basic(&bc7Settings);
                                }

                                CompressBlocksBC7(&surface, pDstTextureBufferUint8, &bc7Settings);
                            }
                            else
                            {
                                HO_ASSERT(
                                    false,
                                    "Source image must be linear integer format. Please check source image format.");
                                return false;
                            }
                            break;
                        case eTextureFormat::BC7_SRGB:
                            if (isIntegerImageFormat(baseFormat) && !isLinearImageFormat(baseFormat))
                            {
                                bc7_enc_settings bc7Settings{};
                                if (baseLogicalChannels < 4)
                                {
                                    GetProfile_basic(&bc7Settings);
                                }
                                else
                                {
                                    GetProfile_alpha_basic(&bc7Settings);
                                }

                                CompressBlocksBC7(&surface, pDstTextureBufferUint8, &bc7Settings);
                            }
                            else
                            {
                                HO_ASSERT(
                                    false,
                                    "Source image must be sRGB integer format. Please check source image format.");
                                return false;
                            }
                            break;
                        default:
                            HO_ASSERT(false, "Invalid texture format.");
                            return false;
                    }
                }
                break;
                default:
                    HO_ASSERT(false, "Invalid texture format.");
                    return false;
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
    return true;
}

bool importShader(ObjectHandle<RShaderAsset> hOutAsset,
                  std::string* pOutErrorMsg,
                  const parser::ShaderIR& srcShaderIR,
                  eShaderStage stage,
                  eMaterialAssetType compatibleMaterialTypes)
{
    hOutAsset->SourceStr = srcShaderIR.SourceStr;

    const shaderc::Compiler compiler;
    bool bOptimize = true; // NOLINT
#if defined(DEBUG) | defined(_DEBUG)
    bOptimize = false;
#endif
    std::vector<uint32_t> binary = compileFileGLSL(
        compiler, srcShaderIR.NameStr, toShadercStageGLSL(stage), srcShaderIR.SourceStr, pOutErrorMsg, bOptimize);
    if (binary.empty())
    {
        return false;
    }
    hOutAsset->Binary = FixedArray<uint32_t>(static_cast<int32_t>(binary.size()));
    for (int32_t i = 0; i < static_cast<int32_t>(binary.size()); ++i)
    {
        hOutAsset->Binary[i] = binary[i];
    }

    hOutAsset->Stage = stage;
    hOutAsset->CompatibleMaterialType = compatibleMaterialTypes;
    return true;
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

shaderc_shader_kind toShadercStageGLSL(eShaderStage stage)
{
    switch (stage)
    {
        case eShaderStage::VertexShader:
            return shaderc_shader_kind::shaderc_glsl_vertex_shader;
        case eShaderStage::GeometryShader:
            return shaderc_shader_kind::shaderc_glsl_geometry_shader;
        case eShaderStage::FragmentShader:
            return shaderc_shader_kind::shaderc_glsl_fragment_shader;
        case eShaderStage::ComputeShader:
            return shaderc_shader_kind::shaderc_glsl_compute_shader;
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
        return "";
    }

    return {result.cbegin(), result.cend()};
}

std::vector<uint32_t> compileFileGLSL(const shaderc::Compiler& compiler,
                                      const std::string& sourceName,
                                      shaderc_shader_kind kind,
                                      const std::string& source,
                                      std::string* pOutErrorMsg,
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
        if (pOutErrorMsg)
        {
            *pOutErrorMsg = module.GetErrorMessage();
        }
        return std::vector<uint32_t>();
    }

    return {module.cbegin(), module.cend()};
}

} // namespace importer
} // namespace ho