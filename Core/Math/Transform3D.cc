#include "Transform3D.h"

#include "AABB.h"
#include "Sphere.h"

namespace ho
{
AABB Transform3D::Transform(const AABB& volume) const
{
    // Transforms the local AABB into a world-space AABB using Arvo's Algorithm.
    //
    // 1. Instead of transforming all 8 corners, we optimize by calculating the world center
    //    and then determining the new world extents based on the sum of projected lengths.
    //
    // 2. Transform the local center to world space to get the new world center.
    //
    // 3. For the new world extents, we project each local basis axis onto the world axes
    //    and sum their absolute contribution.
    //
    // 4. By taking the absolute value of the matrix elements, we ensure that every
    //    local axis component adds to the overall "thickness" of the box regardless of orientation.
    //
    // This effectively finds the outermost boundary (the corner) in a single matrix-vector operation.
    const Vector3 extents = (volume.MaxEdges - volume.MinEdges) * 0.5f;

    const Vector3 newCenter = Transform(volume.GetCenter());

    Vector3 newExtents;
    newExtents.X = math::Abs(Matrix.Data[0][0]) * extents.X + math::Abs(Matrix.Data[0][1]) * extents.Y +
                   math::Abs(Matrix.Data[0][2]) * extents.Z;

    newExtents.Y = math::Abs(Matrix.Data[1][0]) * extents.X + math::Abs(Matrix.Data[1][1]) * extents.Y +
                   math::Abs(Matrix.Data[1][2]) * extents.Z;

    newExtents.Z = math::Abs(Matrix.Data[2][0]) * extents.X + math::Abs(Matrix.Data[2][1]) * extents.Y +
                   math::Abs(Matrix.Data[2][2]) * extents.Z;

    return AABB(newCenter - newExtents, newCenter + newExtents);
}

Sphere Transform3D::Transform(const Sphere& volume) const
{
    const Vector3 newCenter = Transform(volume.Center);

    const float maxScale = math::Max(GetScaleX(), math::Max(GetScaleY(), GetScaleZ()));

    return Sphere(newCenter, volume.Radius * maxScale);
}

std::string Transform3D::ToString() const
{
    char buf[256];
    std::snprintf(buf,
                  sizeof(buf),
                  "Transform3D Origin: %s, Basis: %s",
                  GetOrigin().ToString().c_str(),
                  GetBasis().ToString().c_str());
    return std::string(buf);
}

} // namespace ho
