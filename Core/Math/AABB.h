#pragma once

#include <string>

#include "Vector3.h"

namespace ho
{
struct alignas(16) AABB
{
    [[nodiscard]] static AABB FromPositions(const Vector3* pPositions, int32_t count);

    [[nodiscard]] constexpr static AABB Merge(const AABB& lhs, const AABB& rhs);

    constexpr AABB() = default;
    constexpr AABB(const AABB& aabb);
    constexpr AABB(const Vector3& min, const Vector3& max);
    constexpr AABB& operator=(const AABB& rhs);
    ~AABB() = default;

    [[nodiscard]] constexpr Vector3 GetCenter() const;

    [[nodiscard]] constexpr bool operator==(const AABB& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const AABB& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const AABB& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const AABB& rhs) const;

    [[nodiscard]] std::string ToString() const;

    Vector3 MinEdges;
    Vector3 MaxEdges;
};

constexpr AABB::AABB(const AABB& aabb) = default;

constexpr AABB::AABB(const Vector3& min, const Vector3& max)
  : MinEdges(min)
  , MaxEdges(max)
{
}

constexpr AABB& AABB::operator=(const AABB& rhs) = default;

constexpr Vector3 AABB::GetCenter() const
{
    return (MinEdges + MaxEdges) * 0.5f;
}

constexpr bool AABB::operator==(const AABB& rhs) const
{
    return (MinEdges == rhs.MinEdges && MaxEdges == rhs.MaxEdges);
}

constexpr bool AABB::operator!=(const AABB& rhs) const
{
    return !(*this == rhs);
}

constexpr bool AABB::IsEqualApprox(const AABB& rhs) const
{
    return (MinEdges.IsEqualApprox(rhs.MinEdges) && MaxEdges.IsEqualApprox(rhs.MaxEdges));
}

constexpr bool AABB::IsNotEqualApprox(const AABB& rhs) const
{
    return !IsEqualApprox(rhs);
}

constexpr AABB AABB::Merge(const AABB& lhs, const AABB& rhs)
{
    const float minX = math::Min(lhs.MinEdges.X, rhs.MinEdges.X);
    const float minY = math::Min(lhs.MinEdges.Y, rhs.MinEdges.Y);
    const float minZ = math::Min(lhs.MinEdges.Z, rhs.MinEdges.Z);

    const float maxX = math::Max(lhs.MaxEdges.X, rhs.MaxEdges.X);
    const float maxY = math::Max(lhs.MaxEdges.Y, rhs.MaxEdges.Y);
    const float maxZ = math::Max(lhs.MaxEdges.Z, rhs.MaxEdges.Z);

    return AABB(Vector3(minX, minY, minZ), Vector3(maxX, maxY, maxZ));
}

} // namespace ho
