#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "AssetBuiltInAssets.h"
#include "Assets.h"
#include "Core/String/StringHandle.h"
#include "Core/Templates/FixedQueue.h"
#include "Subsystems/Object/ObjectHandle.h"
#include "Subsystems/Object/ObjectManager.h"

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
struct ShaderIR;
} // namespace parser

class Engine;

class AssetSystem final
{
    friend Engine;

  public:
    AssetSystem(const AssetSystem&) = delete;
    AssetSystem& operator=(const AssetSystem&) = delete;

    static AssetSystem& GetInstance();

    bool LoadBuiltInAssets();
    const AssetBuiltInAssets& GetBuiltInAsset() const;

    // Creates and returns an Asset in the PendingUpload state.
    // The Asset does not own any resources and has not been uploaded yet.
    template <typename T>
    ObjectHandle<T> CreateAsset(StringHandle hName);

    // Queues an Asset import for deferred processing.
    // The returned handle refers to an Asset in the PendingUpload state.
    // The Asset transitions to Ready if the import succeeds, or Failed if it fails.
    //
    // TODO: Import functions share and reference IRs returned by the parser.
    // Currently, designing the import context to own the parser IRs and directly
    // manage their lifetimes is non-trivial. Therefore, the caller must keep the
    // IRs alive until the AssetSystem has finished processing the import context.
    // This lifetime management is cumbersome and should be improved in the future.
    ObjectHandle<RStaticMeshAsset> ImportStaticMesh(
        const parser::MeshIR& srcMeshIR,
        const std::vector<std::unique_ptr<const parser::MaterialIR>>& pMaterialIRs,
        const parser::SkeletonIR& srcSkeletonIR,
        const parser::SkinIR& srcSkinIR);
    ObjectHandle<RMorphTargetsAsset> ImportMorphTargets(const parser::MeshIR& srcMeshIR);
    ObjectHandle<RMorphAnimationAsset> ImportMorphAnimation(const parser::AnimationIR& srcAnimIR);
    ObjectHandle<RMaterialAsset> ImportMaterial(const parser::MaterialIR& srcMaterialIR, eMaterialAssetType type);
    ObjectHandle<RMaterialAsset> ImportTexturedMaterial(
        const parser::MaterialIR& srcMaterialIR,
        eMaterialAssetType type,
        const std::vector<std::unique_ptr<const parser::TextureIR>>& pSrcTextureIRs);
    ObjectHandle<RTextureAsset> ImportTexture2D(const parser::TextureIR& srcTextureIR,
                                                eTextureFormat dstFormat,
                                                bool bGenerateMipmap);
    ObjectHandle<RTextureAsset> ImportTextureCubemap(const std::string& nameStr,
                                                     const parser::TextureIR& srcPosX,
                                                     const parser::TextureIR& srcNegX,
                                                     const parser::TextureIR& srcPosY,
                                                     const parser::TextureIR& srcNegY,
                                                     const parser::TextureIR& srcPosZ,
                                                     const parser::TextureIR& srcNegZ,
                                                     eTextureFormat dstFormat,
                                                     bool bGenerateMipmap);
    ObjectHandle<RShaderAsset> ImportShader(std::string* pOutErrorMsg,
                                            const parser::ShaderIR& srcShaderIR,
                                            eShaderStage stage,
                                            eMaterialAssetType compatibleMaterialType);

    ObjectHandle<RStaticMeshAsset> ImportStaticMeshImmediate(
        const parser::MeshIR& srcMeshIR,
        const std::vector<std::unique_ptr<const parser::MaterialIR>>& pMaterialIRs,
        const parser::SkeletonIR& srcSkeletonIR,
        const parser::SkinIR& srcSkinIR);
    ObjectHandle<RMorphTargetsAsset> ImportMorphTargetsImmediate(const parser::MeshIR& srcMeshIR);
    ObjectHandle<RMorphAnimationAsset> ImportMorphAnimationImmediate(const parser::AnimationIR& srcAnimIR);
    ObjectHandle<RMaterialAsset> ImportMaterialImmediate(const parser::MaterialIR& srcMaterialIR,
                                                         eMaterialAssetType type);
    ObjectHandle<RMaterialAsset> ImportTexturedMaterialImmediate(
        const parser::MaterialIR& srcMaterialIR,
        eMaterialAssetType type,
        const std::vector<std::unique_ptr<const parser::TextureIR>>& pSrcTextureIRs);
    ObjectHandle<RTextureAsset> ImportTexture2DImmediate(const parser::TextureIR& srcTextureIR,
                                                         eTextureFormat dstFormat,
                                                         bool bGenerateMipmap);
    ObjectHandle<RTextureAsset> ImportTextureCubemapImmediate(const std::string& nameStr,
                                                              const parser::TextureIR& srcPosX,
                                                              const parser::TextureIR& srcNegX,
                                                              const parser::TextureIR& srcPosY,
                                                              const parser::TextureIR& srcNegY,
                                                              const parser::TextureIR& srcPosZ,
                                                              const parser::TextureIR& srcNegZ,
                                                              eTextureFormat dstFormat,
                                                              bool bGenerateMipmap);
    ObjectHandle<RShaderAsset> ImportShaderImmediate(std::string* pOutErrorMsg,
                                                     const parser::ShaderIR& srcShaderIR,
                                                     eShaderStage stage,
                                                     eMaterialAssetType compatibleMaterialType);

    void DeleteAsset(ObjectHandle<RIAsset> hAsset);

    void DeleteAssetImmediate(ObjectHandle<RIAsset> hAsset);

    template <typename T>
    ObjectHandle<T> GetAssetByName(StringHandle hName) const;

    bool IsUploaded(ObjectHandle<RIAsset> hAsset) const;

    // Binds material assets to the static mesh asset using material binding names and assets registered in the asset
    // system.
    void ResolveStaticMeshMaterials(ObjectHandle<RStaticMeshAsset> hStaticMesh) const;

    // Binds texture assets to the material asset using texture binding names and assets registered in the asset system.
    void ResolveMaterialTextures(ObjectHandle<RMaterialAsset> hMaterial) const;

    template <typename TAsset>
    ObjectContainer<TAsset> GetAssets();

    template <typename TAsset>
    ObjectContainer<const TAsset> GetAssets() const;

    template <typename TAsset>
    int32_t GetAssetCount() const;

  private:
    static const int32_t sPendingQueueSize = 256;

    enum class eImportType
    {
        None,
        StaticMesh,
        MorphTargets,
        MorphAnim,
        Material,
        TexturedMaterial,
        Texture2D,
        TextureCubemap,
        Shader,
    };

    struct StaticMeshImportContext
    {
        const parser::MeshIR* pMeshIR = nullptr;
        const std::vector<std::unique_ptr<const parser::MaterialIR>>* pMaterialIRs = nullptr;
        const parser::SkeletonIR* pSkeletonIR = nullptr;
        const parser::SkinIR* pSkinIR = nullptr;
    };

    struct MorphTargetsImportContext
    {
        const parser::MeshIR* pMeshIR = nullptr;
    };

    struct MorphAnimImportContext
    {
        const parser::AnimationIR* pAnimIR = nullptr;
    };

    struct MaterialImportContext
    {
        const parser::MaterialIR* pMaterialIR = nullptr;
        eMaterialAssetType MaterialType = eMaterialAssetType::None;
    };

    struct TexturedMaterialImportContext
    {
        const parser::MaterialIR* pMaterialIR = nullptr;
        eMaterialAssetType MaterialType = eMaterialAssetType::None;
        const std::vector<std::unique_ptr<const parser::TextureIR>>* pTextureIRs = nullptr;
    };

    struct Texture2DImportContext
    {
        const parser::TextureIR* pTextureIR = nullptr;
        eTextureFormat TextureDstFormat = eTextureFormat::None;
        bool bGenerateMipmap = false;
    };

    struct TextureCubemapImportContext
    {
        char Name[128] = {};
        const parser::TextureIR* pTexturePosX = nullptr;
        const parser::TextureIR* pTextureNegX = nullptr;
        const parser::TextureIR* pTexturePosY = nullptr;
        const parser::TextureIR* pTextureNegY = nullptr;
        const parser::TextureIR* pTexturePosZ = nullptr;
        const parser::TextureIR* pTextureNegZ = nullptr;
        eTextureFormat TextureDstFormat = eTextureFormat::None;
        bool bGenerateMipmap = false;
    };

    struct ShaderImportContext
    {
        std::string** ppOutErrorMsg = nullptr;
        const parser::ShaderIR* pSrcShaderIR = nullptr;
        eShaderStage Stage = eShaderStage::None;
        eMaterialAssetType CompatibleMaterialType = eMaterialAssetType::None;
    };

    struct AssetImportContext
    {
        eImportType ImportType = eImportType::None;
        ObjectHandle<RIAsset> hDstAsset;
        int32_t BeginCount = 5;

        union
        {
            StaticMeshImportContext StaticMeshContext = {};
            MorphTargetsImportContext MorphTargetsContext;
            MorphAnimImportContext MorphAnimContext;
            MaterialImportContext MaterialContext;
            TexturedMaterialImportContext TexturedMaterialContext;
            Texture2DImportContext Texture2DContext;
            TextureCubemapImportContext TextureCubemapContext;
            ShaderImportContext ShaderContext;
        };
    };

    AssetSystem();
    ~AssetSystem();

    static void createInstance();
    static void deleteInstance();

    bool init();
    void shutdown();

    void uploadPendingAssets();
    void deletePendingAssets();

    FixedQueue<AssetImportContext> mPendingAssetImportQueue;

    FixedQueue<ObjectHandle<RIAsset>> mPendingAssetDeleteQueue;

    ObjectManagerHandle mhAssetPool;

    std::unordered_map<StringHandle, ObjectHandle<RIAsset>> mNameToAssetMap;

    bool mbBuiltInAssetLoaded = false;
    AssetBuiltInAssets mBuiltInAssets;

    static AssetSystem* spInstance;
};

template <typename T>
ObjectHandle<T> AssetSystem::CreateAsset(StringHandle hName)
{
    static_assert(std::is_base_of_v<RIAsset, std::remove_const_t<T>>, "T is not Asset drived class.");

    if (mNameToAssetMap.find(hName) != mNameToAssetMap.end())
    {
        return ObjectHandle<T>::sNULL;
    }

    ObjectHandle<T> hNewAsset = mhAssetPool->NewObject<T>();
    if (hNewAsset.IsValid())
    {
        hNewAsset->hName = hName;
        hNewAsset->TransferState = eAssetTransferState::PendingUpload;
        mNameToAssetMap[hNewAsset->hName] = hNewAsset.template Cast<RIAsset>();
    }

    return hNewAsset;
}

template <typename T>
ObjectHandle<T> AssetSystem::GetAssetByName(StringHandle hName) const
{
    auto it = mNameToAssetMap.find(hName);
    if (it == mNameToAssetMap.end())
    {
        return ObjectHandle<T>::sNULL;
    }
    return it->second.Cast<T>();
}

template <typename TAsset>
ObjectContainer<TAsset> AssetSystem::GetAssets()
{
    return mhAssetPool->GetObjects<TAsset>();
}

template <typename TAsset>
ObjectContainer<const TAsset> AssetSystem::GetAssets() const
{
    return mhAssetPool->GetObjects<const TAsset>();
}

template <typename TAsset>
int32_t AssetSystem::GetAssetCount() const
{
    return mhAssetPool->GetObjectCount<TAsset>();
}

} // namespace ho