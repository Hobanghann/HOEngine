#pragma once

#include "AssetDefs.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"
#include "Core/Templates/FixedArray.h"
#include "IAsset.h"

namespace ho
{
struct GpuMorphTargets;
using GpuMorphTargetsHandle = WeakTypedHandle<GpuMorphTargets, TypedObjectPool>;

struct RMorphTargetsAsset final : public RIAsset
{
    HO_INJECT_REFLECTION_CODE(RMorphTargetsAsset)

    struct MorphTarget
    {
        StringHandle hName = StringHandle::sNULL;

        int32_t PositionOffset = 0;
        int32_t NormalOffset = 0;  // -1 means morph target doesn't have normal.
        int32_t TangentOffset = 0; // -1 means morph target doesn't have tangent.

        float Weight = 0.0f;
    };

    struct MorphTargets
    {
        int32_t SubMeshIndex = 0;
        // Assumes the vertex count is always the same between the submesh and the morph target.
        int32_t VertexCount = 0;

        FixedArray<MorphTarget> Targets;
    };

    RMorphTargetsAsset() = default;
    RMorphTargetsAsset(const RMorphTargetsAsset&) = delete;
    RMorphTargetsAsset& operator=(const RMorphTargetsAsset&) = delete;
    ~RMorphTargetsAsset() override = default;

    RMorphTargetsAsset(RMorphTargetsAsset&&) noexcept = default;
    RMorphTargetsAsset& operator=(RMorphTargetsAsset&&) noexcept = default;

    int32_t GetByteSize() const override;

    FixedArray<MorphTargets> SubMeshMorphTargets;

    FixedArray<Vector3> Positions;
    FixedArray<Vector3> Normals;
    FixedArray<Vector4> Tangents;

    GpuMorphTargetsHandle hRenderProxy;
};

} // namespace ho