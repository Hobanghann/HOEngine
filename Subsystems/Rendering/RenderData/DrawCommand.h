#pragma once

#include "Core/Memory/WeakTypedHandle.h"

namespace ho
{
struct GpuStaticMesh;
using GpuStaticMeshHandle = WeakTypedHandle<GpuStaticMesh, TypedObjectPool>;
struct GpuMaterial;
using GpuMaterialHandle = WeakTypedHandle<GpuMaterial, TypedObjectPool>;
struct GpuMorphTargets;
using GpuMorphTargetsHandle = WeakTypedHandle<GpuMorphTargets, TypedObjectPool>;

struct DrawCommandDesc
{
    GpuStaticMeshHandle hStaticMesh;
    GpuMaterialHandle hMaterial;
    int32_t VertexOffset = 0;
    int32_t IndexCount = 0;
    int32_t IndexOffset = 0;
    // -1 means no stencil writing. 0 to 7 represents the target bit index.
    int32_t StencilWriteBitIndex = -1;
};

struct DrawCommand
{
    GpuStaticMeshHandle hGpuStaticMesh;
    GpuMaterialHandle hGpuMaterial;
    GpuMorphTargetsHandle hGpuMorphTargets;
    int32_t VertexOffset = 0;
    int32_t IndexOffset = 0;
    int32_t IndexCount = 0;
    int32_t InstanceOffset = 0;
    int32_t InstanceCount = 0;
    int32_t MorphControlDataOffset = -1; // -1 means it's not morph animated.
    float SortDepth = 0.0f;
    int32_t StencilWriteBitIndex = -1;
};
} // namespace ho