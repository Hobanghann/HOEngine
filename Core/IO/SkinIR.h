#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Math/Transform3D.h"

namespace ho
{
struct SkinIR
{
    SkinIR(std::string&& nameStr, std::vector<Transform3D>&& offsetTransforms) noexcept
      : NameStr(std::move(nameStr))
      , OffsetTransforms(std::move(offsetTransforms))
    {
    }

    SkinIR(SkinIR&& rhs) noexcept = default;
    SkinIR& operator=(SkinIR&& rhs) noexcept = default;

    SkinIR(const SkinIR&) = delete;
    SkinIR& operator=(const SkinIR&) = delete;

    FORCE_INLINE const Transform3D& GetOffsetTransform(int32_t index) const
    {
        HO_ASSERT(index < static_cast<int32_t>(OffsetTransforms.size()), "out of bound.");
        return OffsetTransforms[index];
    }

    std::string NameStr;
    std::vector<Transform3D> OffsetTransforms;
};
} // namespace ho
