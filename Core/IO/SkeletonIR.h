#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Math/Transform3D.h"

namespace ho
{
struct SkeletonIR
{
    SkeletonIR(std::string&& nameStr,
               std::vector<std::string>&& boneNameStrs,
               std::vector<Transform3D>&& localTransforms,
               std::vector<int32_t>&& parents) noexcept
      : NameStr(std::move(nameStr))
      , BoneNameStrs(std::move(boneNameStrs))
      , LocalTransforms(std::move(localTransforms))
      , Parents(std::move(parents))
    {
        HO_ASSERT((BoneNameStrs.size() == LocalTransforms.size() && BoneNameStrs.size() == Parents.size()),
                  "Invalid SkeletonIR");

        const int32_t count = BoneNameStrs.size();

        Parents.reserve(BoneNameStrs.size());
        Children.resize(count);
        for (int32_t i = 0; i < static_cast<int32_t>(Parents.size()); ++i)
        {
            int32_t parent = Parents[i];
            if (parent >= 0)
            {
                Children[parent].push_back(i);
            }
        }

        for (int32_t i = 0; i < static_cast<int32_t>(BoneNameStrs.size()); i++)
        {
            BoneNameToIndex[BoneNameStrs[i]] = i;
        }
    }

    SkeletonIR(SkeletonIR&& rhs) noexcept = default;
    SkeletonIR& operator=(SkeletonIR&& rhs) noexcept = default;

    SkeletonIR(const SkeletonIR&) = delete;
    SkeletonIR& operator=(const SkeletonIR&) = delete;

    FORCE_INLINE int32_t GetBoneCount() const
    {
        return static_cast<int32_t>(BoneNameStrs.size());
    }

    FORCE_INLINE int32_t GetBoneIndex(const std::string& nameStr) const
    {
        auto it = BoneNameToIndex.find(nameStr);
        HO_ASSERT(it != BoneNameToIndex.end(),
                  (std::string("There is no bone ") + nameStr + "in skeleton " + NameStr).c_str());
        return it->second;
    }

    FORCE_INLINE const std::string& GetBoneName(int32_t boneIndex) const
    {
        HO_ASSERT(boneIndex < static_cast<int32_t>(BoneNameStrs.size()), "out of bound.");
        return BoneNameStrs[boneIndex];
    }

    FORCE_INLINE const Transform3D& GetLocalTransform(const std::string& nameStr) const
    {
        int32_t index = GetBoneIndex(nameStr);
        return GetLocalTransform(index);
    }

    FORCE_INLINE const Transform3D& GetLocalTransform(int32_t boneIndex) const
    {
        HO_ASSERT(boneIndex < static_cast<int32_t>(LocalTransforms.size()), "out of bound.");
        return LocalTransforms[boneIndex];
    }

    FORCE_INLINE int32_t GetParentIndex(const std::string& nameStr) const
    {
        return GetParentIndex(GetBoneIndex(nameStr));
    }

    FORCE_INLINE int32_t GetParentIndex(int32_t boneIndex) const
    {
        HO_ASSERT(boneIndex < static_cast<int32_t>(Parents.size()), "out of bound.");
        return Parents[boneIndex];
    }

    FORCE_INLINE int32_t GetChildCount(const std::string& nameStr) const
    {
        return GetChildCount(GetBoneIndex(nameStr));
    }

    FORCE_INLINE int32_t GetChildCount(int32_t index) const
    {
        HO_ASSERT(index < static_cast<int32_t>(Children.size()), "out of bound.");
        return static_cast<int32_t>(Children[index].size());
    }

    FORCE_INLINE int32_t GetChildIndex(const std::string& parentNameStr, int32_t childIndex) const
    {
        return GetChildIndex(GetBoneIndex(parentNameStr), childIndex);
    }

    FORCE_INLINE int32_t GetChildIndex(int32_t parentIndex, int32_t childIndex) const
    {
        HO_ASSERT(parentIndex < static_cast<int32_t>(Children.size()), "out of bound.");
        HO_ASSERT(childIndex < static_cast<int32_t>(Children[parentIndex].size()), "out of bound.");
        return Children[parentIndex][childIndex];
    }

    std::string NameStr;
    std::vector<std::string> BoneNameStrs;
    std::vector<Transform3D> LocalTransforms;
    std::vector<int32_t> Parents;
    std::vector<std::vector<int32_t>> Children;
    std::unordered_map<std::string, int32_t> BoneNameToIndex;
};
} // namespace ho