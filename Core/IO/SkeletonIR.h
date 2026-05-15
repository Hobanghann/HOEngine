#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Math/Transform3D.h"
#include "Path.h"

namespace ho
{
namespace parser
{
struct SkeletonIR
{
    SkeletonIR(const Path& path,
               std::string&& nameStr,
               std::vector<std::string>&& boneNameStrs,
               std::vector<Transform3D>&& localTransforms,
               std::vector<int32_t>&& parents) noexcept
      : ResourcePath(path)
      , NameStr(std::move(nameStr))
      , BoneNameStrs(std::move(boneNameStrs))
      , LocalTransforms(std::move(localTransforms))
      , Parents(std::move(parents))
    {
        HO_ASSERT((BoneNameStrs.size() == LocalTransforms.size() && BoneNameStrs.size() == Parents.size()),
                  "Invalid SkeletonIR");

        const int32_t count = static_cast<int32_t>(BoneNameStrs.size());

        Parents.reserve(BoneNameStrs.size());
        Children.resize(count);
        for (int32_t i = 0; i < static_cast<int32_t>(Parents.size()); ++i)
        {
            const int32_t parent = Parents[i];
            if (parent >= 0)
            {
                Children[parent].push_back(i);
            }
        }

        for (int32_t i = 0; i < static_cast<int32_t>(BoneNameStrs.size()); i++)
        {
            BoneNameToIndexMap[BoneNameStrs[i]] = i;
        }
    }

    SkeletonIR(const SkeletonIR&) = delete;
    SkeletonIR& operator=(const SkeletonIR&) = delete;

    SkeletonIR(SkeletonIR&& rhs) noexcept = default;
    SkeletonIR& operator=(SkeletonIR&& rhs) noexcept = default;

    const Transform3D& GetLocalTransformByBoneName(const std::string& nameStr) const
    {
        auto it = BoneNameToIndexMap.find(nameStr);
        HO_ASSERT(it != BoneNameToIndexMap.end(), "Invalid bone name.");
        HO_ASSERT(it->second >= 0 && it->second < static_cast<int32_t>(LocalTransforms.size()), "Invalid bone name.");
        return LocalTransforms[it->second];
    }

    int32_t GetParentIndexByBoneName(const std::string& nameStr) const
    {
        auto it = BoneNameToIndexMap.find(nameStr);
        HO_ASSERT(it != BoneNameToIndexMap.end(), "Invalid bone name.");
        HO_ASSERT(it->second >= 0 && it->second < static_cast<int32_t>(Parents.size()), "Invalid bone name.");
        return Parents[it->second];
    }

    const std::vector<int32_t>& GetChildIndicesByBoneName(const std::string& nameStr) const
    {
        auto it = BoneNameToIndexMap.find(nameStr);
        HO_ASSERT(it != BoneNameToIndexMap.end(), "Invalid bone name.");
        HO_ASSERT(it->second >= 0 && it->second < static_cast<int32_t>(Children.size()), "Invalid bone name.");
        return Children[it->second];
    }

    Path ResourcePath;
    std::string NameStr;
    std::vector<std::string> BoneNameStrs;
    std::vector<Transform3D> LocalTransforms;
    std::vector<int32_t> Parents;
    std::vector<std::vector<int32_t>> Children;
    std::unordered_map<std::string, int32_t> BoneNameToIndexMap;
};
} // namespace parser
} // namespace ho