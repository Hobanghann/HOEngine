#pragma once

#include <array>

#include "AssetDefs.h"
#include "Core/Math/AABB.h"
#include "Core/Math/Color32.h"
#include "Core/Math/Sphere.h"
#include "Core/Math/Transform3D.h"
#include "Core/Templates/FixedArray.h"
#include "Core/Templates/GlobalPoolIndex.h"
#include "Core/Templates/WeakLocalPoolIndex.h"
#include "MaterialAsset.h"

namespace ho
{
struct GpuStaticMesh;
using GpuStaticMeshHandle = WeakLocalPoolIndex<GpuStaticMesh, ObjectPool>;

struct StaticMeshAsset
{
    static const uint32_t sMaxUVChannel = 2;
    static const uint32_t sMaxColorChannel = 1;

    struct SubMesh
    {
        StringHandle hName = StringHandle::sNULL;

        eMeshPrimitiveType PrimitiveType = eMeshPrimitiveType::None;

        int32_t PositionOffset = 0;
        int32_t NormalOffset = 0;
        int32_t TangentOffset = 0;
        int32_t UVOffset = 0;
        int32_t ColorOffset = 0;
        int32_t VertexCount = 0;

        int32_t IndexOffset = 0;
        int32_t IndexCount = 0;

        StringHandle hRenderMaterialName = StringHandle::sNULL;
        MaterialHandle hRenderMaterial = MaterialHandle::sNULL;

        AABB Aabb;
        Sphere Sphere;

        Transform3D LocalTransform;
    };

    StaticMeshAsset() = default;
    StaticMeshAsset(const StaticMeshAsset&) = delete;
    StaticMeshAsset& operator=(const StaticMeshAsset&) = delete;
    ~StaticMeshAsset() = default;

    StaticMeshAsset(StaticMeshAsset&&) noexcept = default;
    StaticMeshAsset& operator=(StaticMeshAsset&&) noexcept = default;

    StringHandle hName = StringHandle::sNULL;

    FixedArray<Vector3> Positions;
    FixedArray<Vector3> Normals;
    FixedArray<Vector4> Tangents;
    FixedArray<std::array<Vector2, sMaxUVChannel>> UVs;
    FixedArray<std::array<Color32, sMaxColorChannel>> Colors;

    FixedArray<uint32_t> Indices;

    FixedArray<SubMesh> SubMeshes;

    AABB Aabb;
    Sphere Sphere;

    GpuStaticMeshHandle hRenderProxy = GpuStaticMeshHandle::sNULL;
};

using StaticMeshHandle = WeakLocalPoolIndex<StaticMeshAsset, ObjectPool>;
} // namespace ho