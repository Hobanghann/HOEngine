#include "ResourceLoader.h"

#include <assimp/pbrmaterial.h>
#include <assimp/scene.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stb_image.h>
#include <string>
#include <unordered_map>

#include "AnimationIR.h"
#include "Core/Math/Transform3D.h"
#include "Image.h"
#include "MaterialIR.h"
#include "MeshIR.h"
#include "ModelIR.h"
#include "Path.h"
#include "ResourceImporter.h"
#include "ShaderIR.h"
#include "SkeletonIR.h"
#include "SkinIR.h"
#include "TextureIR.h"

namespace ho
{

static std::string extractFileName(const std::string& fileNameWithExtStr)
{
    size_t dotIdx = fileNameWithExtStr.rfind('.');
    if (dotIdx == std::string::npos)
    {
        return fileNameWithExtStr;
    }
    return fileNameWithExtStr.substr(0, dotIdx);
}

std::unique_ptr<const ModelIR> ResourceLoader::LoadModel(const std::string& nameStr,
                                                         const Path& path,
                                                         bool bMakeStatic,
                                                         bool bConvertToLeftHanded)
{
    std::unique_ptr<ModelImportContext> upImportContext =
        ResourceImporter::ImportModel(path, bMakeStatic, bConvertToLeftHanded);
    if (!upImportContext)
    {
        return nullptr;
    }

    Path parentPath = path.GetParentPath();
    std::string fileNameStr = path.GetFileName().ToString();

    std::unique_ptr<ModelIR> upModelIR = std::make_unique<ModelIR>();

    upModelIR->NameStr = nameStr;

    // Load materials
    std::unordered_map<std::string, int32_t> matNameCount; // for handling duplicated name
    matNameCount.reserve(upImportContext->pAssimpScene->mNumMaterials);
    for (int32_t mi = 0; mi < static_cast<int32_t>(upImportContext->pAssimpScene->mNumMaterials); ++mi)
    {
        const aiMaterial* pAssimpMaterial = upImportContext->pAssimpScene->mMaterials[mi];
        HO_ASSERT(pAssimpMaterial,
                  "Assimp material mapping failed: Mesh refers to an invalid or non-existent material index.");
        aiTexture** pAssimpEmbTextures = upImportContext->pAssimpScene->mTextures;
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

        std::unique_ptr<const MaterialIR> upMatIR =
            loadMaterial(matNameStr, *pAssimpMaterial, pAssimpEmbTextures, parentPath, &(upModelIR->upTextureIRs));
        HO_ASSERT(upMatIR, (std::string("Failed to load material of ") + matNameStr).c_str());

        upModelIR->upMaterialIRs.emplace_back(std::move(upMatIR));
    }

    // Load skeleton
    std::unique_ptr<const SkeletonIR> upSkeletonIR = loadSkeleton(nameStr, *upImportContext);
    HO_ASSERT(upSkeletonIR, (std::string("Failed to load skeleton of ") + fileNameStr).c_str());
    upModelIR->upSkeletonIR = std::move(upSkeletonIR);

    // Load mesh
    std::unique_ptr<const MeshIR> upMeshIR =
        loadMesh(nameStr, *upImportContext, *(upModelIR->upSkeletonIR), upModelIR->upMaterialIRs);
    HO_ASSERT(upMeshIR, (std::string("Failed to load mesh of ") + fileNameStr).c_str());
    upModelIR->upMeshIR = std::move(upMeshIR);

    // Load animations
    std::unordered_map<std::string, int> AnimNameCount;
    AnimNameCount.reserve(upImportContext->pAssimpScene->mNumAnimations);
    for (int32_t ai = 0; ai < static_cast<int32_t>(upImportContext->pAssimpScene->mNumAnimations); ++ai)
    {
        const aiAnimation* pAssimpAnim = upImportContext->pAssimpScene->mAnimations[ai];
        std::string animNameStr;
        if (pAssimpAnim->mName.Empty())
        {
            animNameStr = fileNameStr + "_" + "unnamed_animation_" + std::to_string(ai);
        }
        else
        {
            animNameStr = pAssimpAnim->mName.C_Str();
            auto it = AnimNameCount.find(animNameStr);
            if (it == AnimNameCount.end())
            {
                AnimNameCount.insert({animNameStr, 1});
            }
            else
            {
                animNameStr += "_" + std::to_string(it->second);
                ++it->second;
            }
        }
        std::unique_ptr<const AnimationIR> upAnimIR =
            loadAnimation(animNameStr, *pAssimpAnim, *(upModelIR->upSkeletonIR));
        HO_ASSERT(upAnimIR, (std::string("Failed to load animation of ") + fileNameStr).c_str());
        upModelIR->upAnimationIRs.emplace_back(std::move(upAnimIR));
    }

    // Load Skin
    std::unique_ptr<const SkinIR> upSkinIR = loadSkin(nameStr, *upImportContext, *(upModelIR->upSkeletonIR));
    HO_ASSERT(upSkinIR, (std::string("Failed to load skin of ") + fileNameStr).c_str());
    upModelIR->upSkinIR = std::move(upSkinIR);

    return upModelIR;
}

std::unique_ptr<const TextureIR> ResourceLoader::LoadTexture(const std::string& nameStr, const Path& path)
{
    std::unique_ptr<Image> upImg = ResourceImporter::ImportImage(path);

    if (!upImg)
    {
        return nullptr;
    }

    std::vector<std::unique_ptr<Image>> upImgs;
    upImgs.emplace_back(std::move(upImg));
    return std::make_unique<TextureIR>(std::string(nameStr), std::move(upImgs), TextureIR::eTextureType::Texture2D);
}

std::unique_ptr<const ShaderIR> ResourceLoader::LoadShader(const std::string& nameStr, const Path& path)
{
    std::string shaderSourceStr;

    std::ifstream shaderStream(path.ToString(), std::ios::binary | std::ios::ate);

    if (!shaderStream.is_open())
    {
        return nullptr;
    }

    std::streamsize size = shaderStream.tellg();
    shaderStream.seekg(0, std::ios::beg);

    shaderSourceStr.resize(static_cast<size_t>(size));

    if (!shaderStream.read(shaderSourceStr.data(), size))
    {
        return nullptr;
    }

    shaderStream.close();

    std::unique_ptr<ShaderIR> upShaderIR;
    upShaderIR->NameStr = nameStr;
    upShaderIR->SourceStr = std::move(shaderSourceStr);

    return upShaderIR;
}

std::unique_ptr<const TextureIR> ResourceLoader::loadEmbeddedTexture(const aiTexture& assimpTexture)
{
    int32_t width = 0;
    int32_t height = 0;
    int32_t numColorChannels = 0;

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
            pStbiBitmapHDR =
                stbi_loadf_from_memory(pCompressedData, compressedSize, &width, &height, &numColorChannels, 0);
            if (!pStbiBitmapHDR)
            {
                return nullptr;
            }
        }
        else
        {
            pStbiBitmapLDR =
                stbi_load_from_memory(pCompressedData, compressedSize, &width, &height, &numColorChannels, 0);
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

        int32_t size = width * height * numColorChannels;
        pStbiBitmapLDR = new std::uint8_t[size];

        for (int32_t y = 0; y < height; ++y)
        {
            for (int32_t x = 0; x < width; ++x)
            {
                const aiTexel& assimpTexel = assimpTexture.pcData[y * width + x];
                int32_t idx = (y * width + x) * numColorChannels;
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

    std::unique_ptr<Image> upImg = std::make_unique<Image>(
        Path(std::string()), assimpTexture.mFilename.C_Str(), format, width, height, pFinalBitmap);

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

    std::vector<std::unique_ptr<Image>> upImgs;
    upImgs.emplace_back(std::move(upImg));
    return std::make_unique<TextureIR>(
        std::string(assimpTexture.mFilename.C_Str()), std::move(upImgs), TextureIR::eTextureType::Texture2D);
}

std::unique_ptr<const MaterialIR> ResourceLoader::loadMaterial(
    const std::string& nameStr,
    const aiMaterial& assimpMaterial,
    aiTexture** pAssimpEmbTextures,
    const Path& parentPath,
    std::vector<std::unique_ptr<const TextureIR>>* pOutTextureIRs)
{
    std::unique_ptr<MaterialIR> upMaterialIR = std::make_unique<MaterialIR>();
    upMaterialIR->NameStr = nameStr;

    aiColor3D color3D;
    aiColor4D color4D;
    float fVal = 0.0f;
    int iVal = 0;
    aiString aiStr;

    // Legacy / Phong Attributes
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_COLOR_AMBIENT, color3D))
    {
        upMaterialIR->Ambient = Color128(color3D.r, color3D.g, color3D.b);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_COLOR_DIFFUSE, color3D))
    {
        upMaterialIR->Diffuse = Color128(color3D.r, color3D.g, color3D.b);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_COLOR_SPECULAR, color3D))
    {
        upMaterialIR->Specular = Color128(color3D.r, color3D.g, color3D.b);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_SHININESS, fVal))
    {
        upMaterialIR->Shininess = fVal;
    }

    // PBR Core Attributes
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_BASE_COLOR, color4D))
    {
        upMaterialIR->Albedo = Color128(color4D.r, color4D.g, color4D.b, color4D.a);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_METALLIC_FACTOR, fVal))
    {
        upMaterialIR->Metallic = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_ROUGHNESS_FACTOR, fVal))
    {
        upMaterialIR->Roughness = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_REFRACTI, fVal))
    {
        upMaterialIR->IndexOfRefraction = fVal;
    }

    // Global / Transparency Attributes
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_OPACITY, fVal))
    {
        upMaterialIR->Opacity = fVal;
    }

    upMaterialIR->AlphaMode =
        upMaterialIR->Opacity < 1.f ? MaterialIR::eAlphaMode::Blend : MaterialIR::eAlphaMode::Opaque;
    upMaterialIR->AlphaThreshold = 0.5f;

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_GLTF_ALPHAMODE, aiStr))
    {
        std::string mode(aiStr.C_Str());
        if (mode == "MASK")
        {
            upMaterialIR->AlphaMode = MaterialIR::eAlphaMode::Mask;
        }
        else if (mode == "BLEND")
        {
            upMaterialIR->AlphaMode = MaterialIR::eAlphaMode::Blend;
        }
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_GLTF_ALPHACUTOFF, fVal))
    {
        upMaterialIR->AlphaThreshold = fVal;
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_BLEND_FUNC, iVal))
    {
        if (static_cast<aiBlendMode>(iVal) == aiBlendMode::aiBlendMode_Default)
        {
            upMaterialIR->BlendMode = MaterialIR::eAlphaBlendMode::Default;
        }
        if (static_cast<aiBlendMode>(iVal) == aiBlendMode::aiBlendMode_Additive)
        {
            upMaterialIR->BlendMode = MaterialIR::eAlphaBlendMode::Additive;
        }
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_COLOR_EMISSIVE, color3D))
    {
        upMaterialIR->Emissive = Color128(color3D.r, color3D.g, color3D.b);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_EMISSIVE_INTENSITY, fVal))
    {
        upMaterialIR->EmissiveIntensity = fVal;
    }

    // Advanced PBR Attributes
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_TRANSMISSION_FACTOR, fVal))
    {
        upMaterialIR->TransmissionFactor = fVal;
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_CLEARCOAT_FACTOR, fVal))
    {
        upMaterialIR->ClearcoatFactor = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, fVal))
    {
        upMaterialIR->ClearcoatRoughness = fVal;
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_SHEEN_COLOR_FACTOR, color3D))
    {
        upMaterialIR->SheenColor = Color128(color3D.r, color3D.g, color3D.b);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_SHEEN_ROUGHNESS_FACTOR, fVal))
    {
        upMaterialIR->SheenRoughnessFactor = fVal;
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_VOLUME_THICKNESS_FACTOR, fVal))
    {
        upMaterialIR->VolumeThicknessFactor = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_VOLUME_ATTENUATION_DISTANCE, fVal))
    {
        upMaterialIR->VolumeAttenuationDist = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_VOLUME_ATTENUATION_COLOR, color3D))
    {
        upMaterialIR->VolumeAttenuationColor = Color128(color3D.r, color3D.g, color3D.b);
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_ANISOTROPY_FACTOR, fVal))
    {
        upMaterialIR->AnisotropyFactor = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_ANISOTROPY_ROTATION, fVal))
    {
        upMaterialIR->AnisotropyRotation = fVal;
    }

    // Texture Strength / Scale
    if (AI_SUCCESS == assimpMaterial.Get("$mat.gltf.normalScale", 0, 0, fVal))
    {
        upMaterialIR->NormalScale = fVal;
    }
    else if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_BUMPSCALING, fVal))
    {
        upMaterialIR->ParallaxScale = fVal;
    }
    if (AI_SUCCESS == assimpMaterial.Get("$mat.gltf.occlusionStrength", 0, 0, fVal))
    {
        upMaterialIR->OcclusionStrength = fVal;
    }

    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_BUMPSCALING, fVal))
    {
        upMaterialIR->ParallaxScale = fVal;
    }

    // Render States
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_ENABLE_WIREFRAME, iVal))
    {
        upMaterialIR->bWireframe = (iVal != 0);
    }
    if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_TWOSIDED, iVal))
    {
        upMaterialIR->bBackfaceCulling = (iVal == 0);
    }

    // Texture Load Helper
    auto loadTextureToMaterial = [&](aiTextureType aiType, MaterialIR::eTextureUsage engineType)
    {
        if (upMaterialIR->TextureIRIndices[static_cast<int32_t>(engineType)] != -1)
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
            std::unique_ptr<const TextureIR> upTextureIR;
            if (bEmbedded)
            {
                upTextureIR = loadEmbeddedTexture(*pAssimpEmbTextures[embTexIdx]);
            }
            else if (!texFileNameStr.empty())
            {
                Path texAbsPath = parentPath / Path(texFileNameStr);
                upTextureIR = LoadTexture(texNameStr, texAbsPath);
            }

            if (upTextureIR)
            {
                texIRIndex = pOutTextureIRs->size();
                pOutTextureIRs->emplace_back(std::move(upTextureIR));
            }
        }

        upMaterialIR->TextureIRIndices[static_cast<int32_t>(engineType)] = texIRIndex;
        upMaterialIR->UVChannels[static_cast<int32_t>(engineType)] = static_cast<int32_t>(uv);

        // UV Transform Matrix
        aiUVTransform aiTransform;
        if (AI_SUCCESS == assimpMaterial.Get(AI_MATKEY_UVTRANSFORM(aiType, 0), aiTransform))
        {
            Matrix3x3 T = Matrix3x3::sIdentity;
            T.Data[0][2] = aiTransform.mTranslation.x;
            T.Data[1][2] = aiTransform.mTranslation.y;

            Matrix3x3 R = Matrix3x3::sIdentity;
            float sin = 0.f;
            float cos = 0.f;
            math::SinCos(&sin, &cos, aiTransform.mRotation);
            R.Data[0][0] = cos;
            R.Data[0][1] = -sin;
            R.Data[1][0] = sin;
            R.Data[1][1] = cos;

            Matrix3x3 S = Matrix3x3::sIdentity;
            S.Data[0][0] = aiTransform.mScaling.x;
            S.Data[1][1] = aiTransform.mScaling.y;

            upMaterialIR->UVTransforms[static_cast<int32_t>(engineType)] = T * R * S;
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

    return upMaterialIR;
}

std::unique_ptr<const SkeletonIR> ResourceLoader::loadSkeleton(const std::string& nameStr,
                                                               const ModelImportContext& importContext)
{
    std::vector<std::string> boneNameStrs;
    std::vector<Transform3D> localTransforms;
    std::vector<int32_t> parents;
    std::unordered_map<std::string, int32_t> boneNameToIndex;

    boneNameStrs.reserve(importContext.pFlattedScene.size());
    localTransforms.reserve(importContext.pFlattedScene.size());
    parents.reserve(importContext.pFlattedScene.size());
    boneNameToIndex.reserve(importContext.pFlattedScene.size());

    std::unordered_map<std::string, int32_t> boneNameCount; // for handling duplicated bone name
    boneNameCount.reserve(importContext.pFlattedScene.size());
    for (int32_t i = 0; i < static_cast<int32_t>(importContext.pFlattedScene.size()); ++i)
    {
        const aiNode* pAssimpNode = importContext.pFlattedScene[i];

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
        aiMatrix4x4 aim = pAssimpNode->mTransformation;
        Matrix4x4 m(Vector4(aim.a1, aim.b1, aim.c1, aim.d1),
                    Vector4(aim.a2, aim.b2, aim.c2, aim.d2),
                    Vector4(aim.a3, aim.b3, aim.c3, aim.d3),
                    Vector4(aim.a4, aim.b4, aim.c4, aim.d4));
        localTransforms.emplace_back(Transform3D(m));

        boneNameToIndex[boneNameStr] = i;

        // parent bone
        if (pAssimpNode->mParent)
        {
            auto it = importContext.NodeToIndex.find(pAssimpNode->mParent);
            if (it == importContext.NodeToIndex.end())
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
        std::string(nameStr), std::move(boneNameStrs), std::move(localTransforms), std::move(parents));
}

std::unique_ptr<const MeshIR> ResourceLoader::loadMesh(
    const std::string& nameStr,
    const ModelImportContext& importContext,
    const SkeletonIR& skeletonIR,
    const std::vector<std::unique_ptr<const MaterialIR>>& materialIRs)
{
    std::vector<MeshIR::SubMesh> subMeshes;
    subMeshes.reserve(importContext.pAssimpScene->mNumMeshes);

    // submeshes
    std::unordered_map<std::string, int> subMeshNameCount; // for handling duplicated submesh name
    subMeshNameCount.reserve(importContext.pAssimpScene->mNumMeshes);
    for (int32_t mi = 0; mi < static_cast<int32_t>(importContext.pAssimpScene->mNumMeshes); ++mi)
    {
        const aiMesh* pAssimpMesh = importContext.pAssimpScene->mMeshes[mi];
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
                Vector3 tangent(aiTangent.x, aiTangent.y, aiTangent.z);
                Vector3 bitangent(aiBitangent.x, aiBitangent.y, aiBitangent.z);

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
                const int32_t boneIndex = skeletonIR.GetBoneIndex(pAssimpBone->mName.C_Str());

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
                    Vector3 tangent(aiTangent.x, aiTangent.y, aiTangent.z);
                    Vector3 bitangent(aiBitangent.x, aiBitangent.y, aiBitangent.z);

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

            float weight = pAssimpAnimMesh->mWeight;

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
                                pAssimpMesh->mMaterialIndex);

        subMeshes.push_back(std::move(subMesh));
    }

    std::string meshNameStr = nameStr;

    return std::make_unique<MeshIR>(std::move(meshNameStr), std::move(subMeshes));
}

std::unique_ptr<const AnimationIR> ResourceLoader::loadAnimation(const std::string& nameStr,
                                                                 const aiAnimation& assimpAnim,
                                                                 const SkeletonIR& skeletonIR)
{
    float duration = (assimpAnim.mDuration != 0.0) ? static_cast<float>(assimpAnim.mDuration) : 1.0f;
    float ticksPerSecond =
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

        std::string boneNameStr = pAssimpChannel->mNodeName.C_Str();
        int32_t boneIndex = skeletonIR.GetBoneIndex(boneNameStr);

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

        std::string boneNameStr = pAssimpChannel->mName.C_Str();
        int32_t boneIndex = skeletonIR.GetBoneIndex(boneNameStr);

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
                morphTargetIndices.push_back(key.mValues[vi]);
                weights.push_back(static_cast<float>(key.mWeights[vi]));
            }
            morphingKeys.emplace_back(
                static_cast<float>(key.mTime) / ticksPerSecond, std::move(morphTargetIndices), std::move(weights));
        }

        morphTargetTracks.emplace_back(boneIndex, std::move(morphingKeys));
    }

    std::string animNameStr = nameStr;
    return std::make_unique<AnimationIR>(
        std::move(animNameStr), duration / ticksPerSecond, std::move(skeletalTracks), std::move(morphTargetTracks));
}

std::unique_ptr<SkinIR> ResourceLoader::loadSkin(const std::string& nameStr,
                                                 const ModelImportContext& importContext,
                                                 const SkeletonIR& skeletonIR)
{
    std::vector<Transform3D> offsetTransforms(skeletonIR.GetBoneCount(), Transform3D());

    for (int32_t mi = 0; mi < static_cast<int32_t>(importContext.pAssimpScene->mNumMeshes); ++mi)
    {
        const aiMesh* pAssimpMesh = importContext.pAssimpScene->mMeshes[mi];
        for (int32_t bi = 0; bi < static_cast<int32_t>(pAssimpMesh->mNumBones); ++bi)
        {
            aiBone* pAssimpBone = pAssimpMesh->mBones[bi];
            std::string boneNameStr = pAssimpBone->mName.C_Str();
            const aiMatrix4x4& om = pAssimpBone->mOffsetMatrix;
            Matrix4x4 m(Vector4(om.a1, om.b1, om.c1, om.d1),
                        Vector4(om.a2, om.b2, om.c2, om.d2),
                        Vector4(om.a3, om.b3, om.c3, om.d3),
                        Vector4(om.a4, om.b4, om.c4, om.d4));
            offsetTransforms[skeletonIR.GetBoneIndex(boneNameStr)] = Transform3D(m);
        }
    }

    std::string skinNameStr = nameStr;
    return std::make_unique<SkinIR>(std::move(skinNameStr), std::move(offsetTransforms));
}
} // namespace ho