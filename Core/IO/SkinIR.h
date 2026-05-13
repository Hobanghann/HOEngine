#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Math/Transform3D.h"

namespace ho
{
namespace parser
{
struct SkinIR
{
    SkinIR(std::string&& nameStr,
           std::vector<Transform3D>&& offsetTransforms,
           std::vector<std::vector<int32_t>>&& boneToSubMeshMap) noexcept
      : NameStr(std::move(nameStr))
      , OffsetTransforms(std::move(offsetTransforms))
      , BindSubMeshIndices(boneToSubMeshMap)
    {
    }

    SkinIR(const SkinIR&) = delete;
    SkinIR& operator=(const SkinIR&) = delete;

    SkinIR(SkinIR&& rhs) noexcept = default;
    SkinIR& operator=(SkinIR&& rhs) noexcept = default;

    std::string NameStr;
    std::vector<Transform3D> OffsetTransforms;
    std::vector<std::vector<int32_t>> BindSubMeshIndices;
};
} // namespace parser
} // namespace ho
