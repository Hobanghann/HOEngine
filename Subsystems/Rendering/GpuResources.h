#pragma once

#include <cstdint>

#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector4.h"
#include "Core/Templates/ObjectPool.h"
#include "Core/Templates/WeakLocalPoolIndex.h"
#include "Subsystems/Asset/AssetDefs.h"

namespace ho
{
struct StaticMeshAsset;
using StaticMeshHandle = WeakLocalPoolIndex<StaticMeshAsset, ObjectPool>;
struct MaterialAsset;
using MaterialHandle = WeakLocalPoolIndex<MaterialAsset, ObjectPool>;
struct ShaderAsset;
using ShaderHandle = WeakLocalPoolIndex<ShaderAsset, ObjectPool>;
struct TextureAsset;
using TextureHandle = WeakLocalPoolIndex<TextureAsset, ObjectPool>;

struct GpuStaticMesh;
using GpuStaticMeshHandle = WeakLocalPoolIndex<GpuStaticMesh, ObjectPool>;
struct GpuMaterial;
using GpuMaterialHandle = WeakLocalPoolIndex<GpuMaterial, ObjectPool>;
struct GpuTexture;
using GpuTextureHandle = WeakLocalPoolIndex<GpuTexture, ObjectPool>;
struct GpuShader;
using GpuShaderHandle = WeakLocalPoolIndex<GpuShader, ObjectPool>;

// 'uint32_t NativeHandlePoolIndex;' used to retrieve API-specific native handles from the rendering system's
// NativeHandle object pool. (index 0 means NULL.)
//
// Since different graphics APIs (GL, DX, Vulkan) require different types and quantities
// of native resources (e.g., VAO/VBO vs PipelineStateObject), this struct avoids direct
// API dependencies. The concrete RenderingSystem implementation uses this index to directly
// access its internal native handle struct with $O(1)$ lookup time.

struct GpuStaticMesh
{
    uint32_t NativeHandlePoolIndex = 0;

    StaticMeshHandle hOrigin = StaticMeshHandle::sNULL;
    uint64_t LastUsedFrame = 0;
    GpuStaticMeshHandle hThis;
    bool bPersistent = false;
    bool bPendingUpload = true;
};

struct GpuMaterial
{
    struct GpuPipelineState
    {
        eMaterialAlphaMode AlphaMode = eMaterialAlphaMode::None;
        eMaterialAlphaBlendMode AlphaBlendMode = eMaterialAlphaBlendMode::None;
        bool bWireframe = false;
        bool bBackfaceCulling = true;
    };

    struct AttributeLayout
    {
        Vector4 Ambient{0.f, 0.f, 0.f, 1.f};
        Vector4 Diffuse{1.f, 1.f, 1.f, 1.f};
        Vector4 Specular{1.f, 1.f, 1.f, 1.f};
        Vector4 Albedo{1.f, 1.f, 1.f, 1.f};
        Vector4 Emissive{0.f, 0.f, 0.f, 1.f};

        float Shininess = 32.0f;
        float Metallic = 0.0f;
        float Roughness = 0.5f;
        float IndexOfRefraction = 1.45f;

        float Opacity = 1.0f;
        float AlphaThreshold = 0.5f;
        float EmissiveIntensity = 1.0f;
        float NormalScale = 1.0f;
        float OcclusionStrength = 1.0f;

        struct alignas(16) GpuInt32
        {
            int32_t Value = 0;
            int32_t Padding[3] = {};
        };

        alignas(16) GpuInt32 UVChannels[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};

        alignas(16) Matrix4x4 UVMatrices[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};

        alignas(16) GpuInt32 bHasTexture[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    };

    GpuPipelineState PipelineState;
    eMaterialAssetType Type = eMaterialAssetType::None;

    uint32_t NativeHandlePoolIndex = 0;
    uint32_t TextureNativeHandlePoolIndices[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    uint32_t ShaderNativeHandlePoolIndices[static_cast<int32_t>(eShaderStage::Last)] = {};

    GpuTextureHandle hGpuTextures[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    GpuShaderHandle hGpuShaders[static_cast<int32_t>(eShaderStage::Last)] = {};

    MaterialHandle hOrigin = MaterialHandle::sNULL;
    uint64_t LastUsedFrame = 0;
    GpuMaterialHandle hThis = GpuMaterialHandle::sNULL;
    bool bPersistent = false;
    bool bPendingUpload = true;
};

struct GpuTexture
{
    uint32_t NativeHandlePoolIndex = 0;

    TextureHandle hOrigin;
    uint64_t LastUsedFrame = 0;
    GpuTextureHandle hThis;
    bool bPersistent = false;
    bool bPendingUpload = true;
};

struct GpuShader
{
    uint32_t NativeHandlePoolIndex = 0;

    ShaderHandle hOrigin;
    uint64_t LastUsedFrame = 0;
    GpuShaderHandle hThis;
    bool bPersistent = false;
    bool bPendingUpload = true;
};
} // namespace ho