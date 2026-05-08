#include "AABB.h"

namespace ho
{

AABB AABB::FromPositions(const Vector3* pPositions, int32_t count)
{
    HO_ASSERT(pPositions, "pPositions is null.");
    HO_ASSERT(count > 0, "count must be greater than zero.");
    if (count == 1)
    {
        return AABB(pPositions[0], pPositions[0]);
    }
    Vector3 MinEdges = Vector3(math::FLOAT_MAX, math::FLOAT_MAX, math::FLOAT_MAX);
    Vector3 MaxEdges = Vector3(-math::FLOAT_MAX, -math::FLOAT_MAX, -math::FLOAT_MAX);

    for (int32_t i = 0; i < count; ++i)
    {
        // construct aabb edges
        if (pPositions[i].X > MaxEdges.X)
        {
            MaxEdges.X = pPositions[i].X;
        }
        if (pPositions[i].X < MinEdges.X)
        {
            MinEdges.X = pPositions[i].X;
        }
        if (pPositions[i].Y > MaxEdges.Y)
        {
            MaxEdges.Y = pPositions[i].Y;
        }
        if (pPositions[i].Y < MinEdges.Y)
        {
            MinEdges.Y = pPositions[i].Y;
        }
        if (pPositions[i].Z > MaxEdges.Z)
        {
            MaxEdges.Z = pPositions[i].Z;
        }
        if (pPositions[i].Z < MinEdges.Z)
        {
            MinEdges.Z = pPositions[i].Z;
        }
    }
    return AABB(MinEdges, MaxEdges);
}

std::string AABB::ToString() const
{
    char buf[100];
    snprintf(buf,
             sizeof(buf),
             "Min: (%.3f, %.3f, %.3f), Max : (%.3f, %.3f, %.3f)",
             MinEdges.X,
             MinEdges.Y,
             MinEdges.Z,
             MaxEdges.X,
             MaxEdges.Y,
             MaxEdges.Z);
    return buf;
}
} // namespace ho