#pragma once

#include <string>

#include "Vector3.h"

// plane defined by normal vector and 1 point
// when plane defined by 4-dimensional vector, x, y, z component is normal
// vector component, w component is dot product normal vector and point
namespace ho
{
struct AABB;
struct Sphere;

struct Plane
{
    FORCE_INLINE Plane();
    FORCE_INLINE Plane(float a, float b, float c, float d);
    FORCE_INLINE Plane(const Vector3& normal, const Vector3& point);
    FORCE_INLINE Plane(const Vector3& v1,
                       const Vector3& v2,
                       const Vector3& v3,
                       math::eClockDirection = math::eClockDirection::CounterClockWise);
    FORCE_INLINE Plane(const Plane&) = default;
    FORCE_INLINE Plane& operator=(const Plane& rhs);
    ~Plane() = default;

    [[nodiscard]] FORCE_INLINE bool operator==(const Plane& rhs) const;
    [[nodiscard]] FORCE_INLINE bool operator!=(const Plane& rhs) const;

    [[nodiscard]] FORCE_INLINE bool IsEqualApprox(const Plane& rhs) const;
    [[nodiscard]] FORCE_INLINE bool IsNotEqualApprox(const Plane& rhs) const;

    [[nodiscard]] FORCE_INLINE float GetDistToPoint(const Vector3& point) const;

    [[nodiscard]] FORCE_INLINE Vector3 Project(const Vector3& point) const;

    [[nodiscard]] FORCE_INLINE math::eSide GetPointSide(const Vector3& p) const;
    [[nodiscard]] FORCE_INLINE math::eSide GetLineSide(const Vector3& p1, const Vector3& p2) const;
    [[nodiscard]] FORCE_INLINE math::eSide GetTriangleSide(const Vector3& p1,
                                                           const Vector3& p2,
                                                           const Vector3& p3) const;
    [[nodiscard]] math::eSide GetAabbSide(const AABB& aabb) const;
    [[nodiscard]] math::eSide GetSphereSide(const Sphere& sphere) const;

    [[nodiscard]] std::string ToString() const;

    Vector3 Normal;
    float D;

  private:
    template <int32_t N>
    [[nodiscard]] FORCE_INLINE math::eSide GetSide(const std::array<Vector3, N>& points) const;
};

Plane::Plane()
  : Normal(Vector3::sZero)
  , D(0.0f)
{
}

Plane::Plane(float a, float b, float c, float d)
{
    Normal = Vector3(a, b, c);
    const float mag = Normal.Magnitude();
    Normal /= mag;
    D = d / mag;
}

Plane::Plane(const Vector3& normal, const Vector3& point)
  : Normal(normal.Normalized())
  , D(-normal.Dot(point))
{
}

Plane::Plane(const Vector3& v1, const Vector3& v2, const Vector3& v3, math::eClockDirection dir)
{
    switch (dir)
    {
        case math::eClockDirection::ClockWise:
            Normal = (v3 - v1).Cross(v2 - v1).Normalized();
            break;
        case math::eClockDirection::CounterClockWise:
            Normal = (v2 - v1).Cross(v3 - v1).Normalized();
            break;
    }
    D = -Normal.Dot(v1);
}

Plane& Plane::operator=(const Plane& rhs)
{
    Normal = rhs.Normal;
    D = rhs.D;
    return *this;
}

bool Plane::operator==(const Plane& rhs) const
{
    return Normal == rhs.Normal && D == rhs.D;
}

bool Plane::operator!=(const Plane& rhs) const
{
    return !(*this == rhs);
}

bool Plane::IsEqualApprox(const Plane& rhs) const
{
    return Normal.IsEqualApprox(rhs.Normal) && math::IsEqualApprox(D, rhs.D);
}

bool Plane::IsNotEqualApprox(const Plane& rhs) const
{
    return !IsEqualApprox(rhs);
}

float Plane::GetDistToPoint(const Vector3& point) const
{
    return Normal.Dot(point) + D;
}

Vector3 Plane::Project(const Vector3& point) const
{
    const float dist = GetDistToPoint(point);
    return point - dist * Normal;
}

template <int32_t N>
math::eSide Plane::GetSide(const std::array<Vector3, N>& points) const
{
    int32_t outsideCount = 0;
    for (const auto& p : points)
    {
        if (GetSide(p) == math::eSide::Outside)
        {
            ++outsideCount;
        }
    }

    if (outsideCount == N)
    {
        return math::eSide::Outside;
    }
    if (outsideCount == 0)
    {
        return math::eSide::Inside;
    }
    return math::eSide::Intersect;
}

math::eSide Plane::GetPointSide(const Vector3& p) const
{
    const float res = Normal.Dot(p) + D;
    if (res > math::EPSILON_POINT_ON_PLANE)
    {
        return math::eSide::Outside;
    }
    else if (res < -math::EPSILON_POINT_ON_PLANE)
    {
        return math::eSide::Inside;
    }
    else
    {
        return math::eSide::On;
    }
}

math::eSide Plane::GetLineSide(const Vector3& p1, const Vector3& p2) const
{
    int32_t outsideCount = 0;
    if (GetPointSide(p1) == math::eSide::Outside)
    {
        ++outsideCount;
    }
    if (GetPointSide(p2) == math::eSide::Outside)
    {
        ++outsideCount;
    }
    if (outsideCount == 2)
    {
        return math::eSide::Outside;
    }
    if (outsideCount == 0)
    {
        return math::eSide::Inside;
    }
    return math::eSide::Intersect;
}

math::eSide Plane::GetTriangleSide(const Vector3& p1, const Vector3& p2, const Vector3& p3) const
{
    int32_t outsideCount = 0;
    if (GetPointSide(p1) == math::eSide::Outside)
    {
        ++outsideCount;
    }
    if (GetPointSide(p2) == math::eSide::Outside)
    {
        ++outsideCount;
    }
    if (GetPointSide(p3) == math::eSide::Outside)
    {
        ++outsideCount;
    }
    if (outsideCount == 3)
    {
        return math::eSide::Outside;
    }
    if (outsideCount == 0)
    {
        return math::eSide::Inside;
    }
    return math::eSide::Intersect;
}

} // namespace ho
