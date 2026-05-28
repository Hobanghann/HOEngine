#pragma once

#include <unordered_map>

#include "Assets.h"
#include "Core/Templates/GlobalPoolIndex.h"
#include "Core/Templates/ObjectPool.h"

namespace ho
{
class Engine;

class AssetSystem final
{
    friend Engine;

  public:
    AssetSystem(const AssetSystem&) = delete;
    AssetSystem& operator=(const AssetSystem&) = delete;

    static AssetSystem& GetInstance();

    StaticMeshHandle AddStaticMesh(StaticMeshAsset&& staticMesh);
    MaterialHandle AddMaterial(MaterialAsset&& material);
    TextureHandle AddTexture(TextureAsset&& texture);
    ShaderHandle AddShader(ShaderAsset&& shader);

    bool ResolveMaterialTextures(MaterialHandle hMaterial);

    bool DeleteStaticMesh(StaticMeshHandle hStaticMesh);
    bool DeleteMaterial(MaterialHandle hMaterial);
    bool DeleteTexture(TextureHandle hTexture);
    bool DeleteShader(ShaderHandle hShader);

    [[nodiscard]] bool HasStaticMesh(StaticMeshHandle hStaticMesh) const;
    [[nodiscard]] bool HasMaterial(MaterialHandle hMaterial) const;
    [[nodiscard]] bool HasTexture(TextureHandle hTexture) const;
    [[nodiscard]] bool HasShader(ShaderHandle hShader) const;

    [[nodiscard]] StaticMeshHandle GetStaticMeshHandle(StringHandle hName) const;
    [[nodiscard]] MaterialHandle GetMaterialHandle(StringHandle hName) const;
    [[nodiscard]] TextureHandle GetTextureHandle(StringHandle hName) const;
    [[nodiscard]] ShaderHandle GetShaderHandle(StringHandle hName) const;

  private:
    AssetSystem();
    ~AssetSystem() = default;

    static void createInstance();
    static void deleteInstance();

    bool init();
    void shutdown();

    ObjectPool<StaticMeshAsset> mStaticMeshPool;
    ObjectPool<MaterialAsset> mMaterialPool;
    ObjectPool<TextureAsset> mTexturePool;
    ObjectPool<ShaderAsset> mShaderPool;

    std::unordered_map<StringHandle, StaticMeshHandle> mNameToStaticMeshMap;
    std::unordered_map<StringHandle, MaterialHandle> mNameToMaterialMap;
    std::unordered_map<StringHandle, TextureHandle> mNameToTextureMap;
    std::unordered_map<StringHandle, ShaderHandle> mNameToShaderMap;

    static AssetSystem* spInstance;
};
} // namespace ho