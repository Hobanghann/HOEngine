#include "sphere.h"

namespace ho
{

Sphere Sphere::FromPositionsCentroid(const Vector3* pPositions, int32_t count)
{
    HO_ASSERT(pPositions, "pPositions is null.");
    HO_ASSERT(count > 0, "count must be greater than zero.");
    if (count == 1)
    {
        return Sphere(pPositions[0], 0.0f);
    }
    Vector3 center;
    float radius = 0;

    int n = 0;
    for (int32_t i = 0; i < count; ++i)
    {
        // construct sphere center
        n += 1;
        const Vector3& x = Vector3(pPositions[i]);
        center += (x - center) * (1.0f / static_cast<float>(n));
    }

    // construct sphere radius
    float maxSqrdDist = 0;
    for (int32_t i = 0; i < count; ++i)
    {
        const float sqrdDist = math::SqrdDistance(center, pPositions[i]);
        maxSqrdDist = math::Max(maxSqrdDist, sqrdDist);
    }
    radius = math::Sqrt(maxSqrdDist);

    return Sphere(center, radius);
}

// @brief Constructs a bounding sphere for the mesh using Ritter's Algorithm.
//
// This method provides a fast, linear-time (O(N)) approximation of the
// minimum bounding sphere. While not perfectly optimal, it produces a
// sphere that is generally much tighter than one based on the simple
// average of vertex positions.
//
// The algorithm proceeds in two main steps:
//
// 1.  Find Initial Sphere:
// - An initial guess is made by finding the two most distant points in the
// vertex set to form the sphere's initial diameter.
// - (Optimization: To avoid an O(N^2) search, we find the min/max points
// along each axis (X, Y, Z) and use the most distant pair among them).
//
// 2.  Expand the Sphere:
// - The algorithm iterates through all vertices one more time.
// - If a vertex is found outside the current sphere, the sphere is expanded
// just enough to include both the old sphere and this new point. The new
// sphere will pass through the new point and the point on the old
// sphere's surface farthest from the new point.
//
// This process is repeated until all vertices are contained within the sphere.

Sphere Sphere::FromPositionsRitter(const Vector3* pPositions, int32_t count)
{
    HO_ASSERT(pPositions, "pPositions is null.");
    HO_ASSERT(count > 0, "count must be greater than zero.");
    if (count == 1)
    {
        return Sphere(pPositions[0], 0.0f);
    }

    // Seed the initial sphere using the most separated pair among axis extremes
    int32_t xMinIdx = 0;
    int32_t xMaxIdx = 0;
    int32_t yMinIdx = 0;
    int32_t yMaxIdx = 0;
    int32_t zMinIdx = 0;
    int32_t zMaxIdx = 0;

    float xMin = math::FLOAT_MAX;
    float yMin = math::FLOAT_MAX;
    float zMin = math::FLOAT_MAX;
    float xMax = -math::FLOAT_MAX;
    float yMax = -math::FLOAT_MAX;
    float zMax = -math::FLOAT_MAX;

    for (int32_t i = 0; i < count; ++i)
    {
        const Vector3& pos = pPositions[i];
        if (pos.X < xMin)
        {
            xMin = pos.X;
            xMinIdx = i;
        }
        if (pos.X > xMax)
        {
            xMax = pos.X;
            xMaxIdx = i;
        }
        if (pos.Y < yMin)
        {
            yMin = pos.Y;
            yMinIdx = i;
        }
        if (pos.Y > yMax)
        {
            yMax = pos.Y;
            yMaxIdx = i;
        }
        if (pos.Z < zMin)
        {
            zMin = pos.Z;
            zMinIdx = i;
        }
        if (pos.Z > zMax)
        {
            zMax = pos.Z;
            zMaxIdx = i;
        }
    }

    const Vector3& xMinVec = pPositions[xMinIdx];
    const Vector3& xMaxVec = pPositions[xMaxIdx];
    const Vector3& yMinVec = pPositions[yMinIdx];
    const Vector3& yMaxVec = pPositions[yMaxIdx];
    const Vector3& zMinVec = pPositions[zMinIdx];
    const Vector3& zMaxVec = pPositions[zMaxIdx];

    const float xSqrdDist = math::SqrdDistance(xMinVec, xMaxVec);
    const float ySqrdDist = math::SqrdDistance(yMinVec, yMaxVec);
    const float zSqrdDist = math::SqrdDistance(zMinVec, zMaxVec);

    Vector3 a;
    Vector3 b;
    if (xSqrdDist > ySqrdDist && xSqrdDist > zSqrdDist)
    {
        a = xMinVec;
        b = xMaxVec;
    }
    else if (ySqrdDist > zSqrdDist)
    {
        a = yMinVec;
        b = yMaxVec;
    }
    else
    {
        a = zMinVec;
        b = zMaxVec;
    }

    // Initial sphere: diameter AB
    Vector3 center = (a + b) * 0.5f;
    float radius = math::Sqrt(math::SqrdDistance(a, b)) * 0.5f;

    // Ritter expansion: adjust only when a point lies outside the current sphere
    for (int32_t i = 0; i < count; i++)
    {
        const Vector3& pos = pPositions[i];
        const float sqrdDist = math::SqrdDistance(pos, center);

        // If the point is outside the sphere (with a small tolerance)
        if (sqrdDist > radius * radius * (1.0f + math::EPSILON_CMP2))
        {
            const float dist = math::Sqrt(sqrdDist);
            if (dist > 0.0f)
            {
                const float newRadius = (radius + dist) * 0.5f;
                // Move center toward pos by (newf - radius) along the direction to pos
                const Vector3 dir = (pos - center) / dist;
                center += dir * (newRadius - radius);
                radius = newRadius;
            }
        }
    }

    return Sphere(center, radius);
}

Sphere Sphere::Merge(const Sphere& lhs, const Sphere& rhs)
{
    const Vector3 center1 = lhs.Center;
    const Vector3 center2 = rhs.Center;
    const float radius1 = lhs.Radius;
    const float radius2 = rhs.Radius;

    const Vector3 diff = center2 - center1;
    const float dist = diff.Magnitude();

    if (radius1 >= dist + radius2)
    {
        return lhs;
    }
    if (radius2 >= dist + radius1)
    {
        return rhs;
    }

    const float newRadius = (dist + radius1 + radius2) * 0.5f;

    const Vector3 dir = diff / dist;
    const Vector3 newCenter = center1 + dir * (newRadius - radius1);

    return Sphere{newCenter, newRadius};
}

std::string Sphere::ToString() const
{
    char buf[100];
    snprintf(buf,
             sizeof(buf),
             "Center: (%.3f, %.3f, %.3f), Radius : %.3f",
             static_cast<float>(Center.X),
             static_cast<float>(Center.Y),
             static_cast<float>(Center.Z),
             static_cast<float>(Radius));
    return buf;
}
} // namespace ho
