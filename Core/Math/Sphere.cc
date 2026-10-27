#include "Sphere.h"

#include "AABB.h"

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

Sphere Sphere::FromPositionsRitter(const Vector3* pPositions, int32_t count)
{
    HO_ASSERT(pPositions, "pPositions is null.");
    HO_ASSERT(count > 0, "count must be greater than zero.");
    if (count == 1)
    {
        return Sphere(pPositions[0], 0.0f);
    }

    // Set initial sphere
    Vector3 basePos = pPositions[0];
    Vector3 p1;
    float maxSqrdDist = -math::FLOAT_MAX;

    for (int32_t i = 0; i < count; ++i)
    {
        const float sqrdDist = math::SqrdDistance(basePos, pPositions[i]);
        if (sqrdDist > maxSqrdDist)
        {
            maxSqrdDist = sqrdDist;
            p1 = pPositions[i];
        }
    }

    basePos = p1;
    Vector3 p2;
    maxSqrdDist = -math::FLOAT_MAX;

    for (int32_t i = 0; i < count; ++i)
    {
        const float sqrdDist = math::SqrdDistance(basePos, pPositions[i]);
        if (sqrdDist > maxSqrdDist)
        {
            maxSqrdDist = sqrdDist;
            p2 = pPositions[i];
        }
    }

    Vector3 center = (p1 + p2) * 0.5f;
    float radius = (p1 - center).Magnitude();

    // Expand sphere
    for (int32_t i = 0; i < count; i++)
    {
        const Vector3& pos = pPositions[i];
        const float sqrdDist = math::SqrdDistance(pos, center);

        if (sqrdDist > radius * radius)
        {
            const float dist = math::Sqrt(sqrdDist);
            if (dist > 0.0f)
            {
                const float newRadius = (radius + dist) * 0.5f;
                const Vector3 dir = (pos - center) / dist;
                center += dir * (newRadius - radius);
                radius = newRadius;
            }
        }
    }

    return Sphere(center, radius);
}

math::eSide Sphere::GetSide(const Sphere& sphere) const
{
    const float dx = sphere.Center.Data[0] - Center.Data[0];
    const float dy = sphere.Center.Data[1] - Center.Data[1];
    const float dz = sphere.Center.Data[2] - Center.Data[2];
    const float sqDist = dx * dx + dy * dy + dz * dz;

    const float sumRadius = Radius + sphere.Radius;
    if (sqDist > sumRadius * sumRadius)
    {
        return math::eSide::Outside;
    }

    if (Radius >= sphere.Radius)
    {
        const float subRadius = Radius - sphere.Radius;
        if (sqDist <= subRadius * subRadius)
        {
            return math::eSide::Inside;
        }
    }

    return math::eSide::Intersect;
}

math::eSide Sphere::GetSide(const AABB& aabb) const
{
    float sqDistClosest = 0.0f;
    float sqDistFarthest = 0.0f;

    for (int32_t i = 0; i < 3; ++i)
    {
        const float center = Center.Data[i];
        const float minEdge = aabb.MinEdges.Data[i];
        const float maxEdge = aabb.MaxEdges.Data[i];

        if (center < minEdge)
        {
            const float diff = minEdge - center;
            sqDistClosest += diff * diff;
        }
        else if (center > maxEdge)
        {
            const float diff = center - maxEdge;
            sqDistClosest += diff * diff;
        }

        const float dMin = (center > minEdge) ? (center - minEdge) : (minEdge - center);
        const float dMax = (center > maxEdge) ? (center - maxEdge) : (maxEdge - center);
        const float maxDiff = (dMin > dMax) ? dMin : dMax;

        sqDistFarthest += maxDiff * maxDiff;
    }

    const float sqRadius = Radius * Radius;

    if (sqDistClosest > sqRadius)
    {
        return math::eSide::Outside;
    }

    if (sqDistFarthest <= sqRadius)
    {
        return math::eSide::Inside;
    }

    return math::eSide::Intersect;
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
