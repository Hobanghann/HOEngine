#include "ResourceImporter.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image.h>

#include "Image.h"
#include "Path.h"

namespace ho
{
std::unique_ptr<Image> ResourceImporter::ImportImage(const Path& path)
{
    Image::eFormat format = Image::eFormat::RGBA32F;
    int32_t width = 0;
    int32_t height = 0;
    int32_t numColorChannels = 0;

    std::unique_ptr<Image> upImg = nullptr;

    if (stbi_is_hdr(path.ToString().c_str()))
    {
        float* pStbiBitmap = stbi_loadf(path.ToString().c_str(), &width, &height, &numColorChannels, 0);

        if (pStbiBitmap == nullptr)
        {
            return nullptr;
        }

        switch (numColorChannels)
        {
            case 1:
                format = Image::eFormat::R32F;
                break;
            case 2:
                format = Image::eFormat::RG32F;
                break;
            case 3:
                format = Image::eFormat::RGB32F;
                break;
            case 4:
                format = Image::eFormat::RGBA32F;
                break;
            default:
                stbi_image_free(pStbiBitmap);
                return nullptr;
        }
        upImg = std::make_unique<Image>(
            path, path.GetFileName().ToString(), format, width, height, reinterpret_cast<const uint8_t*>(pStbiBitmap));
        stbi_image_free(pStbiBitmap);
    }
    else
    {
        std::uint8_t* pStbiBitmap =
            reinterpret_cast<std::uint8_t*>(stbi_load(path.ToString().c_str(), &width, &height, &numColorChannels, 0));

        if (pStbiBitmap == nullptr)
        {
            return nullptr;
        }

        switch (numColorChannels)
        {
            case 1:
                format = Image::eFormat::R8;
                break;
            case 2:
                format = Image::eFormat::RG8;
                break;
            case 3:
                format = Image::eFormat::RGB8;
                break;
            case 4:
                format = Image::eFormat::RGBA8;
                break;
            default:
                stbi_image_free(pStbiBitmap);
                return nullptr;
        }
        upImg = std::make_unique<Image>(path, path.GetFileName().ToString(), format, width, height, pStbiBitmap);
        stbi_image_free(pStbiBitmap);
    }

    return upImg;
}

std::unique_ptr<ModelImportContext> ResourceImporter::ImportModel(const Path& path,
                                                                  bool bMakeStatic,
                                                                  bool bConvertToLeftHanded)
{
    std::unique_ptr<ModelImportContext> upImportContext = std::make_unique<ModelImportContext>();

    unsigned int importFlag = static_cast<unsigned int>(
        aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals | aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials |
        aiProcess_FindInstances | aiProcess_OptimizeMeshes | aiProcess_LimitBoneWeights |
        aiProcess_ValidateDataStructure | aiProcess_FindDegenerates | aiProcess_FindInvalidData |
        aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes);
    if (bMakeStatic)
    {
        importFlag |= static_cast<unsigned int>(aiProcess_PreTransformVertices);
    }
    else
    {
        importFlag |= static_cast<unsigned int>(aiProcess_OptimizeGraph);
    }
    if (bConvertToLeftHanded)
    {
        importFlag |= static_cast<unsigned int>(aiProcess_ConvertToLeftHanded);
    }

    upImportContext->Importer.ReadFile(path.ToString(), importFlag);
    upImportContext->pAssimpScene = upImportContext->Importer.GetScene();
    if (!upImportContext->pAssimpScene)
    {
        return nullptr;
    }
    topologicalSortRecursive(*upImportContext->pAssimpScene->mRootNode, &upImportContext->pFlattedScene);
    for (int32_t i = 0; i < static_cast<int32_t>(upImportContext->pFlattedScene.size()); ++i)
    {
        upImportContext->NodeToIndex[upImportContext->pFlattedScene[i]] = i;
    }
    return upImportContext;
}

void ResourceImporter::topologicalSortRecursive(aiNode& root, std::deque<aiNode*>* outFlattedScene)
{
    for (int32_t i = 0; i < static_cast<int32_t>(root.mNumChildren); ++i)
    {
        topologicalSortRecursive(*(root.mChildren[i]), outFlattedScene);
    }
    outFlattedScene->push_front(&root);
}

} // namespace ho