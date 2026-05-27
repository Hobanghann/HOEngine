#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform3D.h"
#include "Core/Math/Vector3.h"
#include "Macros.h"
#include "Path.h"

namespace ho
{
namespace parser
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

    struct MorphingKey
    {
        MorphingKey(float time, std::vector<int32_t>&& morphTargetIndices, std::vector<float>&& weights) noexcept
          : Time(time)
          , MorphTargetIndices(std::move(morphTargetIndices))
          , Weights(std::move(weights))
        {
            HO_ASSERT(MorphTargetIndices.size() == Weights.size(), "Invalid MorphingKey format.");
        }

        MorphingKey(const MorphingKey& rhs) = delete;
        MorphingKey& operator=(const MorphingKey& rhs) = delete;

        MorphingKey(MorphingKey&& rhs) noexcept = default;
        MorphingKey& operator=(MorphingKey&& rhs) noexcept = default;

        float Time;
        std::vector<int32_t> MorphTargetIndices;
        std::vector<float> Weights;
    };

    struct SkeletalTrack
    {
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

        SkeletalTrack(const SkeletalTrack& rhs) = delete;
        SkeletalTrack& operator=(const SkeletalTrack& rhs) = delete;

        SkeletalTrack(SkeletalTrack&& rhs) noexcept = default;
        SkeletalTrack& operator=(SkeletalTrack&& rhs) noexcept = default;

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
        MorphTargetTrack(int32_t boneIndex, std::vector<MorphingKey>&& keySequence) noexcept
          : BoneIndex(boneIndex)
          , KeySequance(std::move(keySequence))
        {
        }

        MorphTargetTrack(const MorphTargetTrack& rhs) = delete;
        MorphTargetTrack& operator=(const MorphTargetTrack& rhs) = delete;

        MorphTargetTrack(MorphTargetTrack&& rhs) noexcept = default;
        MorphTargetTrack& operator=(MorphTargetTrack&& rhs) noexcept = default;

        int32_t BoneIndex;
        std::vector<MorphingKey> KeySequance;
    };

    AnimationIR(const Path& path,
                std::string&& nameStr,
                float duration,
                std::vector<SkeletalTrack>&& skeletalTracks,
                std::vector<MorphTargetTrack>&& morphTargetTracks) noexcept
      : ResourcePath(path)
      , NameStr(std::move(nameStr))
      , Duration(duration)
      , SkeletalTracks(std::move(skeletalTracks))
      , MorphTargetTracks(std::move(morphTargetTracks))
    {
        BoneIndexToSkeletalTrackIndexMap.reserve(SkeletalTracks.size());
        for (int32_t i = 0; i < static_cast<int32_t>(SkeletalTracks.size()); ++i)
        {
            BoneIndexToSkeletalTrackIndexMap[SkeletalTracks[i].BoneIndex] = i;
        }
        BoneIndexToMorphTargetTrackIndexMap.reserve(MorphTargetTracks.size());
        for (int32_t i = 0; i < static_cast<int32_t>(MorphTargetTracks.size()); ++i)
        {
            BoneIndexToMorphTargetTrackIndexMap[MorphTargetTracks[i].BoneIndex] = i;
        }
    }

    AnimationIR(const AnimationIR&) = delete;
    AnimationIR& operator=(const AnimationIR&) = delete;

    AnimationIR(AnimationIR&& rhs) noexcept = default;
    AnimationIR& operator=(AnimationIR&& rhs) noexcept = default;

    const SkeletalTrack& GetSkeletalTrackByBoneIndex(int32_t boneIndex) const
    {
        auto it = BoneIndexToSkeletalTrackIndexMap.find(boneIndex);
        HO_ASSERT(it != BoneIndexToSkeletalTrackIndexMap.end(), "Invalid bone index.");
        HO_ASSERT(it->second >= 0 && it->second < static_cast<int32_t>(SkeletalTracks.size()), "Invalid bone index.");
        return SkeletalTracks[it->second];
    }

    const MorphTargetTrack& GetMorphTargetTrackByBoneIndex(int32_t boneIndex) const
    {
        auto it = BoneIndexToMorphTargetTrackIndexMap.find(boneIndex);
        HO_ASSERT(it != BoneIndexToMorphTargetTrackIndexMap.end(), "Invalid bone index.");
        HO_ASSERT(it->second >= 0 && it->second < static_cast<int32_t>(MorphTargetTracks.size()),
                  "Invalid bone index.");
        return MorphTargetTracks[it->second];
    }

    Path ResourcePath;
    std::string NameStr;
    float Duration;
    std::vector<SkeletalTrack> SkeletalTracks;
    std::vector<MorphTargetTrack> MorphTargetTracks;
    std::unordered_map<int32_t, int32_t> BoneIndexToSkeletalTrackIndexMap;
    std::unordered_map<int32_t, int32_t> BoneIndexToMorphTargetTrackIndexMap;
};
} // namespace parser
} // namespace ho
