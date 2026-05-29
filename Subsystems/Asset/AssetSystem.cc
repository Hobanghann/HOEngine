#include "AssetSystem.h"

#include "Core/Log/Logger.h"

namespace ho
{
AssetSystem& AssetSystem::GetInstance()
{
    HO_ASSERT(spInstance, "Asset system is not created");
    return *spInstance;
}

StaticMeshHandle AssetSystem::AddStaticMesh(StaticMeshAsset&& staticMesh)
{
    if (mNameToStaticMeshMap.find(staticMesh.hName) != mNameToStaticMeshMap.end())
    {
        HO_LOG_INFO("[AssetSystem] Asset '%s' already exists. Skipping.", staticMesh.hName.Get()->c_str());
        return StaticMeshHandle::sNULL;
    }
    const uint32_t idx = mStaticMeshPool.Add(std::move(staticMesh));
    const StaticMeshHandle hNewStaticMesh = StaticMeshHandle(&mStaticMeshPool, idx);
    mNameToStaticMeshMap.insert({hNewStaticMesh.Get()->hName, hNewStaticMesh});
    return hNewStaticMesh;
}

MaterialHandle AssetSystem::AddMaterial(MaterialAsset&& material)
{
    if (mNameToMaterialMap.find(material.hName) != mNameToMaterialMap.end())
    {
        HO_LOG_INFO("[AssetSystem] Asset '%s' already exists. Skipping.", material.hName.Get()->c_str());
        return MaterialHandle::sNULL;
    }
    const uint32_t idx = mMaterialPool.Add(std::move(material));
    const MaterialHandle hNewMaterial = MaterialHandle(&mMaterialPool, idx);
    mNameToMaterialMap.insert({hNewMaterial.Get()->hName, hNewMaterial});
    return hNewMaterial;
}

TextureHandle AssetSystem::AddTexture(TextureAsset&& texture)
{
    if (mNameToTextureMap.find(texture.hName) != mNameToTextureMap.end())
    {
        HO_LOG_INFO("[AssetSystem] Asset '%s' already exists. Skipping.", texture.hName.Get()->c_str());
        return TextureHandle::sNULL;
    }
    const uint32_t idx = mTexturePool.Add(std::move(texture));
    const TextureHandle hNewTexture = TextureHandle(&mTexturePool, idx);
    mNameToTextureMap.insert({hNewTexture.Get()->hName, hNewTexture});
    return hNewTexture;
}

ShaderHandle AssetSystem::AddShader(ShaderAsset&& shader)
{
    if (mNameToShaderMap.find(shader.hName) != mNameToShaderMap.end())
    {
        HO_LOG_INFO("[AssetSystem] Asset '%s' already exists. Skipping.", shader.hName.Get()->c_str());
        return ShaderHandle::sNULL;
    }
    const uint32_t idx = mShaderPool.Add(std::move(shader));
    const ShaderHandle hNewShader = ShaderHandle(&mShaderPool, idx);
    mNameToShaderMap.insert({hNewShader.Get()->hName, hNewShader});
    return hNewShader;
}

bool AssetSystem::ResolveStaticMeshMaterials(StaticMeshHandle hStaticMesh)
{
    HO_ASSERT(hStaticMesh.IsValid(), "Static mesh is invalid.");
    StaticMeshAsset* pStaticMesh = hStaticMesh.Get();
    bool bSuccess = true;
    for (auto& subMesh : pStaticMesh->SubMeshes)
    {
        if (subMesh.hRenderMaterialName.IsNULL())
        {
            continue;
        }
        auto it = mNameToMaterialMap.find(subMesh.hRenderMaterialName);
        if (it == mNameToMaterialMap.end())
        {
            HO_LOG_ERROR("[AssetSystem] Material '%s' doesn't exist for StaticMesh '%s'. Skipping resolution.",
                         subMesh.hRenderMaterialName.Get()->c_str(),
                         pStaticMesh->hName.Get()->c_str());
            bSuccess = false;
            continue;
        }
        subMesh.hRenderMaterial = it->second;
    }
    return bSuccess;
}

bool AssetSystem::ResolveMaterialTextures(MaterialHandle hMaterial)
{
    HO_ASSERT(hMaterial.IsValid(), "Material is invalid.");
    MaterialAsset* pMaterial = hMaterial.Get();
    bool bSuccess = true;
    for (int32_t i = 0; i < static_cast<int32_t>(eMaterialTextureUsage::Last); ++i)
    {
        if (pMaterial->hTextureNames[i].IsNULL())
        {
            continue;
        }
        auto it = mNameToTextureMap.find(pMaterial->hTextureNames[i]);
        if (it == mNameToTextureMap.end())
        {
            HO_LOG_ERROR("[AssetSystem] Texture '%s' doesn't exist for Material '%s'. Skipping resolution.",
                         pMaterial->hTextureNames[i].Get()->c_str(),
                         pMaterial->hName.Get()->c_str());
            bSuccess = false;
            continue;
        }
        pMaterial->hTextures[i] = it->second;
    }
    return bSuccess;
}

bool AssetSystem::DeleteStaticMesh(StaticMeshHandle hStaticMesh)
{
    mNameToStaticMeshMap.erase(hStaticMesh.Get()->hName);
    return mStaticMeshPool.Remove(hStaticMesh.GetIndex());
}

bool AssetSystem::DeleteMaterial(MaterialHandle hMaterial)
{
    mNameToMaterialMap.erase(hMaterial.Get()->hName);
    return mMaterialPool.Remove(hMaterial.GetIndex());
}

bool AssetSystem::DeleteTexture(TextureHandle hTexture)
{
    mNameToTextureMap.erase(hTexture.Get()->hName);
    return mTexturePool.Remove(hTexture.GetIndex());
}

bool AssetSystem::DeleteShader(ShaderHandle hShader)
{
    mNameToShaderMap.erase(hShader.Get()->hName);
    return mShaderPool.Remove(hShader.GetIndex());
}

bool AssetSystem::HasStaticMesh(StaticMeshHandle hStaticMesh) const
{
    return mStaticMeshPool.Has(hStaticMesh.GetIndex());
}

bool AssetSystem::HasMaterial(MaterialHandle hMaterial) const
{
    return mMaterialPool.Has(hMaterial.GetIndex());
}

bool AssetSystem::HasTexture(TextureHandle hTexture) const
{
    return mTexturePool.Has(hTexture.GetIndex());
}

bool AssetSystem::HasShader(ShaderHandle hShader) const
{
    return mShaderPool.Has(hShader.GetIndex());
}

StaticMeshHandle AssetSystem::GetStaticMeshHandle(StringHandle hName) const
{
    auto it = mNameToStaticMeshMap.find(hName);
    if (it == mNameToStaticMeshMap.end())
    {
        return StaticMeshHandle::sNULL;
    }
    return it->second;
}

MaterialHandle AssetSystem::GetMaterialHandle(StringHandle hName) const
{
    auto it = mNameToMaterialMap.find(hName);
    if (it == mNameToMaterialMap.end())
    {
        return MaterialHandle::sNULL;
    }
    return it->second;
}

TextureHandle AssetSystem::GetTextureHandle(StringHandle hName) const
{
    auto it = mNameToTextureMap.find(hName);
    if (it == mNameToTextureMap.end())
    {
        return TextureHandle::sNULL;
    }
    return it->second;
}

ShaderHandle AssetSystem::GetShaderHandle(StringHandle hName) const
{
    auto it = mNameToShaderMap.find(hName);
    if (it == mNameToShaderMap.end())
    {
        return ShaderHandle::sNULL;
    }
    return it->second;
}

AssetSystem::AssetSystem()
{
    HO_ASSERT(!spInstance, "Asset system muse be created by 'createInstance' function.");
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
        // add default assets?

        return true;
    }
}

void AssetSystem::shutdown() {}

AssetSystem* AssetSystem::spInstance = nullptr;
} // namespace ho