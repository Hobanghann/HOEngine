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
    Vector3 minEdges = Vector3(math::FLOAT_MAX, math::FLOAT_MAX, math::FLOAT_MAX);
    Vector3 maxEdges = Vector3(-math::FLOAT_MAX, -math::FLOAT_MAX, -math::FLOAT_MAX);

    for (int32_t i = 0; i < count; ++i)
    {
        // construct aabb edges
        if (pPositions[i].X > maxEdges.X)
        {
            maxEdges.X = pPositions[i].X;
        }
        if (pPositions[i].X < minEdges.X)
        {
            minEdges.X = pPositions[i].X;
        }
        if (pPositions[i].Y > maxEdges.Y)
        {
            maxEdges.Y = pPositions[i].Y;
        }
        if (pPositions[i].Y < minEdges.Y)
        {
            minEdges.Y = pPositions[i].Y;
        }
        if (pPositions[i].Z > maxEdges.Z)
        {
            maxEdges.Z = pPositions[i].Z;
        }
        if (pPositions[i].Z < minEdges.Z)
        {
            minEdges.Z = pPositions[i].Z;
        }
    }
    return AABB(minEdges, maxEdges);
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