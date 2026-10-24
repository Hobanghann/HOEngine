#pragma once

#include <cstdint>

#include "Core/Memory/TypedObjectPool.h"
#include "Core/Memory/WeakTypedHandle.h"
#include "Core/Templates/FixedArray.h"
#include "FrameBufferDesc.h"
#include "Subsystems/Asset/AssetDefs.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
struct RStaticMeshAsset;
struct RMaterialAsset;
struct RShaderAsset;
struct RTextureAsset;
struct RMorphTargetsAsset;

struct GpuStaticMesh;
using GpuStaticMeshHandle = WeakTypedHandle<GpuStaticMesh, TypedObjectPool>;
struct GpuMaterial;
using GpuMaterialHandle = WeakTypedHandle<GpuMaterial, TypedObjectPool>;
struct GpuTexture;
using GpuTextureHandle = WeakTypedHandle<GpuTexture, TypedObjectPool>;
struct GpuShader;
using GpuShaderHandle = WeakTypedHandle<GpuShader, TypedObjectPool>;
struct GpuMorphTargets;
using GpuMorphTargetsHandle = WeakTypedHandle<GpuMorphTargets, TypedObjectPool>;
struct GpuBuffer;
using GpuBufferHandle = WeakTypedHandle<GpuBuffer, TypedObjectPool>;
struct FrameBuffer;
using FrameBufferHandle = WeakTypedHandle<FrameBuffer, TypedObjectPool>;

// 'uint32_t NativeHandlePoolIndex;' used to retrieve API-specific native handles from the rendering system's
// NativeHandle object pool. (index 0 means NULL.)
//
// Since different graphics APIs (GL, DX, Vulkan) require different types and quantities
// of native resources (e.g., VAO/VBO vs PipelineStateObject), this struct avoids direct
// API dependencies.

// TODO: State member is occurs race condition.
// It needs to be changed as atomic. And profiling non-atomic, atomic.
enum class eGpuResourceState
{
    PendingUpload,
    UpToDate,
    Obsolete
};

struct GpuStaticMesh
{
    uint64_t NativeHandlePoolID = 0;

    ObjectHandle<RStaticMeshAsset> hOrigin = ObjectHandle<RStaticMeshAsset>::sNULL;
    uint64_t LastUsedFrame = 0;
    GpuStaticMeshHandle hThis = GpuStaticMeshHandle::sNULL;
    bool bPersistent = false;
    eGpuResourceState State = eGpuResourceState::PendingUpload;
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

    GpuPipelineState PipelineState;
    eMaterialAssetType Type = eMaterialAssetType::None;

    GpuBufferHandle hAttributeBuffer;

    GpuTextureHandle hGpuTextures[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    bool bHasTexture[static_cast<int32_t>(eMaterialTextureUsage::Last)] = {};
    GpuShaderHandle hGpuShaders[static_cast<int32_t>(eShaderStage::Last)] = {};

    ObjectHandle<RMaterialAsset> hOrigin = ObjectHandle<RMaterialAsset>::sNULL;
    uint64_t LastUsedFrame = 0;
    GpuMaterialHandle hThis = GpuMaterialHandle::sNULL;
    bool bPersistent = false;
    eGpuResourceState State = eGpuResourceState::PendingUpload;
};

struct GpuTexture
{
    uint64_t NativeHandlePoolID = 0;

    int32_t SampleCount = 1;

    int32_t Width = 0;
    int32_t Height = 0;
    int32_t Depth = 0;
    int32_t ArraySize = 0;
    int32_t MipLevels = 0;

    eTextureFormat Format = eTextureFormat::None;
    eTextureType Type = eTextureType::None;

    ObjectHandle<RTextureAsset> hOrigin = ObjectHandle<RTextureAsset>::sNULL;
    uint64_t LastUsedFrame = 0;
    int32_t FrameBufferAttachmentCount = 0;
    GpuTextureHandle hThis = GpuTextureHandle::sNULL;
    bool bPersistent = false;
    eGpuResourceState State = eGpuResourceState::PendingUpload;
};

struct GpuShader
{
    uint64_t NativeHandlePoolID = 0;

    ObjectHandle<RShaderAsset> hOrigin = ObjectHandle<RShaderAsset>::sNULL;
    uint64_t LastUsedFrame = 0;
    GpuShaderHandle hThis = GpuShaderHandle::sNULL;
    bool bPersistent = false;
    eGpuResourceState State = eGpuResourceState::PendingUpload;
};

struct GpuMorphTargets
{
    uint64_t NativeHandlePoolID = 0;

    // All morph target attributes are uploaded into a single monolithic buffer:
    // | All Positions | All Normals | All Tangents |
    // The following members are byte offsets used to locate the start of each attribute array within the buffer.
    int32_t AllPositionByteOffset = 0;
    int32_t AllPositionByteSize = 0;
    int32_t AllNormalByteOffset = 0;
    int32_t AllNormalByteSize = 0;
    int32_t AllTangentByteOffset = 0;
    int32_t AllTangentByteSize = 0;

    // Used by the rendering system to translate a client-provided SubMesh index into the correct morph target data.
    // This array is indexed by SubMeshIndex, and each element stores the starting offset (index) of that SubMesh's
    // first target within the flattened global morph target array. If a SubMesh has no morph targets, the value is -1.
    FixedArray<int32_t> SubMeshBaseTargetOffset;

    ObjectHandle<RMorphTargetsAsset> hOrigin = ObjectHandle<RMorphTargetsAsset>::sNULL;
    uint64_t LastUsedFrame = 0;
    GpuMorphTargetsHandle hThis = GpuMorphTargetsHandle::sNULL;
    bool bPersistent = false;
    eGpuResourceState State = eGpuResourceState::PendingUpload;
};

enum class eGpuBufferUsage
{
    Uniform, // OpenGL UBO / D3D12 ConstantBuffer
    Storage, // OpenGL SSBO / D3D12 StructuredBuffer(UAV)
};

enum class eGpuBufferAccess : uint8_t
{
    Static,  // CPU writes once upon creation
    Dynamic, // CPU writes/updates frequently
    ReadBack // GPU writes, CPU reads back frequently
};

// The engine defers physical VRAM allocation for GPU buffers until data is first uploaded.
// Therefore, the 'UpToDate' state signifies that the buffer metadata has been created
// and registered with the rendering system, while 'bAllocated' indicates whether
// physical VRAM has actually been allocated.
struct GpuBuffer
{
    uint64_t NativeHandlePoolID = 0;

    int32_t ByteSize = 0;
    eGpuBufferUsage Usage = eGpuBufferUsage::Uniform;
    eGpuBufferAccess Access = eGpuBufferAccess::Dynamic;

    uint64_t LastUsedFrame = 0;
    GpuBufferHandle hThis = GpuBufferHandle::sNULL;
    bool bAllocated = false;
    bool bPersistent = false;
    eGpuResourceState State = eGpuResourceState::PendingUpload;
};

struct FrameBuffer
{
    struct TargetBindingState
    {
        GpuTextureHandle hTarget;
        int32_t BaseSlice = 0;
        int32_t SliceCount = 0;
        int32_t MipLevel = 0;
    };

    StringHandle hName = StringHandle::sNULL;
    int32_t Width = 0;
    int32_t Height = 0;
    TargetBindingState ColorTargetStates[FrameBufferDesc::sMaxColorTargetCount] = {};
    TargetBindingState DepthTargetState;
    FrameBufferHandle hThis = FrameBufferHandle::sNULL;
};

} // namespace ho