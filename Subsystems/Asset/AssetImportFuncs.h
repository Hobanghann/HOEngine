#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AssetDefs.h"

namespace ho
{
namespace parser
{
struct MeshIR;
struct SkeletonIR;
struct SkinIR;
struct AnimationIR;
struct MaterialIR;
struct TextureIR;
struct ShaderIR;
} // namespace parser

struct StaticMeshAsset;
struct TextureAsset;
struct MaterialAsset;
struct ShaderAsset;

namespace importer
{
[[nodiscard]] std::unique_ptr<StaticMeshAsset> importStaticMesh(const parser::MeshIR& srcMeshIR,
                                                                const parser::SkeletonIR& srcSkeletonIR,
                                                                const parser::SkinIR& srcSkinIR);
[[nodiscard]] std::unique_ptr<MaterialAsset> importMaterial(const parser::MaterialIR& srcMaterialIR,
                                                            eMaterialAssetType type);
[[nodiscard]] std::unique_ptr<MaterialAsset> importTexturedMaterial(
    const parser::MaterialIR& srcMaterialIR,
    eMaterialAssetType type,
    const std::vector<std::unique_ptr<const parser::TextureIR>>& pSrcTextureIRs);
[[nodiscard]] std::unique_ptr<TextureAsset> importTexture2D(const parser::TextureIR& srcTextureIR,
                                                            eTextureFormat dstFormat,
                                                            bool bGenerateMipmap);
[[nodiscard]] std::unique_ptr<TextureAsset> importTextureCubeMap(const std::string& nameStr,
                                                                 const parser::TextureIR& srcPosX,
                                                                 const parser::TextureIR& srcNegX,
                                                                 const parser::TextureIR& srcPosY,
                                                                 const parser::TextureIR& srcNegY,
                                                                 const parser::TextureIR& srcPosZ,
                                                                 const parser::TextureIR& srcNegZ,
                                                                 eTextureFormat dstFormat,
                                                                 bool bGenerateMipmap);
[[nodiscard]] std::unique_ptr<ShaderAsset> importShader(const parser::ShaderIR& srcShaderIR,
                                                        eShaderStage stage,
                                                        uint8_t compatibleMaterialTypes);

} // namespace importer

} // namespace ho