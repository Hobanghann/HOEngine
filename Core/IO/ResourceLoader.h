#pragma once

#include "ModelIR.h"
#include "ResourceImporter.h"

class aiMaterial;
class aiMaterial;
class aiAnimation;

namespace ho
{
struct ModelImportContext;
struct ModelIR;
struct MeshIR;
struct MaterialIR;
struct SkeletonIR;
struct AnimationIR;
struct SkinIR;
struct TextureIR;
struct ShaderIR;

class ResourceLoader
{
  public:
    [[nodiscard]] static std::unique_ptr<const ModelIR> LoadModel(const std::string& nameStr,
                                                                  const Path& path,
                                                                  bool bMakeStatic,
                                                                  bool bConvertToLeftHanded);

    [[nodiscard]] static std::unique_ptr<const TextureIR> LoadTexture(const std::string& nameStr, const Path& path);

    [[nodiscard]] static std::unique_ptr<const ShaderIR> LoadShader(const std::string& nameStr, const Path& path);

  private:
    [[nodiscard]] static std::unique_ptr<const TextureIR> loadEmbeddedTexture(const aiTexture& assimpTexture);
    [[nodiscard]] static std::unique_ptr<const MaterialIR> loadMaterial(
        const std::string& nameStr,
        const aiMaterial& assimpMaterial,
        aiTexture** pAssimpEmbTextures,
        const Path& parent_path,
        std::vector<std::unique_ptr<const TextureIR>>* pOutTextureIRs); // this parameter for texture loading
    [[nodiscard]] static std::unique_ptr<const SkeletonIR> loadSkeleton(const std::string& nameStr,
                                                                        const ModelImportContext& importContext);
    [[nodiscard]] static std::unique_ptr<const MeshIR> loadMesh(
        const std::string& nameStr,
        const ModelImportContext& importContext,
        const SkeletonIR& skeletonIR,
        const std::vector<std::unique_ptr<const MaterialIR>>& materialIRs);
    [[nodiscard]] static std::unique_ptr<const AnimationIR> loadAnimation(const std::string& nameStr,
                                                                          const aiAnimation& AssimpAnim,
                                                                          const SkeletonIR& skeletonIR);
    [[nodiscard]] static std::unique_ptr<SkinIR> loadSkin(const std::string& nameStr,
                                                          const ModelImportContext& importContext,
                                                          const SkeletonIR& skeletonIR);
};
} // namespace ho