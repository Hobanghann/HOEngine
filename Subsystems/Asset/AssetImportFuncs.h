#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AssetDefs.h"
#include "Subsystems/Object/ObjectHandle.h"

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

struct RStaticMeshAsset;
struct RTextureAsset;
struct RMaterialAsset;
struct RShaderAsset;
struct RMorphTargetsAsset;
struct RMorphAnimationAsset;

namespace importer
{
[[nodiscard]] bool importStaticMesh(ObjectHandle<RStaticMeshAsset> hOutAsset,
                                    const parser::MeshIR& srcMeshIR,
                                    const std::vector<std::unique_ptr<const parser::MaterialIR>>& pMaterialIRs,
                                    const parser::SkeletonIR& srcSkeletonIR,
                                    const parser::SkinIR& srcSkinIR);
[[nodiscard]] bool importMorphTargets(ObjectHandle<RMorphTargetsAsset> hOutAsset, const parser::MeshIR& srcMeshIR);
[[nodiscard]] bool importMorphAnimation(ObjectHandle<RMorphAnimationAsset> hOutAsset,
                                        const parser::AnimationIR& srcAnimIR);
[[nodiscard]] bool importMaterial(ObjectHandle<RMaterialAsset> hOutAsset,
                                  const parser::MaterialIR& srcMaterialIR,
                                  eMaterialAssetType type);
[[nodiscard]] bool importTexturedMaterial(ObjectHandle<RMaterialAsset> hOutAsset,
                                          const parser::MaterialIR& srcMaterialIR,
                                          eMaterialAssetType type,
                                          const std::vector<std::unique_ptr<const parser::TextureIR>>& pSrcTextureIRs);
[[nodiscard]] bool importTexture2D(ObjectHandle<RTextureAsset> hOutAsset,
                                   const parser::TextureIR& srcTextureIR,
                                   eTextureFormat dstFormat,
                                   bool bGenerateMipmap);
[[nodiscard]] bool importTextureCubemap(ObjectHandle<RTextureAsset> hOutAsset,
                                        const std::string& nameStr,
                                        const parser::TextureIR& srcPosX,
                                        const parser::TextureIR& srcNegX,
                                        const parser::TextureIR& srcPosY,
                                        const parser::TextureIR& srcNegY,
                                        const parser::TextureIR& srcPosZ,
                                        const parser::TextureIR& srcNegZ,
                                        eTextureFormat dstFormat,
                                        bool bGenerateMipmap);
[[nodiscard]] bool importShader(ObjectHandle<RShaderAsset> hOutAsset,
                                std::string* pOutErrorMsg,
                                const parser::ShaderIR& srcShaderIR,
                                eShaderStage stage,
                                eMaterialAssetType compatibleMaterialType);

} // namespace importer

} // namespace ho