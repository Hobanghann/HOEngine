#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Macros.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform3D.h"
#include "Core/Math/Vector3.h"

namespace ho
{

struct AnimationIR
{
    enum class eExtrapolationMode
    {
        // Outside the animation time range, fall back to the node's
        // original (bind/default) local transform.
        Default = 0,

        // Outside the animation time range, keep using the closest
        // animation key value without any interpolation.
        Constant = 1,

        // Outside the animation time range, estimate the value by
        // extending the curve linearly using the two nearest keys.
        Linear = 2,

        // Loop the animation time back into the valid key range and
        // evaluate the animation as if it were repeating endlessly.
        Repeat = 3
    };

    enum class eInterpolationMode
    {
        // The value stays constant until the next keyframe is reached.
        // No interpolation is performed between keys.
        Step = 0,

        // Interpolate linearly between two neighboring key values.
        // Used for translations, scales, and any linear quantity.
        Linear = 1,

        // Interpolate rotations using spherical linear interpolation (slerp).
        // Ensures constant angular velocity and avoids artifacts of linear
        // interpolation.
        SphericalLinear = 2,

        // Interpolate using cubic splines defined by in/out tangents.
        // Produces smooth C1-continuous curves between keyframes.
        CubicSpline = 3
    };

    HO_DISABLE_COMPILER_WARNING_PUSH();
    HO_DISABLE_COMPILER_WARNING_PADDED_DUE_TO_ALIGNMENT_SPECIFIER();

    struct TranslationKey
    {
        float Time;
        Vector3 TranslationVector;
    };

    struct RotationKey
    {
        float Time;
        Quaternion Rotation;
    };

    struct ScalingKey
    {
        float Time;
        Vector3 Scale;
    };

    HO_DISABLE_COMPILER_WARNING_POP();

    struct MorphingKey
    {
        MorphingKey() = default;

        MorphingKey(float time, std::vector<int32_t>&& morphTargetIndices, std::vector<float>&& weights) noexcept
          : Time(time)
          , MorphTargetIndices(std::move(morphTargetIndices))
          , Weights(std::move(weights))
        {
            HO_ASSERT(MorphTargetIndices.size() == Weights.size(), "Invalid MorphingKey format.");
        }

        MorphingKey(MorphingKey&& rhs) noexcept = default;
        MorphingKey& operator=(MorphingKey&& rhs) noexcept = default;

        MorphingKey(const MorphingKey& rhs) = delete;
        MorphingKey& operator=(const MorphingKey& rhs) = delete;

        float Time;
        std::vector<int32_t> MorphTargetIndices;
        std::vector<float> Weights;
    };

    struct SkeletalTrack
    {
        SkeletalTrack() = default;

        SkeletalTrack(int32_t boneIndex,
                      std::vector<TranslationKey>&& translationKeySequence,
                      eInterpolationMode translationInterpMode,
                      std::vector<RotationKey>&& rotationKeySequence,
                      eInterpolationMode rotationInterpMode,
                      std::vector<ScalingKey>&& scalingKeySequence,
                      eInterpolationMode scalingInterpMode,
                      eExtrapolationMode preExtrapMode,
                      eExtrapolationMode postExtrapMode) noexcept
          : BoneIndex(boneIndex)
          , TranslationKeySequence(std::move(translationKeySequence))
          , TranslationInterpMode(translationInterpMode)
          , RotationKeySequence(std::move(rotationKeySequence))
          , RotationInterpMode(rotationInterpMode)
          , ScalingKeySequence(std::move(scalingKeySequence))
          , ScalingInterpMode(scalingInterpMode)
          , PreExtrapMode(preExtrapMode)
          , PostExtrapMode(postExtrapMode)
        {
        }

        SkeletalTrack(SkeletalTrack&& rhs) noexcept = default;
        SkeletalTrack& operator=(SkeletalTrack&& rhs) noexcept = default;

        SkeletalTrack(const SkeletalTrack& rhs) = delete;
        SkeletalTrack& operator=(const SkeletalTrack& rhs) = delete;

        int32_t BoneIndex;
        std::vector<TranslationKey> TranslationKeySequence;
        eInterpolationMode TranslationInterpMode;
        std::vector<RotationKey> RotationKeySequence;
        eInterpolationMode RotationInterpMode;
        std::vector<ScalingKey> ScalingKeySequence;
        eInterpolationMode ScalingInterpMode;
        eExtrapolationMode PreExtrapMode;
        eExtrapolationMode PostExtrapMode;
    };

    struct MorphTargetTrack
    {
        MorphTargetTrack() = default;

        MorphTargetTrack(int32_t boneIndex, std::vector<MorphingKey>&& keySequence) noexcept
          : BoneIndex(boneIndex)
          , KeySequance(std::move(keySequence))
        {
        }

        MorphTargetTrack(MorphTargetTrack&& rhs) noexcept = default;
        MorphTargetTrack& operator=(MorphTargetTrack&& rhs) noexcept = default;

        MorphTargetTrack(const MorphTargetTrack& rhs) = delete;
        MorphTargetTrack& operator=(const MorphTargetTrack& rhs) = delete;

        int32_t BoneIndex;
        std::vector<MorphingKey> KeySequance;
    };

    AnimationIR() = default;

    AnimationIR(std::string&& nameStr,
                float duration,
                std::vector<SkeletalTrack>&& skeletalTracks,
                std::vector<MorphTargetTrack>&& morphTargetTracks) noexcept
      : NameStr(std::move(nameStr))
      , Duration(duration)
      , SkeletalTracks(std::move(skeletalTracks))
      , MorphTargetTracks(std::move(morphTargetTracks))
    {
        BoneIndexToSkeletalTrack.reserve(SkeletalTracks.size());
        for (int32_t i = 0; i < static_cast<int32_t>(SkeletalTracks.size()); ++i)
        {
            BoneIndexToSkeletalTrack[SkeletalTracks[i].BoneIndex] = i;
        }
        BoneIndexToMorphTargetTrack.reserve(MorphTargetTracks.size());
        for (int32_t i = 0; i < static_cast<int32_t>(MorphTargetTracks.size()); ++i)
        {
            BoneIndexToMorphTargetTrack[MorphTargetTracks[i].BoneIndex] = i;
        }
    }

    AnimationIR(AnimationIR&& rhs) noexcept = default;
    AnimationIR& operator=(AnimationIR&& rhs) noexcept = default;

    AnimationIR(const AnimationIR&) = delete;
    AnimationIR& operator=(const AnimationIR&) = delete;

    FORCE_INLINE int32_t GetSkeletalTrackCount() const
    {
        return static_cast<int32_t>(SkeletalTracks.size());
    }

    FORCE_INLINE bool HasSkeletalTrack(int32_t boneIndex) const
    {
        return BoneIndexToSkeletalTrack.find(boneIndex) != BoneIndexToSkeletalTrack.end();
    }

    FORCE_INLINE const SkeletalTrack& GetSkeletalTrack(int32_t boneIndex) const
    {
        auto it = BoneIndexToSkeletalTrack.find(boneIndex);
        HO_ASSERT(it != BoneIndexToSkeletalTrack.end(),
                  (std::string("There is no SkeletalTrack bound to bone index") + std::to_string(boneIndex)).c_str());
        return SkeletalTracks[it->second];
    }

    FORCE_INLINE int32_t GetMorphTargetTrackCount() const
    {
        return static_cast<int32_t>(MorphTargetTracks.size());
    }

    FORCE_INLINE bool HasMorphTargetTrack(int32_t boneIndex) const
    {
        return BoneIndexToMorphTargetTrack.find(boneIndex) != BoneIndexToMorphTargetTrack.end();
    }

    FORCE_INLINE const MorphTargetTrack& GetMorphTargetTrack(int32_t boneIndex) const
    {
        auto it = BoneIndexToMorphTargetTrack.find(boneIndex);
        HO_ASSERT(
            it != BoneIndexToMorphTargetTrack.end(),
            (std::string("There is no MorphTargetTrack bound to bone index") + std::to_string(boneIndex)).c_str());
        return MorphTargetTracks[it->second];
    }

    std::string NameStr;
    float Duration;
    std::vector<SkeletalTrack> SkeletalTracks;
    std::vector<MorphTargetTrack> MorphTargetTracks;
    std::unordered_map<int32_t, int32_t> BoneIndexToSkeletalTrack;
    std::unordered_map<int32_t, int32_t> BoneIndexToMorphTargetTrack;
};

} // namespace ho
