#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AnimationIR.h"
#include "MaterialIR.h"
#include "MeshIR.h"
#include "Path.h"
#include "SkeletonIR.h"
#include "SkinIR.h"
#include "TextureIR.h"

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
    Path ResourcePath = Path(std::string(""));
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