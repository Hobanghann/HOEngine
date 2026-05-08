#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ho
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
    std::unique_ptr<const MeshIR> upMeshIR;
    std::unique_ptr<const SkeletonIR> upSkeletonIR;
    std::unique_ptr<const SkinIR> upSkinIR;
    std::vector<std::unique_ptr<const AnimationIR>> upAnimationIRs;
    std::vector<std::unique_ptr<const MaterialIR>> upMaterialIRs;
    std::vector<std::unique_ptr<const TextureIR>> upTextureIRs;
};
} // namespace ho