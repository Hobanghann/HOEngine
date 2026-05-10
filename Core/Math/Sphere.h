#pragma once

#include <string>

#include "MathFuncs.h"
#include "Vector3.h"

namespace ho
{
struct Sphere final
{
    [[nodiscard]] static Sphere FromPositionsCentroid(const Vector3* pPositions, int32_t count);
    [[nodiscard]] static Sphere FromPositionsRitter(const Vector3* pPositions, int32_t count);

    [[nodiscard]] static Sphere Merge(const Sphere& lhs, const Sphere& rhs);

    constexpr Sphere();
    constexpr Sphere(const Sphere& rhs) = default;
    constexpr Sphere(const Vector3& center, float radius);
    constexpr Sphere& operator=(const Sphere& rhs);
    ~Sphere() = default;

    [[nodiscard]] constexpr bool operator==(const Sphere& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Sphere& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Sphere& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Sphere& rhs) const;

    [[nodiscard]] std::string ToString() const;

    Vector3 Center;
    float Radius;
};

constexpr Sphere::Sphere()
  : Center(Vector3::sZero)
  , Radius(0.0f)
{
}

constexpr Sphere::Sphere(const Vector3& center, float radius)
  : Center(center)
  , Radius(radius)
{
}

constexpr Sphere& Sphere::operator=(const Sphere& rhs) = default;

constexpr bool Sphere::operator==(const Sphere& rhs) const
{
    return Center == rhs.Center && Radius == rhs.Radius;
}

constexpr bool Sphere::operator!=(const Sphere& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Sphere::IsEqualApprox(const Sphere& rhs) const
{
    return Center.IsEqualApprox(rhs.Center) && math::IsEqualApprox(Radius, rhs.Radius);
}

constexpr bool Sphere::IsNotEqualApprox(const Sphere& rhs) const
{
    return !IsEqualApprox(rhs);
}
} // namespace ho
