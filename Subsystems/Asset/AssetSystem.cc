#include "AssetSystem.h"

#include "AssetImportFuncs.h"
#include "Core/IO/ResourceParseFuncs.h"
#include "Core/Log/Logger.h"
#include "Subsystems/Object/ObjectSystem.h"

namespace ho
{
AssetSystem& AssetSystem::GetInstance()
{
    HO_ASSERT(spInstance, "Asset system is not created");
    return *spInstance;
}

bool AssetSystem::LoadBuiltInAssets()
{
    HO_ASSERT(!mbBuiltInAssetLoaded, "Built-in assets are loaded already.");

    // Load default shape meshes
    Path builtInAssetPath = Path(std::string("Subsystems/Asset/BuiltInAssets"));
    builtInAssetPath.ResolveProjectPath();

    ObjectHandle<RStaticMeshAsset>* phDstMeshOnly[] = {
        &mBuiltInAssets.hCubeMesh,
        &mBuiltInAssets.hSparseSphereMesh,
        &mBuiltInAssets.hSphereMesh,
        &mBuiltInAssets.hCapsuleMesh,
        &mBuiltInAssets.hCylinderMesh,
        &mBuiltInAssets.hPlaneMesh,
        &mBuiltInAssets.hQuadMesh,
    };
    const char* relPathMeshOnly[] = {
        "DefaultShapes/Cube/cube.gltf",
        "DefaultShapes/SparseSphere/SparseSphere.gltf",
        "DefaultShapes/Sphere/Sphere.glb",
        "DefaultShapes/Capsule/capsule.gltf",
        "DefaultShapes/Cylinder/Cylinder.glb",
        "DefaultShapes/Plane/plane.glb",
        "DefaultShapes/Quad/quad.glb",
    };
    const char* namesMeshOnly[] = {
        "Cube",
        "SparseSphere",
        "Sphere",
        "Capsule",
        "Cylinder",
        "Plane",
        "Quad",
    };

    static_assert(HO_ARRAY_SIZE(phDstMeshOnly) == HO_ARRAY_SIZE(relPathMeshOnly) &&
                  HO_ARRAY_SIZE(relPathMeshOnly) == HO_ARRAY_SIZE(namesMeshOnly));

    bool bSucceed = true;
    for (int32_t i = 0; i < HO_ARRAY_SIZE(phDstMeshOnly); ++i)
    {
        const Path path = builtInAssetPath / Path(std::string(relPathMeshOnly[i]));
        const std::unique_ptr<const parser::ModelIR> pIR = parser::parseModelFile(path, namesMeshOnly[i], true);
        if (!pIR)
        {
            bSucceed = false;
            break;
        }

        *phDstMeshOnly[i] =
            ImportStaticMeshImmediate(*pIR->pMeshIR, pIR->pMaterialIRs, *pIR->pSkeletonIR, *pIR->pSkinIR);
        if (phDstMeshOnly[i]->IsNULL())
        {
            bSucceed = false;
            break;
        }
    }

    if (!bSucceed)
    {
        for (int i = 0; i < HO_ARRAY_SIZE(phDstMeshOnly); ++i)
        {
            DeleteAssetImmediate(*phDstMeshOnly[i]);
        }
        return false;
    }

    ObjectHandle<RStaticMeshAsset>* phDst[] = {
        &mBuiltInAssets.hStageLightMesh,
        &mBuiltInAssets.hLightBulbMesh,
        &mBuiltInAssets.hFlashLightMesh,
        &mBuiltInAssets.hCameraMesh,
    };
    const char* relPath[] = {
        "DebugShapes/StageLight/scene.gltf",
        "DebugShapes/Lightbulb/scene.gltf",
        "DebugShapes/FlashLight/scene.gltf",
        "DebugShapes/Camera/scene.gltf",
    };
    const char* names[] = {
        "StageLight",
        "Lightbulb",
        "FlashLight",
        "Camera",
    };

    static_assert(HO_ARRAY_SIZE(phDst) == HO_ARRAY_SIZE(relPath) && HO_ARRAY_SIZE(relPath) == HO_ARRAY_SIZE(names));
    for (int32_t i = 0; i < HO_ARRAY_SIZE(phDst); ++i)
    {
        const Path path = builtInAssetPath / Path(std::string(relPath[i]));
        const std::unique_ptr<const parser::ModelIR> pIR = parser::parseModelFile(path, names[i], true);
        if (!pIR)
        {
            bSucceed = false;
            break;
        }

        *phDst[i] = ImportStaticMeshImmediate(*pIR->pMeshIR, pIR->pMaterialIRs, *pIR->pSkeletonIR, *pIR->pSkinIR);
        if (phDst[i]->IsNULL())
        {
            bSucceed = false;
            break;
        }
        FixedArray<ObjectHandle<RMaterialAsset>> hMaterials(static_cast<int32_t>(pIR->pMaterialIRs.size()));
        for (int32_t matIdx = 0; matIdx < hMaterials.GetSize(); ++matIdx)
        {
            hMaterials[matIdx] = ImportMaterialImmediate(*(pIR->pMaterialIRs[matIdx]), eMaterialAssetType::StandardLit);
            if (hMaterials[matIdx].IsNULL())
            {
                bSucceed = false;
                break;
            }
        }
        ResolveStaticMeshMaterials(*phDst[i]);
    }

    if (!bSucceed)
    {
        for (int i = 0; i < HO_ARRAY_SIZE(phDst); ++i)
        {
            DeleteAssetImmediate(*phDst[i]);
        }
        return false;
    }

    mbBuiltInAssetLoaded = true;
    return true;
}

const AssetBuiltInAssets& AssetSystem::GetBuiltInAsset() const
{
    HO_ASSERT(mbBuiltInAssetLoaded, "Built-in assets are not loaded yet.");
    return mBuiltInAssets;
}

ObjectHandle<RStaticMeshAsset> AssetSystem::ImportStaticMesh(
    const parser::MeshIR& srcMeshIR,
    const std::vector<std::unique_ptr<const parser::MaterialIR>>& pMaterialIRs,
    const parser::SkeletonIR& srcSkeletonIR,
    const parser::SkinIR& srcSkinIR)
{
    if (mPendingAssetImportQueue.IsFull())
    {
        HO_ASSERT(false, "Pending import queue is full. Check 'sPendingQueueSize'.");
        return ObjectHandle<RStaticMeshAsset>::sNULL;
    }

    const std::string nameStr =
        srcMeshIR.ResourcePath.GetParentPath().ToString() + "::StaticMesh::" + srcMeshIR.NameStr;
    ObjectHandle<RStaticMeshAsset> hNewAsset = CreateAsset<RStaticMeshAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RStaticMeshAsset>::sNULL;
    }

    AssetImportContext importCtx;
    importCtx.ImportType = eImportType::StaticMesh;
    importCtx.hDstAsset = hNewAsset;
    importCtx.BeginCount = 5;
    importCtx.StaticMeshContext.pMeshIR = &srcMeshIR;
    importCtx.StaticMeshContext.pMaterialIRs = &pMaterialIRs;
    importCtx.StaticMeshContext.pSkeletonIR = &srcSkeletonIR;
    importCtx.StaticMeshContext.pSkinIR = &srcSkinIR;

    mPendingAssetImportQueue.Push(importCtx);

    hNewAsset->TransferState = eAssetTransferState::PendingUpload;
    return hNewAsset;
}

ObjectHandle<RMorphTargetsAsset> AssetSystem::ImportMorphTargets(const parser::MeshIR& srcMeshIR)
{
    if (mPendingAssetImportQueue.IsFull())
    {
        HO_ASSERT(false, "Pending import queue is full. Check 'sPendingQueueSize'.");
        return ObjectHandle<RMorphTargetsAsset>::sNULL;
    }

    const std::string& nameStr =
        srcMeshIR.ResourcePath.RemovedExtension().ToString() + "::MorphTargets::" + srcMeshIR.NameStr;
    ObjectHandle<RMorphTargetsAsset> hNewAsset = CreateAsset<RMorphTargetsAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RMorphTargetsAsset>::sNULL;
    }

    AssetImportContext importCtx;
    importCtx.ImportType = eImportType::MorphTargets;
    importCtx.hDstAsset = hNewAsset;
    importCtx.BeginCount = 5;
    importCtx.MorphTargetsContext.pMeshIR = &srcMeshIR;

    mPendingAssetImportQueue.Push(importCtx);

    hNewAsset->TransferState = eAssetTransferState::PendingUpload;
    return hNewAsset;
}

ObjectHandle<RMorphAnimationAsset> AssetSystem::ImportMorphAnimation(const parser::AnimationIR& srcAnimIR)
{
    if (mPendingAssetImportQueue.IsFull())
    {
        HO_ASSERT(false, "Pending import queue is full. Check 'sPendingQueueSize'.");
        return ObjectHandle<RMorphAnimationAsset>::sNULL;
    }

    const std::string& nameStr =
        srcAnimIR.ResourcePath.GetParentPath().ToString() + "::MorphAnimation::" + srcAnimIR.NameStr;
    ObjectHandle<RMorphAnimationAsset> hNewAsset = CreateAsset<RMorphAnimationAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RMorphAnimationAsset>::sNULL;
    }

    AssetImportContext importCtx;
    importCtx.ImportType = eImportType::MorphAnim;
    importCtx.hDstAsset = hNewAsset;
    importCtx.BeginCount = 5;
    importCtx.MorphAnimContext.pAnimIR = &srcAnimIR;

    mPendingAssetImportQueue.Push(importCtx);

    hNewAsset->TransferState = eAssetTransferState::PendingUpload;
    return hNewAsset;
}

ObjectHandle<RMaterialAsset> AssetSystem::ImportMaterial(const parser::MaterialIR& srcMaterialIR,
                                                         eMaterialAssetType type)
{
    if (mPendingAssetImportQueue.IsFull())
    {
        HO_ASSERT(false, "Pending import queue is full. Check 'sPendingQueueSize'.");
        return ObjectHandle<RMaterialAsset>::sNULL;
    }

    const std::string nameStr =
        srcMaterialIR.ResourcePath.GetParentPath().ToString() + "::Material::" + srcMaterialIR.NameStr;
    ObjectHandle<RMaterialAsset> hNewAsset = CreateAsset<RMaterialAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RMaterialAsset>::sNULL;
    }

    AssetImportContext importCtx;
    importCtx.ImportType = eImportType::Material;
    importCtx.hDstAsset = hNewAsset;
    importCtx.BeginCount = 5;
    importCtx.MaterialContext.pMaterialIR = &srcMaterialIR;
    importCtx.MaterialContext.MaterialType = type;

    mPendingAssetImportQueue.Push(importCtx);

    hNewAsset->TransferState = eAssetTransferState::PendingUpload;
    return hNewAsset;
}

ObjectHandle<RMaterialAsset> AssetSystem::ImportTexturedMaterial(
    const parser::MaterialIR& srcMaterialIR,
    eMaterialAssetType type,
    const std::vector<std::unique_ptr<const parser::TextureIR>>& pSrcTextureIRs)
{
    if (mPendingAssetImportQueue.IsFull())
    {
        HO_ASSERT(false, "Pending import queue is full. Check 'sPendingQueueSize'.");
        return ObjectHandle<RMaterialAsset>::sNULL;
    }

    const std::string nameStr =
        srcMaterialIR.ResourcePath.GetParentPath().ToString() + "::Material::" + srcMaterialIR.NameStr;
    ObjectHandle<RMaterialAsset> hNewAsset = CreateAsset<RMaterialAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RMaterialAsset>::sNULL;
    }

    AssetImportContext importCtx;
    importCtx.ImportType = eImportType::TexturedMaterial;
    importCtx.hDstAsset = hNewAsset;
    importCtx.BeginCount = 5;
    importCtx.TexturedMaterialContext.pMaterialIR = &srcMaterialIR;
    importCtx.TexturedMaterialContext.MaterialType = type;
    importCtx.TexturedMaterialContext.pTextureIRs = &pSrcTextureIRs;

    mPendingAssetImportQueue.Push(importCtx);

    hNewAsset->TransferState = eAssetTransferState::PendingUpload;
    return hNewAsset;
}

ObjectHandle<RTextureAsset> AssetSystem::ImportTexture2D(const parser::TextureIR& srcTextureIR,
                                                         eTextureFormat dstFormat,
                                                         bool bGenerateMipmap)
{
    if (mPendingAssetImportQueue.IsFull())
    {
        HO_ASSERT(false, "Pending import queue is full. Check 'sPendingQueueSize'.");
        return ObjectHandle<RTextureAsset>::sNULL;
    }

    const std::string nameStr =
        srcTextureIR.ResourcePath.GetParentPath().ToString() + "::Texture2D::" + srcTextureIR.NameStr;
    ObjectHandle<RTextureAsset> hNewAsset = CreateAsset<RTextureAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RTextureAsset>::sNULL;
    }

    AssetImportContext importCtx;
    importCtx.ImportType = eImportType::Texture2D;
    importCtx.hDstAsset = hNewAsset;
    importCtx.BeginCount = 5;
    importCtx.Texture2DContext.pTextureIR = &srcTextureIR;
    importCtx.Texture2DContext.TextureDstFormat = dstFormat;
    importCtx.Texture2DContext.bGenerateMipmap = bGenerateMipmap;

    mPendingAssetImportQueue.Push(importCtx);

    hNewAsset->TransferState = eAssetTransferState::PendingUpload;
    return hNewAsset;
}

ObjectHandle<RTextureAsset> AssetSystem::ImportTextureCubemap(const std::string& nameStr,
                                                              const parser::TextureIR& srcPosX,
                                                              const parser::TextureIR& srcNegX,
                                                              const parser::TextureIR& srcPosY,
                                                              const parser::TextureIR& srcNegY,
                                                              const parser::TextureIR& srcPosZ,
                                                              const parser::TextureIR& srcNegZ,
                                                              eTextureFormat dstFormat,
                                                              bool bGenerateMipmap)
{
    if (mPendingAssetImportQueue.IsFull())
    {
        HO_ASSERT(false, "Pending import queue is full. Check 'sPendingQueueSize'.");
        return ObjectHandle<RTextureAsset>::sNULL;
    }

    const std::string texNameStr = srcPosX.ResourcePath.GetParentPath().ToString() + "::TextureCubeMap::" + nameStr;

    ObjectHandle<RTextureAsset> hNewAsset = CreateAsset<RTextureAsset>(StringHandle(texNameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RTextureAsset>::sNULL;
    }

    AssetImportContext importCtx;
    importCtx.ImportType = eImportType::TextureCubemap;
    importCtx.hDstAsset = hNewAsset;
    importCtx.BeginCount = 5;
    snprintf(importCtx.TextureCubemapContext.Name,
             HO_ARRAY_SIZE(importCtx.TextureCubemapContext.Name),
             "%s",
             nameStr.c_str());
    importCtx.TextureCubemapContext.pTexturePosX = &srcPosX;
    importCtx.TextureCubemapContext.pTextureNegX = &srcNegX;
    importCtx.TextureCubemapContext.pTexturePosY = &srcPosY;
    importCtx.TextureCubemapContext.pTextureNegY = &srcNegY;
    importCtx.TextureCubemapContext.pTexturePosZ = &srcPosZ;
    importCtx.TextureCubemapContext.pTextureNegZ = &srcNegZ;
    importCtx.TextureCubemapContext.TextureDstFormat = dstFormat;
    importCtx.TextureCubemapContext.bGenerateMipmap = bGenerateMipmap;

    mPendingAssetImportQueue.Push(importCtx);

    hNewAsset->TransferState = eAssetTransferState::PendingUpload;
    return hNewAsset;
}

ObjectHandle<RShaderAsset> AssetSystem::ImportShader(std::string* pOutErrorMsg,
                                                     const parser::ShaderIR& srcShaderIR,
                                                     eShaderStage stage,
                                                     eMaterialAssetType compatibleMaterialType)
{
    if (mPendingAssetImportQueue.IsFull())
    {
        HO_ASSERT(false, "Pending import queue is full. Check 'sPendingQueueSize'.");
        return ObjectHandle<RShaderAsset>::sNULL;
    }

    const std::string nameStr =
        srcShaderIR.ResourcePath.GetParentPath().ToString() + "::Shader::" + srcShaderIR.NameStr;

    ObjectHandle<RShaderAsset> hNewAsset = CreateAsset<RShaderAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RShaderAsset>::sNULL;
    }

    AssetImportContext importCtx;
    importCtx.ImportType = eImportType::Shader;
    importCtx.hDstAsset = hNewAsset;
    importCtx.BeginCount = 5;
    importCtx.ShaderContext.ppOutErrorMsg = &pOutErrorMsg;
    importCtx.ShaderContext.pSrcShaderIR = &srcShaderIR;
    importCtx.ShaderContext.Stage = stage;
    importCtx.ShaderContext.CompatibleMaterialType = compatibleMaterialType;

    mPendingAssetImportQueue.Push(importCtx);

    hNewAsset->TransferState = eAssetTransferState::PendingUpload;
    return hNewAsset;
}

ObjectHandle<RStaticMeshAsset> AssetSystem::ImportStaticMeshImmediate(
    const parser::MeshIR& srcMeshIR,
    const std::vector<std::unique_ptr<const parser::MaterialIR>>& pMaterialIRs,
    const parser::SkeletonIR& srcSkeletonIR,
    const parser::SkinIR& srcSkinIR)
{
    const std::string nameStr =
        srcMeshIR.ResourcePath.GetParentPath().ToString() + "::StaticMesh::" + srcMeshIR.NameStr;
    ObjectHandle<RStaticMeshAsset> hNewAsset = CreateAsset<RStaticMeshAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RStaticMeshAsset>::sNULL;
    }

    if (!importer::importStaticMesh(hNewAsset, srcMeshIR, pMaterialIRs, srcSkeletonIR, srcSkinIR))
    {
        DeleteAsset(hNewAsset);
        return ObjectHandle<RStaticMeshAsset>::sNULL;
    }

    hNewAsset->TransferState = eAssetTransferState::Ready;
    return hNewAsset;
}

ObjectHandle<RMorphTargetsAsset> AssetSystem::ImportMorphTargetsImmediate(const parser::MeshIR& srcMeshIR)
{
    const std::string& nameStr =
        srcMeshIR.ResourcePath.RemovedExtension().ToString() + "::MorphTargets::" + srcMeshIR.NameStr;
    ObjectHandle<RMorphTargetsAsset> hNewAsset = CreateAsset<RMorphTargetsAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RMorphTargetsAsset>::sNULL;
    }

    if (!importer::importMorphTargets(hNewAsset, srcMeshIR))
    {
        DeleteAsset(hNewAsset);
        return ObjectHandle<RMorphTargetsAsset>::sNULL;
    }

    hNewAsset->TransferState = eAssetTransferState::Ready;
    return hNewAsset;
}

ObjectHandle<RMorphAnimationAsset> AssetSystem::ImportMorphAnimationImmediate(const parser::AnimationIR& srcAnimIR)
{
    const std::string& nameStr =
        srcAnimIR.ResourcePath.GetParentPath().ToString() + "::MorphAnimation::" + srcAnimIR.NameStr;
    ObjectHandle<RMorphAnimationAsset> hNewAsset = CreateAsset<RMorphAnimationAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RMorphAnimationAsset>::sNULL;
    }

    if (!importer::importMorphAnimation(hNewAsset, srcAnimIR))
    {
        DeleteAsset(hNewAsset);
        return ObjectHandle<RMorphAnimationAsset>::sNULL;
    }

    hNewAsset->TransferState = eAssetTransferState::Ready;
    return hNewAsset;
}

ObjectHandle<RMaterialAsset> AssetSystem::ImportMaterialImmediate(const parser::MaterialIR& srcMaterialIR,
                                                                  eMaterialAssetType type)
{
    const std::string nameStr =
        srcMaterialIR.ResourcePath.GetParentPath().ToString() + "::Material::" + srcMaterialIR.NameStr;
    ObjectHandle<RMaterialAsset> hNewAsset = CreateAsset<RMaterialAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RMaterialAsset>::sNULL;
    }

    if (!importer::importMaterial(hNewAsset, srcMaterialIR, type))
    {
        DeleteAsset(hNewAsset);
        return ObjectHandle<RMaterialAsset>::sNULL;
    }

    hNewAsset->TransferState = eAssetTransferState::Ready;
    return hNewAsset;
}

ObjectHandle<RMaterialAsset> AssetSystem::ImportTexturedMaterialImmediate(
    const parser::MaterialIR& srcMaterialIR,
    eMaterialAssetType type,
    const std::vector<std::unique_ptr<const parser::TextureIR>>& pSrcTextureIRs)
{
    const std::string nameStr =
        srcMaterialIR.ResourcePath.GetParentPath().ToString() + "::Material::" + srcMaterialIR.NameStr;
    ObjectHandle<RMaterialAsset> hNewAsset = CreateAsset<RMaterialAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RMaterialAsset>::sNULL;
    }

    if (!importer::importTexturedMaterial(hNewAsset, srcMaterialIR, type, pSrcTextureIRs))
    {
        DeleteAsset(hNewAsset);
        return ObjectHandle<RMaterialAsset>::sNULL;
    }

    hNewAsset->TransferState = eAssetTransferState::Ready;
    return hNewAsset;
}

ObjectHandle<RTextureAsset> AssetSystem::ImportTexture2DImmediate(const parser::TextureIR& srcTextureIR,
                                                                  eTextureFormat dstFormat,
                                                                  bool bGenerateMipmap)
{
    const std::string nameStr =
        srcTextureIR.ResourcePath.GetParentPath().ToString() + "::Texture2D::" + srcTextureIR.NameStr;
    ObjectHandle<RTextureAsset> hNewAsset = CreateAsset<RTextureAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RTextureAsset>::sNULL;
    }

    if (!importer::importTexture2D(hNewAsset, srcTextureIR, dstFormat, bGenerateMipmap))
    {
        DeleteAsset(hNewAsset);
        return ObjectHandle<RTextureAsset>::sNULL;
    }

    hNewAsset->TransferState = eAssetTransferState::Ready;
    return hNewAsset;
}

ObjectHandle<RTextureAsset> AssetSystem::ImportTextureCubemapImmediate(const std::string& nameStr,
                                                                       const parser::TextureIR& srcPosX,
                                                                       const parser::TextureIR& srcNegX,
                                                                       const parser::TextureIR& srcPosY,
                                                                       const parser::TextureIR& srcNegY,
                                                                       const parser::TextureIR& srcPosZ,
                                                                       const parser::TextureIR& srcNegZ,
                                                                       eTextureFormat dstFormat,
                                                                       bool bGenerateMipmap)
{
    const std::string texNameStr = srcPosX.ResourcePath.GetParentPath().ToString() + "::TextureCubeMap::" + nameStr;

    ObjectHandle<RTextureAsset> hNewAsset = CreateAsset<RTextureAsset>(StringHandle(texNameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RTextureAsset>::sNULL;
    }

    if (!importer::importTextureCubemap(
            hNewAsset, nameStr, srcPosX, srcNegX, srcPosY, srcNegY, srcPosZ, srcNegZ, dstFormat, bGenerateMipmap))
    {
        DeleteAsset(hNewAsset);
        return ObjectHandle<RTextureAsset>::sNULL;
    }

    hNewAsset->TransferState = eAssetTransferState::Ready;
    return hNewAsset;
}

ObjectHandle<RShaderAsset> AssetSystem::ImportShaderImmediate(std::string* pOutErrorMsg,
                                                              const parser::ShaderIR& srcShaderIR,
                                                              eShaderStage stage,
                                                              eMaterialAssetType compatibleMaterialType)
{
    const std::string nameStr =
        srcShaderIR.ResourcePath.GetParentPath().ToString() + "::Shader::" + srcShaderIR.NameStr;

    ObjectHandle<RShaderAsset> hNewAsset = CreateAsset<RShaderAsset>(StringHandle(nameStr));
    if (hNewAsset.IsNULL())
    {
        return ObjectHandle<RShaderAsset>::sNULL;
    }

    if (!importer::importShader(hNewAsset, pOutErrorMsg, srcShaderIR, stage, compatibleMaterialType))
    {
        DeleteAsset(hNewAsset);
        return ObjectHandle<RShaderAsset>::sNULL;
    }

    hNewAsset->TransferState = eAssetTransferState::Ready;
    return hNewAsset;
}

void AssetSystem::DeleteAsset(ObjectHandle<RIAsset> hAsset)
{
    if (hAsset.IsNULL())
    {
        return;
    }
    if (mPendingAssetDeleteQueue.IsFull())
    {
        HO_ASSERT(false, "Pending import queue is full. Check 'sPendingQueueSize'.");
        return;
    }

    mPendingAssetDeleteQueue.Push(hAsset);
}

void AssetSystem::DeleteAssetImmediate(ObjectHandle<RIAsset> hAsset)
{
    if (hAsset.IsNULL())
    {
        return;
    }

    mNameToAssetMap.erase(hAsset->hName);
    mhAssetPool->DeleteObject(hAsset);
}

void AssetSystem::ResolveStaticMeshMaterials(ObjectHandle<RStaticMeshAsset> hStaticMesh) const
{
    HO_ASSERT(hStaticMesh.IsValid(), "Static mesh is invalid.");
    for (auto& subMesh : hStaticMesh->SubMeshes)
    {
        if (subMesh.hRenderMaterialName.IsNULL())
        {
            continue;
        }
        auto it = mNameToAssetMap.find(subMesh.hRenderMaterialName);
        if (it == mNameToAssetMap.end())
        {
            continue;
        }
        subMesh.hRenderMaterial = it->second.Cast<RMaterialAsset>();
    }
}

void AssetSystem::ResolveMaterialTextures(ObjectHandle<RMaterialAsset> hMaterial) const
{
    HO_ASSERT(hMaterial.IsValid(), "Material is invalid.");
    for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
    {
        if (hMaterial->hTextureNames[i].IsNULL())
        {
            continue;
        }
        auto it = mNameToAssetMap.find(hMaterial->hTextureNames[i]);
        if (it == mNameToAssetMap.end())
        {
            continue;
        }
        hMaterial->hTextures[i] = it->second.Cast<RTextureAsset>();
    }
}

bool AssetSystem::IsUploaded(ObjectHandle<RIAsset> hAsset) const
{
    return hAsset.IsValid() && hAsset->TransferState == eAssetTransferState::Ready;
}

AssetSystem::AssetSystem()
  : mPendingAssetImportQueue(sPendingQueueSize)
  , mPendingAssetDeleteQueue(sPendingQueueSize)
  , mhAssetPool(ObjectSystem::GetInstance().CreateObjectManager())
{
    HO_ASSERT(!spInstance, "Asset system muse be created by 'createInstance' function.");
}

AssetSystem::~AssetSystem()
{
    ObjectSystem::GetInstance().DestroyObjectManager(mhAssetPool);
}

void AssetSystem::createInstance()
{
    if (spInstance)
    {
        HO_ASSERT(false, "Asset system is already created.");
    }
    else
    {
        spInstance = new AssetSystem();
    }
}

void AssetSystem::deleteInstance()
{
    if (!spInstance)
    {
        HO_ASSERT(false, "Asset system is not created.");
    }
    else
    {
        delete spInstance;
        spInstance = nullptr;
    }
}

bool AssetSystem::init()
{
    if (!spInstance)
    {
        HO_ASSERT(false, "Asset system is not created.");
        return false;
    }
    else
    {
        mhAssetPool->Expand<RStaticMeshAsset>(128);
        mhAssetPool->Expand<RMorphTargetsAsset>(128);
        mhAssetPool->Expand<RMorphAnimationAsset>(128);
        mhAssetPool->Expand<RMaterialAsset>(128);
        mhAssetPool->Expand<RTextureAsset>(128);
        mhAssetPool->Expand<RShaderAsset>(128);
        return true;
    }
}

void AssetSystem::shutdown() {}

void AssetSystem::uploadPendingAssets()
{
    if (!mPendingAssetImportQueue.IsEmpty())
    {
        bool bSucceed = true;
        AssetImportContext importCtx = mPendingAssetImportQueue.Front();
        if (importCtx.hDstAsset.IsValid())
        {
            if (importCtx.BeginCount > 0)
            {
                --importCtx.BeginCount;
                mPendingAssetImportQueue.Push(importCtx);
            }
            else
            {
                switch (importCtx.ImportType)
                {
                    case eImportType::StaticMesh:
                        HO_ASSERT(importCtx.StaticMeshContext.pMeshIR && importCtx.StaticMeshContext.pMaterialIRs &&
                                      importCtx.StaticMeshContext.pSkeletonIR && importCtx.StaticMeshContext.pSkinIR,
                                  "Invalid context.");
                        bSucceed = importer::importStaticMesh(importCtx.hDstAsset.Cast<RStaticMeshAsset>(),
                                                              *importCtx.StaticMeshContext.pMeshIR,
                                                              *importCtx.StaticMeshContext.pMaterialIRs,
                                                              *importCtx.StaticMeshContext.pSkeletonIR,
                                                              *importCtx.StaticMeshContext.pSkinIR);
                        break;

                    case eImportType::MorphTargets:
                        HO_ASSERT(importCtx.MorphTargetsContext.pMeshIR, "Invalid context.");
                        bSucceed = importer::importMorphTargets(importCtx.hDstAsset.Cast<RMorphTargetsAsset>(),
                                                                *importCtx.MorphTargetsContext.pMeshIR);
                        break;

                    case eImportType::MorphAnim:
                        HO_ASSERT(importCtx.MorphAnimContext.pAnimIR, "Invalid context.");
                        bSucceed = importer::importMorphAnimation(importCtx.hDstAsset.Cast<RMorphAnimationAsset>(),
                                                                  *importCtx.MorphAnimContext.pAnimIR);
                        break;

                    case eImportType::Material:
                        HO_ASSERT(importCtx.MaterialContext.pMaterialIR, "Invalid context.");
                        bSucceed = importer::importMaterial(importCtx.hDstAsset.Cast<RMaterialAsset>(),
                                                            *importCtx.MaterialContext.pMaterialIR,
                                                            importCtx.MaterialContext.MaterialType);
                        break;

                    case eImportType::TexturedMaterial:
                        HO_ASSERT(importCtx.TexturedMaterialContext.pMaterialIR &&
                                      importCtx.TexturedMaterialContext.pTextureIRs,
                                  "Invalid context.");
                        bSucceed = importer::importTexturedMaterial(importCtx.hDstAsset.Cast<RMaterialAsset>(),
                                                                    *importCtx.TexturedMaterialContext.pMaterialIR,
                                                                    importCtx.TexturedMaterialContext.MaterialType,
                                                                    *importCtx.TexturedMaterialContext.pTextureIRs);
                        break;

                    case eImportType::Texture2D:
                        HO_ASSERT(importCtx.Texture2DContext.pTextureIR, "Invalid context.");
                        bSucceed = importer::importTexture2D(importCtx.hDstAsset.Cast<RTextureAsset>(),
                                                             *importCtx.Texture2DContext.pTextureIR,
                                                             importCtx.Texture2DContext.TextureDstFormat,
                                                             importCtx.Texture2DContext.bGenerateMipmap);
                        break;

                    case eImportType::TextureCubemap:
                        HO_ASSERT(importCtx.TextureCubemapContext.pTexturePosX &&
                                      importCtx.TextureCubemapContext.pTextureNegX &&
                                      importCtx.TextureCubemapContext.pTexturePosY &&
                                      importCtx.TextureCubemapContext.pTextureNegY &&
                                      importCtx.TextureCubemapContext.pTexturePosZ &&
                                      importCtx.TextureCubemapContext.pTextureNegZ,
                                  "Invalid context.");
                        bSucceed = importer::importTextureCubemap(importCtx.hDstAsset.Cast<RTextureAsset>(),
                                                                  std::string(importCtx.TextureCubemapContext.Name),
                                                                  *importCtx.TextureCubemapContext.pTexturePosX,
                                                                  *importCtx.TextureCubemapContext.pTextureNegX,
                                                                  *importCtx.TextureCubemapContext.pTexturePosY,
                                                                  *importCtx.TextureCubemapContext.pTextureNegY,
                                                                  *importCtx.TextureCubemapContext.pTexturePosZ,
                                                                  *importCtx.TextureCubemapContext.pTextureNegZ,
                                                                  importCtx.TextureCubemapContext.TextureDstFormat,
                                                                  importCtx.TextureCubemapContext.bGenerateMipmap);
                        break;

                    case eImportType::Shader:
                        HO_ASSERT(importCtx.ShaderContext.pSrcShaderIR, "Invalid context.");
                        bSucceed = importer::importShader(importCtx.hDstAsset.Cast<RShaderAsset>(),
                                                          *importCtx.ShaderContext.ppOutErrorMsg,
                                                          *importCtx.ShaderContext.pSrcShaderIR,
                                                          importCtx.ShaderContext.Stage,
                                                          importCtx.ShaderContext.CompatibleMaterialType);
                        break;

                    default:
                        HO_ASSERT(false, "Invalid import type.");
                        bSucceed = false;
                        break;
                }

                importCtx.hDstAsset->TransferState =
                    bSucceed ? eAssetTransferState::Ready : eAssetTransferState::Failed;
            }
        }
        mPendingAssetImportQueue.Pop();
    }
}

void AssetSystem::deletePendingAssets()
{
    while (!mPendingAssetDeleteQueue.IsEmpty())
    {
        const ObjectHandle<RIAsset> hAsset = mPendingAssetDeleteQueue.Front();
        mNameToAssetMap.erase(hAsset->hName);
        mhAssetPool->DeleteObject(hAsset);
        mPendingAssetDeleteQueue.Pop();
    }
}

AssetSystem* AssetSystem::spInstance = nullptr;
} // namespace ho