#pragma once
#include <memory>
#include <string>

#include "Core/IO/ModelIR.h"
#include "Core/IO/Path.h"
#include "Core/IO/ShaderIR.h"
#include "Core/Templates/FixedArray.h"
#include "Subsystems/Asset/AssetDefs.h"
#include "Subsystems/Asset/Assets.h"
#include "Subsystems/Object/ObjectHandle.h"
#include "Subsystems/Rendering./RenderPass/EnvironmentBakePassDesc.h"

namespace ho
{

struct ModelParseContext
{
    Path Path;
    char ModelName[256] = {};
    bool bMakeStatic = false;

    bool bParseComplete = false;
};

struct ModelImportContext
{
    std::unique_ptr<const parser::ModelIR> pParsedModel;

    bool bImportAsStaticMesh = true;

    FixedArray<eMaterialAssetType> MaterialTypes;

    FixedArray<eMaterialTextureUsage> TextureUsage;
    FixedArray<eTextureFormat> TextureFormats;
    FixedArray<bool> bGenerateMipmap;

    bool bHasMorphTargets = false;
    bool bImportMorphTargets = false;

    bool bHasMorphAnimation = false;
    FixedArray<bool> bImportMorphAnimation;

    bool bImportBeginComplete = false;
    bool bImportBeginSucceed = true;

    bool bImportComplete = false;
    bool bImportSucceed = true;

    int32_t TotalImportedAssetCount = 0;
    int32_t ImportedAssetCount = 0;

    ObjectHandle<RStaticMeshAsset> hImportedStaticMesh;
    FixedArray<ObjectHandle<RMaterialAsset>> hImportedMaterials;
    FixedArray<ObjectHandle<RTextureAsset>> hImportedTextures;
    ObjectHandle<RMorphTargetsAsset> hImportedMorphTargets;
    FixedArray<ObjectHandle<RMorphAnimationAsset>> hImportedMorphAnims;
};

struct TextureParseContext
{
    eTextureType TextureType = eTextureType::None;

    Path Path[6] = {};
    char TextureName[256] = {};

    bool bIsLinear = false;

    bool bParseComplete = false;
};

struct TextureImportContext
{
    eTextureType TextureType;

    std::unique_ptr<const parser::TextureIR> pParsedTextures[6] = {};

    eTextureFormat TextureFormat = eTextureFormat::R8G8B8A8_SRGB;
    bool bGenerateMipmap = true;

    bool bImportBeginComplete = false;
    bool bImportBeginSucceed = true;

    bool bImportComplete = false;
    bool bImportSucceed = true;

    int32_t TotalImportedTextureCount = 0;
    int32_t ImportedTextureCount = 0;

    ObjectHandle<RTextureAsset> hImportedTexture;
};

struct ShaderParseContext
{
    Path Path;
    char ShaderName[256] = {};

    bool bParseComplete = false;
};

struct ShaderImportContext
{
    std::unique_ptr<const parser::ShaderIR> pParsedShader;
    eShaderStage ShaderStage = eShaderStage::None;
    eMaterialAssetType CompatibleMaterialType = eMaterialAssetType::None;

    bool bImportComplete = false;
    bool bImportSucceed = true;

    std::string CompileErrorMsg;

    ObjectHandle<RShaderAsset> hImportedShader;
};

struct TextureBakeContext
{
    bool bBakeComplete = false;

    ObjectHandle<RTextureAsset> hSrcTexture;

    char DstTextureName[256] = {};

    eEnvironmentBakeType BakeType = eEnvironmentBakeType::Irradiance;
    int32_t BakedMapSize = 0;
    float SampleDelta = 0.0f;
    int32_t SampleCount = 0;
    bool bBakeSucceed = true;

    ObjectHandle<RTextureAsset> hDstTexture;
};
} // namespace ho