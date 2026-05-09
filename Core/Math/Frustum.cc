#include "Frustum.h"

#include "AABB.h"
#include "Matrix4x4.h"
#include "Sphere.h"
#include "Vector3.h"

namespace ho
{

Frustum Frustum::FromMatrix4x4(const Matrix4x4& m)
{
    Vector4 coefficients = -(m.Row3 + m.Row0);
    const Plane left = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    coefficients = -(m.Row3 - m.Row0);
    const Plane right = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    coefficients = -(m.Row3 + m.Row1);
    const Plane bottom = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    coefficients = -(m.Row3 - m.Row1);
    const Plane top = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    coefficients = -(m.Row3 + m.Row2);
    const Plane near = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    coefficients = -(m.Row3 - m.Row2);
    const Plane far = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    return Frustum(left, right, bottom, top, near, far);
}

Frustum::Frustum(const Frustum& frustum)
{
    Planes[static_cast<int32_t>(ePlanePos::Left)] = frustum.Planes[static_cast<int32_t>(ePlanePos::Left)];
    Planes[static_cast<int32_t>(ePlanePos::Right)] = frustum.Planes[static_cast<int32_t>(ePlanePos::Right)];
    Planes[static_cast<int32_t>(ePlanePos::Bottom)] = frustum.Planes[static_cast<int32_t>(ePlanePos::Bottom)];
    Planes[static_cast<int32_t>(ePlanePos::Top)] = frustum.Planes[static_cast<int32_t>(ePlanePos::Top)];
    Planes[static_cast<int32_t>(ePlanePos::Near)] = frustum.Planes[static_cast<int32_t>(ePlanePos::Near)];
    Planes[static_cast<int32_t>(ePlanePos::Far)] = frustum.Planes[static_cast<int32_t>(ePlanePos::Far)];
}

Frustum::Frustum(
    const Plane& left, const Plane& right, const Plane& bottom, const Plane& top, const Plane& near, const Plane& far)
{
    Planes[static_cast<int32_t>(ePlanePos::Left)] = left;
    Planes[static_cast<int32_t>(ePlanePos::Right)] = right;
    Planes[static_cast<int32_t>(ePlanePos::Bottom)] = bottom;
    Planes[static_cast<int32_t>(ePlanePos::Top)] = top;
    Planes[static_cast<int32_t>(ePlanePos::Near)] = near;
    Planes[static_cast<int32_t>(ePlanePos::Far)] = far;
}

Frustum& Frustum::operator=(const Frustum& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    Planes[static_cast<int32_t>(ePlanePos::Left)] = rhs.Planes[static_cast<int32_t>(ePlanePos::Left)];
    Planes[static_cast<int32_t>(ePlanePos::Right)] = rhs.Planes[static_cast<int32_t>(ePlanePos::Right)];
    Planes[static_cast<int32_t>(ePlanePos::Bottom)] = rhs.Planes[static_cast<int32_t>(ePlanePos::Bottom)];
    Planes[static_cast<int32_t>(ePlanePos::Top)] = rhs.Planes[static_cast<int32_t>(ePlanePos::Top)];
    Planes[static_cast<int32_t>(ePlanePos::Near)] = rhs.Planes[static_cast<int32_t>(ePlanePos::Near)];
    Planes[static_cast<int32_t>(ePlanePos::Far)] = rhs.Planes[static_cast<int32_t>(ePlanePos::Far)];
    return *this;
}

static Frustum::ePlanePos allPlanePos[] = {Frustum::ePlanePos::Left,
                                           Frustum::ePlanePos::Right,
                                           Frustum::ePlanePos::Bottom,
                                           Frustum::ePlanePos::Top,
                                           Frustum::ePlanePos::Near,
                                           Frustum::ePlanePos::Far};

bool Frustum::operator==(const Frustum& rhs) const
{
    for (auto pos : allPlanePos)
    {
        const int32_t idx = static_cast<int32_t>(pos);
        if (Planes[idx] != rhs.Planes[idx])
        {
            return false;
        }
    }
    return true;
}

bool Frustum::operator!=(const Frustum& rhs) const
{
    return !(*this == rhs);
}

bool Frustum::IsEqualApprox(const Frustum& rhs) const
{
    for (auto pos : allPlanePos)
    {
        const int32_t idx = static_cast<int32_t>(pos);
        if (!Planes[idx].IsEqualApprox(rhs.Planes[idx]))
        {
            return false;
        }
    }
    return true;
}

bool Frustum::IsNotEqualApprox(const Frustum& rhs) const
{
    return !IsEqualApprox(rhs);
}

math::eSide Frustum::GetPointSide(const Vector3& p) const
{
    for (auto pos : allPlanePos)
    {
        const Plane& plane = Planes[static_cast<int32_t>(pos)];
        if (plane.GetPointSide(p) == math::eSide::Outside)
        {
            return math::eSide::Outside;
        }
    }
    return math::eSide::Inside;
}

math::eSide Frustum::GetLineSide(const Vector3& p1, const Vector3& p2) const
{
    bool bFullyInside = true;

    for (auto pos : allPlanePos)
    {
        const Plane& plane = Planes[static_cast<int32_t>(pos)];
        const math::eSide side1 = plane.GetPointSide(p1);
        const math::eSide side2 = plane.GetPointSide(p2);

        if (side1 == math::eSide::Outside && side2 == math::eSide::Outside)
        {
            return math::eSide::Outside;
        }

        if (side1 != math::eSide::Inside || side2 != math::eSide::Inside)
        {
            bFullyInside = false;
        }
    }
    if (bFullyInside)
    {
        return math::eSide::Inside;
    }

    return math::eSide::Intersect;
}

math::eSide Frustum::GetTriangleSide(const Vector3& p1, const Vector3& p2, const Vector3& p3) const
{
    bool bFullyInside = true;

    for (auto pos : allPlanePos)
    {
        const Plane& plane = Planes[static_cast<int32_t>(pos)];
        const math::eSide side1 = plane.GetPointSide(p1);
        const math::eSide side2 = plane.GetPointSide(p2);
        const math::eSide side3 = plane.GetPointSide(p3);

        if (side1 == math::eSide::Outside && side2 == math::eSide::Outside && side3 == math::eSide::Outside)
        {
            return math::eSide::Outside;
        }

        if (side1 != math::eSide::Inside || side2 != math::eSide::Inside || side3 != math::eSide::Inside)
        {
            bFullyInside = false;
        }
    }

    if (bFullyInside)
    {
        return math::eSide::Inside;
    }

    return math::eSide::Intersect;
}

math::eSide Frustum::GetAabbSide(const AABB& aabb) const
{
    bool bIntersect = false;
    for (auto pos : allPlanePos)
    {
        const Plane& plane = Planes[static_cast<int32_t>(pos)];
        const math::eSide side = plane.GetAabbSide(aabb);
        if (side == math::eSide::Outside)
        {
            return math::eSide::Outside;
        }
        if (side == math::eSide::Intersect)
        {
            bIntersect = true;
        }
    }
    if (bIntersect)
    {
        return math::eSide::Intersect;
    }
    return math::eSide::Inside;
}

math::eSide Frustum::GetSphereSide(const Sphere& sphere) const
{
    bool bIntersect = false;
    for (auto pos : allPlanePos)
    {
        const Plane& plane = Planes[static_cast<int32_t>(pos)];
        const math::eSide side = plane.GetSphereSide(sphere);
        if (side == math::eSide::Outside)
        {
            return math::eSide::Outside;
        }
        if (side == math::eSide::Intersect)
        {
            bIntersect = true;
        }
    }
    if (bIntersect)
    {
        return math::eSide::Intersect;
    }
    return math::eSide::Inside;
}

std::string Frustum::ToString() const
{
    char buf[300];
    snprintf(buf,
             sizeof(buf),
             "Left: %s\nRight: %s\nTop: %s\nBottom: %s\nNear: %s\nFar: %s",
             Planes[static_cast<int32_t>(ePlanePos::Left)].ToString().c_str(),
             Planes[static_cast<int32_t>(ePlanePos::Right)].ToString().c_str(),
             Planes[static_cast<int32_t>(ePlanePos::Bottom)].ToString().c_str(),
             Planes[static_cast<int32_t>(ePlanePos::Top)].ToString().c_str(),
             Planes[static_cast<int32_t>(ePlanePos::Near)].ToString().c_str(),
             Planes[static_cast<int32_t>(ePlanePos::Far)].ToString().c_str());
    return buf;
}
} // namespace ho