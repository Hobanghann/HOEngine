#include "Plane.h"

#include "AABB.h"
#include "MathFuncs.h"
#include "Sphere.h"

namespace ho
{
math::eSide Plane::GetAabbSide(const AABB& aabb) const
{
    Vector3 nearest = aabb.MinEdges;
    Vector3 farthest = aabb.MaxEdges;
    if (Normal.X < 0.0f)
    {
        nearest.X = aabb.MaxEdges.X;
        farthest.X = aabb.MinEdges.X;
    }
    if (Normal.Y < 0.0f)
    {
        nearest.Y = aabb.MaxEdges.Y;
        farthest.Y = aabb.MinEdges.Y;
    }
    if (Normal.Z < 0.0f)
    {
        nearest.Z = aabb.MaxEdges.Z;
        farthest.Z = aabb.MinEdges.Z;
    }
    if (Normal.Dot(nearest) + D > math::EPSILON_POINT_ON_PLANE)
    {
        return math::eSide::Outside;
    }
    else if (Normal.Dot(farthest) + D > math::EPSILON_POINT_ON_PLANE)
    {
        return math::eSide::Intersect;
    }
    else
    {
        return math::eSide::Inside;
    }
}

math::eSide Plane::GetSphereSide(const Sphere& sphere) const
{
    const float res = Normal.Dot(sphere.Center) + D;
    if (res > math::EPSILON_POINT_ON_PLANE)
    {
        if (res >= sphere.Radius)
        {
            return math::eSide::Outside;
        }
        else
        {
            return math::eSide::Intersect;
        }
    }
    else
    {
        if (res >= -sphere.Radius)
        {
            return math::eSide::Intersect;
        }
        else
        {
            return math::eSide::Inside;
        }
    }
}

std::string Plane::ToString() const
{
    char buf[100];
    snprintf(buf,
             sizeof(buf),
             "%.3f*x + %.3f*y + %.3f*z + %.3f",
             static_cast<float>(Normal.X),
             static_cast<float>(Normal.Y),
             static_cast<float>(Normal.Z),
             static_cast<float>(D));
    return buf;
}
} // namespace ho