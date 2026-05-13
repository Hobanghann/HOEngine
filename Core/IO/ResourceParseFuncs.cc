#include "ResourceParseFuncs.h"

#include <assimp/Importer.hpp>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stb_image.h>

#include "AnimationIR.h"
#include "Core/Math/Transform3D.h"
#include "Image.h"
#include "MaterialIR.h"
#include "MeshIR.h"
#include "ModelIR.h"
#include "Path.h"
#include "ShaderIR.h"
#include "SkeletonIR.h"
#include "SkinIR.h"
#include "TextureIR.h"

namespace ho
{
namespace parser
{

struct ModelParsingContext
{
    const aiScene* pAssimpScene = nullptr;
    std::deque<aiNode*> pFlattedScene;
    std::unordered_map<aiNode*, int32_t> NodeToIndex;
    Assimp::Importer Importer;
};

// ===========================================================================
//  Private Function Declarations
// ===========================================================================

[[nodiscard]] static std::unique_ptr<Image> readImageFile(const Path& path, int32_t desiredChannels = 0);
[[nodiscard]] static std::unique_ptr<ModelParsingContext> readModelFile(const Path& path,
                                                                        bool bMakeStatic,
                                                                        bool bConvertToLeftHanded);
static void topologicalSortSceneRecursive(aiNode& root, std::deque<aiNode*>* pOutFlattedScene);

[[nodiscard]] static std::unique_ptr<const TextureIR> parseEmbeddedTexture(const aiTexture& assimpTexture);
[[nodiscard]] static std::unique_ptr<const MaterialIR> parseMaterial(
    const std::string& nameStr,
    const aiMaterial& assimpMaterial,
    aiTexture** pAssimpEmbTextures,
    const Path& parentPath,
    std::vector<std::unique_ptr<const TextureIR>>* pOutTextureIRs); // this parameter for texture loading
[[nodiscard]] static std::unique_ptr<const SkeletonIR> parseSkeleton(const std::string& nameStr,
                                                                     const ModelParsingContext& parsingContext);
[[nodiscard]] static std::unique_ptr<const MeshIR> parseMesh(
    const std::string& nameStr,
    const ModelParsingContext& parsingContext,
    const SkeletonIR& skeletonIR,
    const std::vector<std::unique_ptr<const MaterialIR>>& materialIRs);
[[nodiscard]] static std::unique_ptr<const AnimationIR> parseAnimation(const std::string& nameStr,
                                                                       const aiAnimation& assimpAnim,
                                                                       const SkeletonIR& skeletonIR);
[[nodiscard]] static std::unique_ptr<SkinIR> parseSkin(const std::string& nameStr,
                                                       const ModelParsingContext& parsingContext,
                                                       const SkeletonIR& skeletonIR);

[[nodiscard]] static std::string extractFileName(const std::string& fileNameWithExtStr);

// ===========================================================================
//  Public Function Definitions
// ===========================================================================

std::unique_ptr<const ModelIR> parseModelFile(const std::string& nameStr,
                                              const Path& path,
                                              bool bMakeStatic,
                                              bool bConvertToLeftHanded)
{
    const std::unique_ptr<ModelParsingContext> pParsingContext = readModelFile(path, bMakeStatic, bConvertToLeftHanded);
    if (!pParsingContext)
    {
        return nullptr;
    }

    const Path parentPath = path.GetParentPath();
    const std::string fileNameStr = path.GetFileName().ToString();

    std::unique_ptr<ModelIR> pModelIR = std::make_unique<ModelIR>();

    pModelIR->NameStr = nameStr;

    // Load materials
    std::unordered_map<std::string, int32_t> matNameCount; // for handling duplicated name
    matNameCount.reserve(pParsingContext->pAssimpScene->mNumMaterials);
    for (int32_t mi = 0; mi < static_cast<int32_t>(pParsingContext->pAssimpScene->mNumMaterials); ++mi)
    {
        const aiMaterial* pAssimpMaterial = pParsingContext->pAssimpScene->mMaterials[mi];
        HO_ASSERT(pAssimpMaterial,
                  "Assimp material mapping failed: Mesh refers to an invalid or non-existent material index.");
        aiTexture** pAssimpEmbTextures = pParsingContext->pAssimpScene->mTextures;
        std::string matNameStr;
        if (pAssimpMaterial->GetName().Empty())
        {
            matNameStr = nameStr + "_" + "unnamed_material_" + std::to_string(mi);
        }
        else
        {
            matNameStr = pAssimpMaterial->GetName().C_Str();
            auto it = matNameCount.find(matNameStr);
            if (it == matNameCount.end())
            {
                matNameCount.insert({matNameStr, 1});
            }
            else
            {
                matNameStr += "_" + std::to_string(it->second);
                ++it->second;
            }
        }

        std::unique_ptr<const MaterialIR> pMatIR =
            parseMaterial(matNameStr, *pAssimpMaterial, pAssimpEmbTextures, parentPath, &(pModelIR->pTextureIRs));
        HO_ASSERT(pMatIR, (std::string("Failed to parse material of ") + matNameStr).c_str());

        pModelIR->pMaterialIRs.emplace_back(std::move(pMatIR));
    }

    // Load skeleton
    std::unique_ptr<const SkeletonIR> pSkeletonIR = parseSkeleton(nameStr, *pParsingContext);
    HO_ASSERT(pSkeletonIR, (std::string("Failed to parse skeleton of ") + fileNameStr).c_str());
    pModelIR->pSkeletonIR = std::move(pSkeletonIR);

    // Load mesh
    std::unique_ptr<const MeshIR> pMeshIR =
        parseMesh(nameStr, *pParsingContext, *(pModelIR->pSkeletonIR), pModelIR->pMaterialIRs);
    HO_ASSERT(pMeshIR, (std::string("Failed to parse mesh of ") + fileNameStr).c_str());
    pModelIR->pMeshIR = std::move(pMeshIR);

    // Load animations
    std::unordered_map<std::string, int> animNameCount;
    animNameCount.reserve(pParsingContext->pAssimpScene->mNumAnimations);
    for (int32_t ai = 0; ai < static_cast<int32_t>(pParsingContext->pAssimpScene->mNumAnimations); ++ai)
    {
        const aiAnimation* pAssimpAnim = pParsingContext->pAssimpScene->mAnimations[ai];
        std::string animNameStr;
        if (pAssimpAnim->mName.Empty())
        {
            animNameStr = fileNameStr + "_" + "unnamed_animation_" + std::to_string(ai);
        }
        else
        {
            animNameStr = pAssimpAnim->mName.C_Str();
            auto it = animNameCount.find(animNameStr);
            if (it == animNameCount.end())
            {
                animNameCount.insert({animNameStr, 1});
            }
            else
            {
                animNameStr += "_" + std::to_string(it->second);
                ++it->second;
            }
        }
        std::unique_ptr<const AnimationIR> pAnimIR =
            parseAnimation(animNameStr, *pAssimpAnim, *(pModelIR->pSkeletonIR));
        HO_ASSERT(pAnimIR, (std::string("Failed to parse animation of ") + fileNameStr).c_str());
        pModelIR->pAnimationIRs.emplace_back(std::move(pAnimIR));
    }

    // Load Skin
    std::unique_ptr<const SkinIR> pSkinIR = parseSkin(nameStr, *pParsingContext, *(pModelIR->pSkeletonIR));
    HO_ASSERT(pSkinIR, (std::string("Failed to parse skin of ") + fileNameStr).c_str());
    pModelIR->pSkinIR = std::move(pSkinIR);

    return pModelIR;
}

std::unique_ptr<const TextureIR> parseTextureFile(const std::string& nameStr, const Path& path)
{
    const int32_t desiredChannel = 4; // for BCn compression.
    const std::unique_ptr<Image> pImg = readImageFile(path, desiredChannel);

    if (!pImg)
    {
        return nullptr;
    }

    return std::make_unique<TextureIR>(std::string(nameStr), std::move(*pImg));
}

std::unique_ptr<const ShaderIR> parseShaderFile(const std::string& nameStr, const Path& path)
{
    std::string shaderSourceStr;

    std::ifstream shaderStream(path.ToString(), std::ios::binary | std::ios::ate);

    if (!shaderStream.is_open())
    {
        return nullptr;
    }

    const std::streamsize size = shaderStream.tellg();
    shaderStream.seekg(0, std::ios::beg);

    shaderSourceStr.resize(static_cast<size_t>(size));

    if (!shaderStream.read(shaderSourceStr.data(), size))
    {
        return nullptr;
    }

    shaderStream.close();

    std::unique_ptr<ShaderIR> pShaderIR;
    pShaderIR->NameStr = nameStr;
    pShaderIR->SourceStr = std::move(shaderSourceStr);

    return pShaderIR;
}

// ===========================================================================
//  Private Function Definitions
// ===========================================================================

std::unique_ptr<Image> readImageFile(const Path& path, int32_t desiredChannels)
{
    Image::eFormat format = Image::eFormat::RGBA32F;
    int32_t width = 0;
    int32_t height = 0;
    int32_t numColorChannels = 0;

    std::unique_ptr<Image> pImg = nullptr;

    if (stbi_is_hdr(path.ToString().c_str()) != 0)
    {
        float* pStbiBitmap = stbi_loadf(path.ToString().c_str(), &width, &height, &numColorChannels, desiredChannels);

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
        pImg = std::make_unique<Image>(path,
                                       path.GetFileName().ToString(),
                                       format,
                                       width,
                                       height,
                                       numColorChannels,
                                       reinterpret_cast<const uint8_t*>(pStbiBitmap));
        stbi_image_free(pStbiBitmap);
    }
    else
    {
        std::uint8_t* pStbiBitmap = reinterpret_cast<std::uint8_t*>(
            stbi_load(path.ToString().c_str(), &width, &height, &numColorChannels, desiredChannels));

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
        pImg = std::make_unique<Image>(
            path, path.GetFileName().ToString(), format, width, height, numColorChannels, pStbiBitmap);
        stbi_image_free(pStbiBitmap);
    }

    return pImg;
}

std::unique_ptr<ModelParsingContext> readModelFile(const Path& path, bool bMakeStatic, bool bConvertToLeftHanded)
{
    std::unique_ptr<ModelParsingContext> pParsingContext = std::make_unique<ModelParsingContext>();

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

    pParsingContext->Importer.ReadFile(path.ToString(), importFlag);
    pParsingContext->pAssimpScene = pParsingContext->Importer.GetScene();
    if (!pParsingContext->pAssimpScene)
    {
        return nullptr;
    }
    topologicalSortSceneRecursive(*pParsingContext->pAssimpScene->mRootNode, &pParsingContext->pFlattedScene);
    for (int32_t i = 0; i < static_cast<int32_t>(pParsingContext->pFlattedScene.size()); ++i)
    {
        pParsingContext->NodeToIndex[pParsingContext->pFlattedScene[i]] = i;
    }
    return pParsingContext;
}

void topologicalSortSceneRecursive(aiNode& root, std::deque<aiNode*>* outFlattedScene)
{
    for (int32_t i = 0; i < static_cast<int32_t>(root.mNumChildren); ++i)
    {
        topologicalSortSceneRecursive(*(root.mChildren[i]), outFlattedScene);
    }
    outFlattedScene->push_front(&root);
}

std::unique_ptr<const TextureIR> parseEmbeddedTexture(const aiTexture& assimpTexture)
{
    int32_t width = 0;
    int32_t height = 0;
    int32_t numColorChannels = 0;
    const int32_t desiredChannel = 4; // for BCn compression.

    std::uint8_t* pStbiBitmapLDR = nullptr;
    float* pStbiBitmapHDR = nullptr;
    bool bHDR = false;

    // Compressed texture
    if (assimpTexture.mHeight == 0)
    {
        const stbi_uc* pCompressedData = reinterpret_cast<const stbi_uc*>(assimpTexture.pcData);
        const int compressedSize = static_cast<int>(assimpTexture.mWidth);

        if (stbi_is_hdr_from_memory(pCompressedData, compressedSize))
        {
            bHDR = true;
            pStbiBitmapHDR = stbi_loadf_from_memory(
                pCompressedData, compressedSize, &width, &height, &numColorChannels, desiredChannel);
            if (!pStbiBitmapHDR)
            {
                return nullptr;
            }
        }
        else
        {
            pStbiBitmapLDR = stbi_load_from_memory(
                pCompressedData, compressedSize, &width, &height, &numColorChannels, desiredChannel);
            if (!pStbiBitmapLDR)
            {
                return nullptr;
            }
        }
    }
    // Uncompressed texture
    else
    {
        width = static_cast<int32_t>(assimpTexture.mWidth);
        height = static_cast<int32_t>(assimpTexture.mHeight);
        numColorChannels = 4;

        const int32_t size = width * height * numColorChannels;
        pStbiBitmapLDR = new std::uint8_t[size];

        for (int32_t y = 0; y < height; ++y)
        {
            for (int32_t x = 0; x < width; ++x)
            {
                const aiTexel& assimpTexel = assimpTexture.pcData[y * width + x];
                const int32_t idx = (y * width + x) * numColorChannels;
                pStbiBitmapLDR[idx + 0] = assimpTexel.r;
                pStbiBitmapLDR[idx + 1] = assimpTexel.g;
                pStbiBitmapLDR[idx + 2] = assimpTexel.b;
                pStbiBitmapLDR[idx + 3] = assimpTexel.a;
            }
        }
    }

    Image::eFormat format;
    if (bHDR)
    {
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
            default:
                format = Image::eFormat::RGBA32F;
                break;
        }
    }
    else
    {
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
            default:
                format = Image::eFormat::RGBA8;
                break;
        }
    }

    const std::uint8_t* pFinalBitmap = bHDR ? reinterpret_cast<const std::uint8_t*>(pStbiBitmapHDR) : pStbiBitmapLDR;

    const std::unique_ptr<Image> pImg = std::make_unique<Image>(
        Path(std::string()), assimpTexture.mFilename.C_Str(), format, width, height, numColorChannels, pFinalBitmap);

    if (bHDR)
    {
        stbi_image_free(pStbiBitmapHDR);
    }
    else if (assimpTexture.mHeight == 0)
    {
        stbi_image_free(pStbiBitmapLDR);
    }
    else
    {
        delete[] pStbiBitmapLDR;
    }

    return std::make_unique<TextureIR>(std::string(assimpTexture.mFilename.C_Str()), std::move(*pImg));
}

std::unique_ptr<const MaterialIR> parseMaterial(const std::string& nameStr,
                                                const aiMaterial& assimpMaterial,
                                                aiTexture** pAssimpEmbTextures,
                                                const Path& parentPath,
                                                std::vector<std::unique_ptr<const TextureIR>>* pOutTextureIRs)
{
    std::unique_ptr<MaterialIR> pMaterialIR = std::make_unique<MaterialIR>();
    pMaterialIR->NameStr = nameStr;

    aiColor3D color3D;
    aiColor4D color4D;
    float fVal = 0.0f;
    int iVal = 0;
    aiString aiStr;

    // Legacy / Phong Attributes
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_COLOR_AMBIENT, color3D))
    {
        pMaterialIR->Ambient = Color128(color3D.r, color3D.g, color3D.b);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_COLOR_DIFFUSE, color3D))
    {
        pMaterialIR->Diffuse = Color128(color3D.r, color3D.g, color3D.b);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_COLOR_SPECULAR, color3D))
    {
        pMaterialIR->Specular = Color128(color3D.r, color3D.g, color3D.b);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_SHININESS, fVal))
    {
        pMaterialIR->Shininess = fVal;
    }

    // PBR Core Attributes
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_BASE_COLOR, color4D))
    {
        pMaterialIR->Albedo = Color128(color4D.r, color4D.g, color4D.b, color4D.a);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_METALLIC_FACTOR, fVal))
    {
        pMaterialIR->Metallic = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_ROUGHNESS_FACTOR, fVal))
    {
        pMaterialIR->Roughness = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_REFRACTI, fVal))
    {
        pMaterialIR->IndexOfRefraction = fVal;
    }

    // Global / Transparency Attributes
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_OPACITY, fVal))
    {
        pMaterialIR->Opacity = fVal;
    }

    pMaterialIR->AlphaMode =
        pMaterialIR->Opacity < 1.f ? MaterialIR::eAlphaMode::Blend : MaterialIR::eAlphaMode::Opaque;
    pMaterialIR->AlphaThreshold = 0.5f;

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_GLTF_ALPHAMODE, aiStr))
    {
        const std::string mode(aiStr.C_Str());
        if (mode == "MASK")
        {
            pMaterialIR->AlphaMode = MaterialIR::eAlphaMode::Mask;
        }
        else if (mode == "BLEND")
        {
            pMaterialIR->AlphaMode = MaterialIR::eAlphaMode::Blend;
        }
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_GLTF_ALPHACUTOFF, fVal))
    {
        pMaterialIR->AlphaThreshold = fVal;
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_BLEND_FUNC, iVal))
    {
        if (static_cast<aiBlendMode>(iVal) == aiBlendMode::aiBlendMode_Default)
        {
            pMaterialIR->BlendMode = MaterialIR::eAlphaBlendMode::Default;
        }
        if (static_cast<aiBlendMode>(iVal) == aiBlendMode::aiBlendMode_Additive)
        {
            pMaterialIR->BlendMode = MaterialIR::eAlphaBlendMode::Additive;
        }
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_COLOR_EMISSIVE, color3D))
    {
        pMaterialIR->Emissive = Color128(color3D.r, color3D.g, color3D.b);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_EMISSIVE_INTENSITY, fVal))
    {
        pMaterialIR->EmissiveIntensity = fVal;
    }

    // Advanced PBR Attributes
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_TRANSMISSION_FACTOR, fVal))
    {
        pMaterialIR->TransmissionFactor = fVal;
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_CLEARCOAT_FACTOR, fVal))
    {
        pMaterialIR->ClearcoatFactor = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, fVal))
    {
        pMaterialIR->ClearcoatRoughness = fVal;
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_SHEEN_COLOR_FACTOR, color3D))
    {
        pMaterialIR->SheenColor = Color128(color3D.r, color3D.g, color3D.b);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_SHEEN_ROUGHNESS_FACTOR, fVal))
    {
        pMaterialIR->SheenRoughnessFactor = fVal;
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_VOLUME_THICKNESS_FACTOR, fVal))
    {
        pMaterialIR->VolumeThicknessFactor = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_VOLUME_ATTENUATION_DISTANCE, fVal))
    {
        pMaterialIR->VolumeAttenuationDist = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_VOLUME_ATTENUATION_COLOR, color3D))
    {
        pMaterialIR->VolumeAttenuationColor = Color128(color3D.r, color3D.g, color3D.b);
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_ANISOTROPY_FACTOR, fVal))
    {
        pMaterialIR->AnisotropyFactor = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_ANISOTROPY_ROTATION, fVal))
    {
        pMaterialIR->AnisotropyRotation = fVal;
    }

    // Texture Strength / Scale
    if (AI_SUCCESS == assimpMaterial.Get("$mat.gltf.normalScale", 0, 0, fVal))
    {
        pMaterialIR->NormalScale = fVal;
    }
    else if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_BUMPSCALING, fVal))
    {
        pMaterialIR->ParallaxScale = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get("$mat.gltf.occlusionStrength", 0, 0, fVal))
    {
        pMaterialIR->OcclusionStrength = fVal;
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_BUMPSCALING, fVal))
    {
        pMaterialIR->ParallaxScale = fVal;
    }

    // Render States
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_ENABLE_WIREFRAME, iVal))
    {
        pMaterialIR->bWireframe = (iVal != 0);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_TWOSIDED, iVal))
    {
        pMaterialIR->bBackfaceCulling = (iVal == 0);
    }

    // Texture Load Helper
    auto loadTextureToMaterial = [&](aiTextureType aiType, MaterialIR::eTextureUsage engineType)
    {
        if (pMaterialIR->TextureIRIndices[static_cast<int32_t>(engineType)] != -1)
        {
            return;
        }

        aiString texFileNameAiStr;
        unsigned int uv = 0;
        if (AI_SUCCESS != assimpMaterial.GetTexture(aiType, 0, &texFileNameAiStr, nullptr, &uv))
        {
            return;
        }
        std::string texFileNameStr = texFileNameAiStr.C_Str();

        int32_t embTexIdx = -1;
        const bool bEmbedded = !texFileNameStr.empty() && texFileNameStr[0] == '*';
        if (bEmbedded)
        {
            embTexIdx = std::atoi(&(texFileNameStr.c_str()[1]));
            texFileNameStr = pAssimpEmbTextures[embTexIdx]->mFilename.C_Str();
        }

        std::string texNameStr = texFileNameStr;
        if (!bEmbedded)
        {
            texNameStr = extractFileName(texFileNameStr);
        }

        int32_t texIRIndex = -1;
        for (int32_t i = 0; i < static_cast<int32_t>(pOutTextureIRs->size()); ++i)
        {
            if ((*pOutTextureIRs)[i]->NameStr == texNameStr)
            {
                texIRIndex = i;
            }
        }

        if (texIRIndex == -1)
        {
            std::unique_ptr<const TextureIR> pTextureIR;
            if (bEmbedded)
            {
                pTextureIR = parseEmbeddedTexture(*pAssimpEmbTextures[embTexIdx]);
            }
            else if (!texFileNameStr.empty())
            {
                const Path texAbsPath = parentPath / Path(texFileNameStr);
                pTextureIR = parseTextureFile(texNameStr, texAbsPath);
            }

            if (pTextureIR)
            {
                texIRIndex = static_cast<int32_t>(pOutTextureIRs->size());
                pOutTextureIRs->emplace_back(std::move(pTextureIR));
            }
        }

        pMaterialIR->TextureIRIndices[static_cast<int32_t>(engineType)] = texIRIndex;
        pMaterialIR->UVChannels[static_cast<int32_t>(engineType)] = static_cast<int32_t>(uv);

        // UV Transform Matrix
        aiUVTransform aiTransform;
        if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_UVTRANSFORM(aiType, 0), aiTransform))
        {
            Matrix3x3 t = Matrix3x3::sIdentity;
            t.Data[0][2] = aiTransform.mTranslation.x;
            t.Data[1][2] = aiTransform.mTranslation.y;

            Matrix3x3 r = Matrix3x3::sIdentity;
            float sin = 0.f;
            float cos = 0.f;
            math::SinCos(&sin, &cos, aiTransform.mRotation);
            r.Data[0][0] = cos;
            r.Data[0][1] = -sin;
            r.Data[1][0] = sin;
            r.Data[1][1] = cos;

            Matrix3x3 s = Matrix3x3::sIdentity;
            s.Data[0][0] = aiTransform.mScaling.x;
            s.Data[1][1] = aiTransform.mScaling.y;

            pMaterialIR->UVTransforms[static_cast<int32_t>(engineType)] = t * r * s;
        }
    };

    // Load textures
    // Phong / legacy
    loadTextureToMaterial(aiTextureType_DIFFUSE, MaterialIR::eTextureUsage::Diffuse);
    loadTextureToMaterial(aiTextureType_SPECULAR, MaterialIR::eTextureUsage::Specular);
    loadTextureToMaterial(aiTextureType_SHININESS, MaterialIR::eTextureUsage::Shininess);
    loadTextureToMaterial(aiTextureType_OPACITY, MaterialIR::eTextureUsage::Opacity);

    // Geometry
    loadTextureToMaterial(aiTextureType_HEIGHT, MaterialIR::eTextureUsage::Height);
    loadTextureToMaterial(aiTextureType_NORMALS, MaterialIR::eTextureUsage::Normal);

    // PBR Core
    loadTextureToMaterial(aiTextureType_BASE_COLOR, MaterialIR::eTextureUsage::Albedo);
    loadTextureToMaterial(aiTextureType_EMISSION_COLOR, MaterialIR::eTextureUsage::Emissive);
    loadTextureToMaterial(aiTextureType_EMISSIVE, MaterialIR::eTextureUsage::Emissive);
    loadTextureToMaterial(aiTextureType_METALNESS, MaterialIR::eTextureUsage::Metallic);
    loadTextureToMaterial(aiTextureType_DIFFUSE_ROUGHNESS, MaterialIR::eTextureUsage::Roughness);
    loadTextureToMaterial(aiTextureType_GLTF_METALLIC_ROUGHNESS, MaterialIR::eTextureUsage::MetallicRoughness);
    loadTextureToMaterial(aiTextureType_AMBIENT_OCCLUSION, MaterialIR::eTextureUsage::AmbientOcclusion);
    loadTextureToMaterial(aiTextureType_LIGHTMAP, MaterialIR::eTextureUsage::AmbientOcclusion);

    // PBR Advanced
    loadTextureToMaterial(aiTextureType_SHEEN, MaterialIR::eTextureUsage::Sheen);
    loadTextureToMaterial(aiTextureType_CLEARCOAT, MaterialIR::eTextureUsage::Clearcoat);
    loadTextureToMaterial(aiTextureType_TRANSMISSION, MaterialIR::eTextureUsage::Transmission);
    loadTextureToMaterial(aiTextureType_ANISOTROPY, MaterialIR::eTextureUsage::Anisotropy);

    return pMaterialIR;
}

std::unique_ptr<const SkeletonIR> parseSkeleton(const std::string& nameStr, const ModelParsingContext& parsingContext)
{
    std::string skeletonNameStr = nameStr;

    std::vector<std::string> boneNameStrs;
    std::vector<Transform3D> localTransforms;
    std::vector<int32_t> parents;
    std::unordered_map<std::string, int32_t> boneNameToIndex;

    boneNameStrs.reserve(parsingContext.pFlattedScene.size());
    localTransforms.reserve(parsingContext.pFlattedScene.size());
    parents.reserve(parsingContext.pFlattedScene.size());
    boneNameToIndex.reserve(parsingContext.pFlattedScene.size());

    std::unordered_map<std::string, int32_t> boneNameCount; // for handling duplicated bone name
    boneNameCount.reserve(parsingContext.pFlattedScene.size());
    for (int32_t i = 0; i < static_cast<int32_t>(parsingContext.pFlattedScene.size()); ++i)
    {
        const aiNode* pAssimpNode = parsingContext.pFlattedScene[i];

        // bone name
        std::string boneNameStr;

        if (pAssimpNode->mName.Empty())
        {
            boneNameStr = nameStr + "_" + "unnamed_bone_" + std::to_string(i);
        }
        else
        {
            boneNameStr = pAssimpNode->mName.C_Str();
            auto it = boneNameCount.find(boneNameStr);
            if (it == boneNameCount.end())
            {
                boneNameCount.insert({boneNameStr, 1});
            }
            else
            {
                boneNameStr += "_" + std::to_string(it->second);
                ++it->second;
            }
        }
        boneNameStrs.push_back(boneNameStr);

        // bone local transform
        const aiMatrix4x4 aim = pAssimpNode->mTransformation;
        const Matrix4x4 m(Vector4(aim.a1, aim.b1, aim.c1, aim.d1),
                          Vector4(aim.a2, aim.b2, aim.c2, aim.d2),
                          Vector4(aim.a3, aim.b3, aim.c3, aim.d3),
                          Vector4(aim.a4, aim.b4, aim.c4, aim.d4));
        localTransforms.emplace_back(Transform3D(m));

        boneNameToIndex[boneNameStr] = i;

        // parent bone
        if (pAssimpNode->mParent)
        {
            auto it = parsingContext.NodeToIndex.find(pAssimpNode->mParent);
            if (it == parsingContext.NodeToIndex.end())
            {
                return nullptr;
            }
            parents.push_back(it->second);
        }
        else
        {
            parents.push_back(-1); // root
        }
    }

    return std::make_unique<SkeletonIR>(
        std::move(skeletonNameStr), std::move(boneNameStrs), std::move(localTransforms), std::move(parents));
}

std::unique_ptr<const MeshIR> parseMesh(const std::string& nameStr,
                                        const ModelParsingContext& parsingContext,
                                        const SkeletonIR& skeletonIR,
                                        const std::vector<std::unique_ptr<const MaterialIR>>& materialIRs)
{
    std::string meshNameStr = nameStr;

    std::vector<MeshIR::SubMesh> subMeshes;
    subMeshes.reserve(parsingContext.pAssimpScene->mNumMeshes);

    // submeshes
    std::unordered_map<std::string, int> subMeshNameCount; // for handling duplicated submesh name
    subMeshNameCount.reserve(parsingContext.pAssimpScene->mNumMeshes);
    for (int32_t mi = 0; mi < static_cast<int32_t>(parsingContext.pAssimpScene->mNumMeshes); ++mi)
    {
        const aiMesh* pAssimpMesh = parsingContext.pAssimpScene->mMeshes[mi];
        if (!pAssimpMesh)
        {
            continue;
        }

        // name
        std::string subMeshNameStr;
        if (pAssimpMesh->mName.Empty())
        {
            subMeshNameStr = nameStr + "_" + "unnamed_submesh_" + std::to_string(mi);
        }
        else
        {
            subMeshNameStr = pAssimpMesh->mName.C_Str();
            auto it = subMeshNameCount.find(subMeshNameStr);
            if (it == subMeshNameCount.end())
            {
                subMeshNameCount.insert({subMeshNameStr, 1});
            }
            else
            {
                subMeshNameStr += "_" + std::to_string(it->second);
                ++it->second;
            }
        }

        // primitive type
        MeshIR::ePrimitiveType primitiveType = MeshIR::ePrimitiveType::Triangle;
        switch (pAssimpMesh->mPrimitiveTypes)
        {
            case aiPrimitiveType_POINT:
                primitiveType = MeshIR::ePrimitiveType::Point;
                break;
            case aiPrimitiveType_LINE:
                primitiveType = MeshIR::ePrimitiveType::Line;
                break;
            case aiPrimitiveType_TRIANGLE:
                primitiveType = MeshIR::ePrimitiveType::Triangle;
                break;
            default:
                HO_ASSERT(false, "Invalid Assimp PrimitiveType.");
        }

        std::vector<Vector3> positions;
        positions.reserve(pAssimpMesh->mNumVertices);
        std::vector<Vector3> normals;
        normals.reserve(pAssimpMesh->mNumVertices);
        std::vector<Vector4> tangents;
        tangents.reserve(pAssimpMesh->mNumVertices);
        std::vector<std::array<Vector2, MeshIR::sMaxUVChannel>> uvs;
        uvs.resize(pAssimpMesh->mNumVertices);
        std::vector<std::array<Color32, MeshIR::sMaxColorChannel>> colors;
        colors.resize(pAssimpMesh->mNumVertices);
        std::vector<MeshIR::BoneWeight> boneWeights;
        boneWeights.reserve(pAssimpMesh->mNumVertices);

        std::vector<uint32_t> indices;
        indices.reserve(pAssimpMesh->mNumFaces * 3);
        std::vector<MeshIR::MorphTarget> morphTargets;
        morphTargets.reserve(pAssimpMesh->mNumAnimMeshes);

        bool bHasPos = pAssimpMesh->HasPositions();
        bool bHasNormal = pAssimpMesh->HasNormals();
        bool bHasTangent = pAssimpMesh->HasTangentsAndBitangents();

        for (int32_t vi = 0; vi < static_cast<int32_t>(pAssimpMesh->mNumVertices); ++vi)
        {
            // Positions
            if (bHasPos)
            {
                const aiVector3D& pos = pAssimpMesh->mVertices[vi];
                positions.emplace_back(pos.x, pos.y, pos.z);
            }

            // Normals
            if (bHasNormal)
            {
                const aiVector3D& normal = pAssimpMesh->mNormals[vi];
                normals.emplace_back(normal.x, normal.y, normal.z);
            }

            // Tangents (handedness in w component)
            if (bHasTangent)
            {
                const aiVector3D& aiTangent = pAssimpMesh->mTangents[vi];
                const aiVector3D& aiBitangent = pAssimpMesh->mBitangents[vi];
                const Vector3 tangent(aiTangent.x, aiTangent.y, aiTangent.z);
                const Vector3 bitangent(aiBitangent.x, aiBitangent.y, aiBitangent.z);

                // calcuate handedness
                float handedness = 1.0f;
                if (bHasNormal)
                {
                    const Vector3& normal = normals[vi];
                    handedness = (normal.Cross(tangent).Dot(bitangent) > 0.0f) ? 1.0f : -1.0f;
                }
                tangents.emplace_back(tangent.X, tangent.Y, tangent.Z, handedness);
            }
        }

        // UVs
        for (int32_t vi = 0; vi < static_cast<int32_t>(pAssimpMesh->mNumVertices); ++vi)
        {
            for (int32_t ui = 0; ui < MeshIR::sMaxUVChannel; ++ui)
            {
                Vector2 uv = Vector2(-1.f, -1.f);
                ;
                if (pAssimpMesh->HasTextureCoords(static_cast<unsigned int>(ui)))
                {
                    const aiVector3D assimpUv = pAssimpMesh->mTextureCoords[ui][vi];
                    uv = Vector2(assimpUv.x, assimpUv.y);
                }

                uvs[vi][ui] = uv;
            }
        }

        // Colors
        for (int32_t vi = 0; vi < static_cast<int32_t>(pAssimpMesh->mNumVertices); ++vi)
        {
            for (int32_t ci = 0; ci < MeshIR::sMaxColorChannel; ++ci)
            {
                Color128 color = Color128(Color32::sMagenta);
                if (pAssimpMesh->HasVertexColors(static_cast<unsigned int>(ci)))
                {
                    const aiColor4D& assimpColor = pAssimpMesh->mColors[ci][vi];
                    color = Color128(assimpColor.r, assimpColor.g, assimpColor.b, assimpColor.a);
                }
                colors[vi][ci] = Color32(color);
            }
        }

        // Bone weights

        if (pAssimpMesh->HasBones())
        {
            // Collect bone weights from assimp
            struct TempBoneWeight
            {
                int32_t BoneIndex;
                float Weight;
            };

            std::vector<std::vector<TempBoneWeight>> tempBoneWeights;
            tempBoneWeights.resize(pAssimpMesh->mNumVertices);
            for (int32_t bi = 0; bi < static_cast<int32_t>(pAssimpMesh->mNumBones); ++bi)
            {
                const aiBone* pAssimpBone = pAssimpMesh->mBones[bi];
                const int32_t boneIndex = skeletonIR.BoneNameToIndexMap.at(pAssimpBone->mName.C_Str());

                for (int32_t wi = 0; wi < static_cast<int32_t>(pAssimpBone->mNumWeights); ++wi)
                {
                    const aiVertexWeight& aiWeight = pAssimpBone->mWeights[wi];
                    HO_ASSERT(aiWeight.mVertexId < tempBoneWeights.size(), "SkeletonIR does not match this mesh.");
                    tempBoneWeights[aiWeight.mVertexId].push_back({boneIndex, aiWeight.mWeight});
                }
            }

            for (int32_t vi = 0; vi < static_cast<int32_t>(pAssimpMesh->mNumVertices); ++vi)
            {
                auto& srcBoneWeight = tempBoneWeights[vi];
                MeshIR::BoneWeight dstBoneWeight;

                if (!srcBoneWeight.empty())
                {
                    std::sort(srcBoneWeight.begin(),
                              srcBoneWeight.end(),
                              [](const TempBoneWeight& a, const TempBoneWeight& b) { return a.Weight > b.Weight; });

                    dstBoneWeight.BoneWeightCount = MeshIR::sMaxBoneChannel < static_cast<int32_t>(srcBoneWeight.size())
                                                        ? MeshIR::sMaxBoneChannel
                                                        : static_cast<int32_t>(srcBoneWeight.size());
                    for (int32_t bwi = 0; bwi < dstBoneWeight.BoneWeightCount; ++bwi)
                    {
                        dstBoneWeight.BoneIndices[bwi] = srcBoneWeight[bwi].BoneIndex;
                        dstBoneWeight.Weights[bwi] = srcBoneWeight[bwi].Weight;
                    }

                    float weightSum = 0.0f;

                    for (int32_t bwi = 0; bwi < dstBoneWeight.BoneWeightCount; ++bwi)
                    {
                        weightSum += dstBoneWeight.Weights[bwi];
                    }

                    if (!math::IsZeroApprox(weightSum))
                    {
                        for (int32_t bwi = 0; bwi < dstBoneWeight.BoneWeightCount; ++bwi)
                        {
                            dstBoneWeight.Weights[bwi] /= weightSum;
                        }
                    }
                }

                boneWeights.emplace_back(dstBoneWeight);
            }
        }

        // Indices
        for (int32_t fi = 0; fi < static_cast<int32_t>(pAssimpMesh->mNumFaces); ++fi)
        {
            const aiFace& assimpFace = pAssimpMesh->mFaces[fi];
            for (int32_t ii = 0; ii < static_cast<int32_t>(assimpFace.mNumIndices); ++ii)
            {
                indices.push_back(static_cast<uint32_t>(assimpFace.mIndices[ii]));
            }
        }

        // Morph targets
        std::unordered_map<std::string, int32_t> morphTargetNameCount; // for handling duplicated morph target name
        morphTargetNameCount.reserve(static_cast<int32_t>(pAssimpMesh->mNumAnimMeshes));
        for (int32_t mti = 0; mti < static_cast<int32_t>(pAssimpMesh->mNumAnimMeshes); ++mti)
        {
            const aiAnimMesh* pAssimpAnimMesh = pAssimpMesh->mAnimMeshes[mti];

            std::string mrphTargetNameStr;
            if (pAssimpAnimMesh->mName.Empty())
            {
                mrphTargetNameStr = nameStr + "_" + subMeshNameStr + "_unnamed_morph_target_" + std::to_string(mti);
            }
            else
            {
                mrphTargetNameStr = pAssimpAnimMesh->mName.C_Str();
                auto it = morphTargetNameCount.find(mrphTargetNameStr);
                if (it == morphTargetNameCount.end())
                {
                    morphTargetNameCount.insert({mrphTargetNameStr, 1});
                }
                else
                {
                    mrphTargetNameStr += "_" + std::to_string(it->second);
                    ++it->second;
                }
            }

            std::vector<Vector3> mtPositions;
            mtPositions.reserve(pAssimpAnimMesh->mNumVertices);
            std::vector<Vector3> mtNormals;
            mtNormals.reserve(pAssimpAnimMesh->mNumVertices);
            std::vector<Vector4> mtTangents;
            mtTangents.reserve(pAssimpAnimMesh->mNumVertices);

            bHasPos = pAssimpAnimMesh->HasPositions();
            bHasNormal = pAssimpAnimMesh->HasNormals();
            bHasTangent = pAssimpAnimMesh->HasTangentsAndBitangents();

            for (int32_t vi = 0; vi < static_cast<int32_t>(pAssimpAnimMesh->mNumVertices); ++vi)
            {
                // Morph target positions
                if (bHasPos)
                {
                    const aiVector3D& pos = pAssimpAnimMesh->mVertices[vi];
                    mtPositions.emplace_back(pos.x, pos.y, pos.z);
                }

                // Morph target Normals
                if (bHasNormal)
                {
                    const aiVector3D& normal = pAssimpAnimMesh->mNormals[vi];
                    mtNormals.emplace_back(normal.x, normal.y, normal.z);
                }

                // Morph target tangents (handedness in w component)
                if (bHasTangent)
                {
                    const aiVector3D& aiTangent = pAssimpAnimMesh->mTangents[vi];
                    const aiVector3D& aiBitangent = pAssimpAnimMesh->mBitangents[vi];
                    const Vector3 tangent(aiTangent.x, aiTangent.y, aiTangent.z);
                    const Vector3 bitangent(aiBitangent.x, aiBitangent.y, aiBitangent.z);

                    // calcuate handedness
                    float handedness = 1.0f;
                    if (bHasNormal)
                    {
                        const Vector3& normal = normals[vi];
                        handedness = (normal.Cross(tangent).Dot(bitangent) > 0.0f) ? 1.0f : -1.0f;
                    }
                    mtTangents.emplace_back(tangent.X, tangent.Y, tangent.Z, handedness);
                }
            }

            const float weight = pAssimpAnimMesh->mWeight;

            MeshIR::MorphTarget morphTarget(std::move(mrphTargetNameStr),
                                            std::move(mtPositions),
                                            std::move(mtNormals),
                                            std::move(mtTangents),
                                            weight);

            morphTargets.push_back(std::move(morphTarget));
        }

        MeshIR::SubMesh subMesh(std::move(subMeshNameStr),
                                primitiveType,
                                std::move(positions),
                                std::move(normals),
                                std::move(tangents),
                                std::move(uvs),
                                std::move(colors),
                                std::move(boneWeights),
                                std::move(indices),
                                std::move(morphTargets),
                                static_cast<int32_t>(pAssimpMesh->mMaterialIndex));

        subMeshes.push_back(std::move(subMesh));
    }

    return std::make_unique<MeshIR>(std::move(meshNameStr), std::move(subMeshes));
}

std::unique_ptr<const AnimationIR> parseAnimation(const std::string& nameStr,
                                                  const aiAnimation& assimpAnim,
                                                  const SkeletonIR& skeletonIR)
{
    std::string animNameStr = nameStr;

    const float duration = (assimpAnim.mDuration != 0.0) ? static_cast<float>(assimpAnim.mDuration) : 1.0f;
    const float ticksPerSecond =
        (assimpAnim.mTicksPerSecond != 0.0) ? static_cast<float>(assimpAnim.mTicksPerSecond) : 25.0f; // fallback

    // Skeletal tracks
    std::vector<AnimationIR::SkeletalTrack> skeletalTracks;
    skeletalTracks.reserve(assimpAnim.mNumChannels);
    for (int32_t ci = 0; ci < static_cast<int32_t>(assimpAnim.mNumChannels); ++ci)
    {
        const aiNodeAnim* pAssimpChannel = assimpAnim.mChannels[ci];
        if (!pAssimpChannel)
        {
            continue;
        }

        const std::string boneNameStr = pAssimpChannel->mNodeName.C_Str();
        const int32_t boneIndex = skeletonIR.BoneNameToIndexMap.at(boneNameStr);

        std::vector<AnimationIR::TranslationKey> translationKeys;
        translationKeys.reserve(pAssimpChannel->mNumPositionKeys);
        AnimationIR::eInterpolationMode translationInterpMode = AnimationIR::eInterpolationMode::Step;
        if (pAssimpChannel->mNumPositionKeys > 0)
        {
            switch (pAssimpChannel->mPositionKeys[0].mInterpolation)
            {
                case aiAnimInterpolation_Step:
                    translationInterpMode = AnimationIR::eInterpolationMode::Step;
                    break;
                case aiAnimInterpolation_Linear:
                    translationInterpMode = AnimationIR::eInterpolationMode::Linear;
                    break;
                case aiAnimInterpolation_Spherical_Linear:
                    translationInterpMode = AnimationIR::eInterpolationMode::SphericalLinear;
                    break;
                case aiAnimInterpolation_Cubic_Spline:
                    translationInterpMode = AnimationIR::eInterpolationMode::CubicSpline;
                    break;
                default:
                    HO_ASSERT(false, "Invalid Assimp interpolation enum.");
            }
        }
        for (int32_t ki = 0; ki < static_cast<int32_t>(pAssimpChannel->mNumPositionKeys); ++ki)
        {
            const aiVectorKey& key = pAssimpChannel->mPositionKeys[ki];
            translationKeys.push_back(
                {static_cast<float>(key.mTime) / ticksPerSecond, Vector3(key.mValue.x, key.mValue.y, key.mValue.z)});
        }
        std::sort(translationKeys.begin(),
                  translationKeys.end(),
                  [](const AnimationIR::TranslationKey& a, const AnimationIR::TranslationKey& b)
                  { return a.Time < b.Time; });

        std::vector<AnimationIR::RotationKey> rotationKeys;
        rotationKeys.reserve(pAssimpChannel->mNumRotationKeys);
        AnimationIR::eInterpolationMode rotationInterpMode = AnimationIR::eInterpolationMode::Step;
        if (pAssimpChannel->mNumRotationKeys > 0)
        {
            switch (pAssimpChannel->mRotationKeys[0].mInterpolation)
            {
                case aiAnimInterpolation_Step:
                    rotationInterpMode = AnimationIR::eInterpolationMode::Step;
                    break;
                case aiAnimInterpolation_Linear:
                    rotationInterpMode = AnimationIR::eInterpolationMode::Linear;
                    break;
                case aiAnimInterpolation_Spherical_Linear:
                    rotationInterpMode = AnimationIR::eInterpolationMode::SphericalLinear;
                    break;
                case aiAnimInterpolation_Cubic_Spline:
                    rotationInterpMode = AnimationIR::eInterpolationMode::CubicSpline;
                    break;
                default:
                    HO_ASSERT(false, "Invalid Assimp interpolation enum.");
            }
        }
        for (int32_t ki = 0; ki < static_cast<int32_t>(pAssimpChannel->mNumRotationKeys); ++ki)
        {
            const aiQuatKey& key = pAssimpChannel->mRotationKeys[ki];
            rotationKeys.push_back({static_cast<float>(key.mTime) / ticksPerSecond,
                                    Quaternion(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w)});
        }

        std::sort(rotationKeys.begin(),
                  rotationKeys.end(),
                  [](const AnimationIR::RotationKey& a, const AnimationIR::RotationKey& b) { return a.Time < b.Time; });

        std::vector<AnimationIR::ScalingKey> scalingKeys;
        scalingKeys.reserve(pAssimpChannel->mNumScalingKeys);
        AnimationIR::eInterpolationMode scalingInterpMode = AnimationIR::eInterpolationMode::Step;
        if (pAssimpChannel->mNumScalingKeys > 0)
        {
            switch (pAssimpChannel->mScalingKeys[0].mInterpolation)
            {
                case aiAnimInterpolation_Step:
                    scalingInterpMode = AnimationIR::eInterpolationMode::Step;
                    break;
                case aiAnimInterpolation_Linear:
                    scalingInterpMode = AnimationIR::eInterpolationMode::Linear;
                    break;
                case aiAnimInterpolation_Spherical_Linear:
                    scalingInterpMode = AnimationIR::eInterpolationMode::SphericalLinear;
                    break;
                case aiAnimInterpolation_Cubic_Spline:
                    scalingInterpMode = AnimationIR::eInterpolationMode::CubicSpline;
                    break;
                default:
                    HO_ASSERT(false, "Invalid Assimp interpolation enum.");
            }
        }
        for (int32_t ki = 0; ki < static_cast<int32_t>(pAssimpChannel->mNumScalingKeys); ++ki)
        {
            const aiVectorKey& key = pAssimpChannel->mScalingKeys[ki];
            scalingKeys.push_back(
                {static_cast<float>(key.mTime) / ticksPerSecond, Vector3(key.mValue.x, key.mValue.y, key.mValue.z)});
        }

        std::sort(scalingKeys.begin(),
                  scalingKeys.end(),
                  [](const AnimationIR::ScalingKey& a, const AnimationIR::ScalingKey& b) { return a.Time < b.Time; });

        AnimationIR::eExtrapolationMode preExtrapMode;
        switch (pAssimpChannel->mPreState)
        {
            case aiAnimBehaviour_DEFAULT:
                preExtrapMode = AnimationIR::eExtrapolationMode::Default;
                break;
            case aiAnimBehaviour_CONSTANT:
                preExtrapMode = AnimationIR::eExtrapolationMode::Constant;
                break;
            case aiAnimBehaviour_LINEAR:
                preExtrapMode = AnimationIR::eExtrapolationMode::Linear;
                break;
            case aiAnimBehaviour_REPEAT:
                preExtrapMode = AnimationIR::eExtrapolationMode::Repeat;
                break;
            default:
                HO_ASSERT(false, "Invalid Assimp extrapolation enum.");
        }
        AnimationIR::eExtrapolationMode postExtrapMode;
        switch (pAssimpChannel->mPostState)
        {
            case aiAnimBehaviour_DEFAULT:
                postExtrapMode = AnimationIR::eExtrapolationMode::Default;
                break;
            case aiAnimBehaviour_CONSTANT:
                postExtrapMode = AnimationIR::eExtrapolationMode::Constant;
                break;
            case aiAnimBehaviour_LINEAR:
                postExtrapMode = AnimationIR::eExtrapolationMode::Linear;
                break;
            case aiAnimBehaviour_REPEAT:
                postExtrapMode = AnimationIR::eExtrapolationMode::Repeat;
                break;
            default:
                HO_ASSERT(false, "Invalid Assimp extrapolation enum.");
        }

        skeletalTracks.emplace_back(boneIndex,
                                    std::move(translationKeys),
                                    translationInterpMode,
                                    std::move(rotationKeys),
                                    rotationInterpMode,
                                    std::move(scalingKeys),
                                    scalingInterpMode,
                                    preExtrapMode,
                                    postExtrapMode);
    }

    // Morph Target tracks
    std::vector<AnimationIR::MorphTargetTrack> morphTargetTracks;
    morphTargetTracks.reserve(assimpAnim.mNumMorphMeshChannels);
    for (int32_t ci = 0; ci < static_cast<int32_t>(assimpAnim.mNumMorphMeshChannels); ++ci)
    {
        const aiMeshMorphAnim* pAssimpChannel = assimpAnim.mMorphMeshChannels[ci];
        if (!pAssimpChannel)
        {
            continue;
        }

        const std::string boneNameStr = pAssimpChannel->mName.C_Str();
        const int32_t boneIndex = skeletonIR.BoneNameToIndexMap.at(boneNameStr);

        std::vector<AnimationIR::MorphingKey> morphingKeys;
        morphingKeys.reserve(pAssimpChannel->mNumKeys);
        for (int32_t ki = 0; ki < static_cast<int32_t>(pAssimpChannel->mNumKeys); ++ki)
        {
            const aiMeshMorphKey& key = pAssimpChannel->mKeys[ki];
            std::vector<int32_t> morphTargetIndices;
            std::vector<float> weights;
            morphTargetIndices.reserve(key.mNumValuesAndWeights);
            weights.reserve(key.mNumValuesAndWeights);
            for (int32_t vi = 0; vi < static_cast<int32_t>(key.mNumValuesAndWeights); ++vi)
            {
                morphTargetIndices.push_back(static_cast<int32_t>(key.mValues[vi]));
                weights.push_back(static_cast<float>(key.mWeights[vi]));
            }
            morphingKeys.emplace_back(
                static_cast<float>(key.mTime) / ticksPerSecond, std::move(morphTargetIndices), std::move(weights));
        }

        std::sort(morphingKeys.begin(),
                  morphingKeys.end(),
                  [](const AnimationIR::MorphingKey& a, const AnimationIR::MorphingKey& b) { return a.Time < b.Time; });

        morphTargetTracks.emplace_back(boneIndex, std::move(morphingKeys));
    }

    return std::make_unique<AnimationIR>(
        std::move(animNameStr), duration / ticksPerSecond, std::move(skeletalTracks), std::move(morphTargetTracks));
}

std::unique_ptr<SkinIR> parseSkin(const std::string& nameStr,
                                  const ModelParsingContext& parsingContext,
                                  const SkeletonIR& skeletonIR)
{
    std::string skinNameStr = nameStr;

    std::vector<Transform3D> offsetTransforms(skeletonIR.BoneNameStrs.size(), Transform3D());

    for (int32_t mi = 0; mi < static_cast<int32_t>(parsingContext.pAssimpScene->mNumMeshes); ++mi)
    {
        const aiMesh* pAssimpMesh = parsingContext.pAssimpScene->mMeshes[mi];
        for (int32_t bi = 0; bi < static_cast<int32_t>(pAssimpMesh->mNumBones); ++bi)
        {
            const aiBone* pAssimpBone = pAssimpMesh->mBones[bi];
            const std::string boneNameStr = pAssimpBone->mName.C_Str();
            const aiMatrix4x4& om = pAssimpBone->mOffsetMatrix;
            const Matrix4x4 m(Vector4(om.a1, om.b1, om.c1, om.d1),
                              Vector4(om.a2, om.b2, om.c2, om.d2),
                              Vector4(om.a3, om.b3, om.c3, om.d3),
                              Vector4(om.a4, om.b4, om.c4, om.d4));
            offsetTransforms[skeletonIR.BoneNameToIndexMap.at(boneNameStr)] = Transform3D(m);
        }
    }

    std::vector<std::vector<int32_t>> boneToSubMeshMap(parsingContext.pFlattedScene.size());

    for (int32_t bi = 0; bi < static_cast<int32_t>(parsingContext.pFlattedScene.size()); ++bi)
    {
        const aiNode* aiNode = parsingContext.pFlattedScene[bi];
        for (int32_t mi = 0; mi < static_cast<int32_t>(aiNode->mNumMeshes); ++mi)
        {
            boneToSubMeshMap[bi].push_back(static_cast<int32_t>(aiNode->mMeshes[mi]));
        }
    }

    return std::make_unique<SkinIR>(std::move(skinNameStr), std::move(offsetTransforms), std::move(boneToSubMeshMap));
}

std::string extractFileName(const std::string& fileNameWithExtStr)
{
    const size_t dotIdx = fileNameWithExtStr.rfind('.');
    if (dotIdx == std::string::npos)
    {
        return fileNameWithExtStr;
    }
    return fileNameWithExtStr.substr(0, dotIdx);
}

} // namespace parser
} // namespace ho