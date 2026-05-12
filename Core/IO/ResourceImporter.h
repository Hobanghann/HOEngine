#pragma once

#include <assimp/Importer.hpp>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>

struct aiScene;
class aiNode;
class aiTexture;

namespace ho
{
class Path;
class Image;

struct ModelImportContext
{
    const aiScene* pAssimpScene = nullptr;
    std::deque<aiNode*> pFlattedScene;
    std::unordered_map<aiNode*, int32_t> NodeToIndex;
    Assimp::Importer Importer;
};

class ResourceImporter final
{
    friend class ResourceLoader;

  public:
    ResourceImporter() = default;

    [[nodiscard]] static std::unique_ptr<Image> ImportImage(const Path& path, uint32_t desiredChannels = 0);
    [[nodiscard]] static std::unique_ptr<ModelImportContext> ImportModel(const Path& path,
                                                                         bool bMakeStatic,
                                                                         bool bConvertToLeftHanded);

  private:
    static void topologicalSortRecursive(aiNode& root, std::deque<aiNode*>* pOutFlattedScene);
};

} // namespace ho