#pragma once

#include <array>

#include "AssetDefs.h"
#include "Core/Math/AABB.h"
#include "Core/Math/Color32.h"
#include "Core/Math/Sphere.h"
#include "Core/Math/Transform3D.h"
#include "Core/Templates/FixedArray.h"
#include "IAsset.h"
#include "MaterialAsset.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
struct GpuStaticMesh;
using GpuStaticMeshHandle = WeakTypedHandle<GpuStaticMesh, TypedObjectPool>;

struct RMaterialAsset;

struct RStaticMeshAsset final : public RIAsset
{
    HO_INJECT_REFLECTION_CODE(RStaticMeshAsset)

    static const uint32_t sMaxUVChannel = 2;
    static const uint32_t sMaxColorChannel = 1;

    struct SubMesh
    {
        StringHandle hName = StringHandle::sNULL;

        eMeshPrimitiveType PrimitiveType = eMeshPrimitiveType::None;

        int32_t PositionOffset = 0;
        int32_t NormalOffset = 0;  // -1 means it dosen't have normal
        int32_t TangentOffset = 0; // -1 means it dosen't have tangent
        int32_t UVOffset = 0;      // -1 means it dosen't have UV
        int32_t ColorOffset = 0;   // -1 means it dosen't have color
        int32_t VertexCount = 0;

        int32_t IndexOffset = 0;
        int32_t IndexCount = 0;

        int32_t LocalTransformOffset = 0;
        int32_t LocalTransformCount = 0;

        StringHandle hRenderMaterialName = StringHandle::sNULL;
        ObjectHandle<RMaterialAsset> hRenderMaterial = ObjectHandle<RMaterialAsset>::GetNULL();

        AABB AabbVolume;
        Sphere SphereVolume;
    };

    RStaticMeshAsset() = default;
    RStaticMeshAsset(const RStaticMeshAsset&) = delete;
    RStaticMeshAsset& operator=(const RStaticMeshAsset&) = delete;
    ~RStaticMeshAsset() override = default;

    RStaticMeshAsset(RStaticMeshAsset&&) noexcept = default;
    RStaticMeshAsset& operator=(RStaticMeshAsset&&) noexcept = default;

    int32_t GetByteSize() const override;

    FixedArray<Vector3> Positions;
    FixedArray<Vector3> Normals;
    FixedArray<Vector4> Tangents;
    FixedArray<std::array<Vector2, sMaxUVChannel>> UVs;
    FixedArray<std::array<Color32, sMaxColorChannel>> Colors;

    FixedArray<uint32_t> Indices;

    FixedArray<Transform3D> LocalTransforms;

    FixedArray<SubMesh> SubMeshes;

    AABB AabbVolume;
    Sphere SphereVolume;

    GpuStaticMeshHandle hRenderProxy;
};

} // namespace ho