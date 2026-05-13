#pragma once

#include <memory>
#include <string>
#include <vector>

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

struct ModelIR
{
    std::string NameStr;
    std::unique_ptr<const MeshIR> pMeshIR;
    std::unique_ptr<const SkeletonIR> pSkeletonIR;
    std::unique_ptr<const SkinIR> pSkinIR;
    std::vector<std::unique_ptr<const AnimationIR>> pAnimationIRs;
    std::vector<std::unique_ptr<const MaterialIR>> pMaterialIRs;
    std::vector<std::unique_ptr<const TextureIR>> pTextureIRs;
};
} // namespace parser
} // namespace ho