#pragma once

#include "AssetDefs.h"
#include "Core/Templates/FixedArray.h"
#include "IAsset.h"

namespace ho
{

struct RMorphAnimationAsset final : public RIAsset
{
    HO_INJECT_REFLECTION_CODE(RMorphAnimationAsset)

    RMorphAnimationAsset() = default;
    RMorphAnimationAsset(const RMorphAnimationAsset&) = delete;
    RMorphAnimationAsset& operator=(const RMorphAnimationAsset&) = delete;
    ~RMorphAnimationAsset() override = default;

    RMorphAnimationAsset(RMorphAnimationAsset&&) noexcept = default;
    RMorphAnimationAsset& operator=(RMorphAnimationAsset&&) noexcept = default;

    int32_t GetByteSize() const override;

    struct MorphKey
    {
        float Time = 0.0f;
        // Flattened representation of (TargetIndex, Weight) pairs using SoA.
        FixedArray<int32_t> MorphTargetIndices;
        FixedArray<float> Weights;
    };

    struct MorphTrack
    {
        FixedArray<int32_t> SubmeshIndices;
        FixedArray<MorphKey> KeySequence;
    };

    float Duration = 0.0f;
    FixedArray<MorphTrack> Tracks;
};
} // namespace ho