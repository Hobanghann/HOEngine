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
    Plane left = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    coefficients = -(m.Row3 - m.Row0);
    Plane right = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    coefficients = -(m.Row3 + m.Row1);
    Plane bottom = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    coefficients = -(m.Row3 - m.Row1);
    Plane top = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    coefficients = -(m.Row3 + m.Row2);
    Plane near = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);

    coefficients = -(m.Row3 - m.Row2);
    Plane far = Plane(coefficients.X, coefficients.Y, coefficients.Z, coefficients.W);
    return Frustum(left, right, bottom, top, near, far);
}

Frustum::Frustum(const Frustum& frustum)
{
    planes[static_cast<int32_t>(ePlanePos::Left)] = frustum.planes[static_cast<int32_t>(ePlanePos::Left)];
    planes[static_cast<int32_t>(ePlanePos::Right)] = frustum.planes[static_cast<int32_t>(ePlanePos::Right)];
    planes[static_cast<int32_t>(ePlanePos::Bottom)] = frustum.planes[static_cast<int32_t>(ePlanePos::Bottom)];
    planes[static_cast<int32_t>(ePlanePos::Top)] = frustum.planes[static_cast<int32_t>(ePlanePos::Top)];
    planes[static_cast<int32_t>(ePlanePos::Near)] = frustum.planes[static_cast<int32_t>(ePlanePos::Near)];
    planes[static_cast<int32_t>(ePlanePos::Far)] = frustum.planes[static_cast<int32_t>(ePlanePos::Far)];
}

Frustum::Frustum(
    const Plane& left, const Plane& right, const Plane& bottom, const Plane& top, const Plane& near, const Plane& far)
{
    planes[static_cast<int32_t>(ePlanePos::Left)] = left;
    planes[static_cast<int32_t>(ePlanePos::Right)] = right;
    planes[static_cast<int32_t>(ePlanePos::Bottom)] = bottom;
    planes[static_cast<int32_t>(ePlanePos::Top)] = top;
    planes[static_cast<int32_t>(ePlanePos::Near)] = near;
    planes[static_cast<int32_t>(ePlanePos::Far)] = far;
}

Frustum& Frustum::operator=(const Frustum& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    planes[static_cast<int32_t>(ePlanePos::Left)] = rhs.planes[static_cast<int32_t>(ePlanePos::Left)];
    planes[static_cast<int32_t>(ePlanePos::Right)] = rhs.planes[static_cast<int32_t>(ePlanePos::Right)];
    planes[static_cast<int32_t>(ePlanePos::Bottom)] = rhs.planes[static_cast<int32_t>(ePlanePos::Bottom)];
    planes[static_cast<int32_t>(ePlanePos::Top)] = rhs.planes[static_cast<int32_t>(ePlanePos::Top)];
    planes[static_cast<int32_t>(ePlanePos::Near)] = rhs.planes[static_cast<int32_t>(ePlanePos::Near)];
    planes[static_cast<int32_t>(ePlanePos::Far)] = rhs.planes[static_cast<int32_t>(ePlanePos::Far)];
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
        int32_t idx = static_cast<int32_t>(pos);
        if (planes[idx] != rhs.planes[idx])
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
        int32_t idx = static_cast<int32_t>(pos);
        if (!planes[idx].IsEqualApprox(rhs.planes[idx]))
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
        const Plane& plane = planes[static_cast<int32_t>(pos)];
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
        const Plane& plane = planes[static_cast<int32_t>(pos)];
        math::eSide side1 = plane.GetPointSide(p1);
        math::eSide side2 = plane.GetPointSide(p2);

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
        const Plane& plane = planes[static_cast<int32_t>(pos)];
        math::eSide side1 = plane.GetPointSide(p1);
        math::eSide side2 = plane.GetPointSide(p2);
        math::eSide side3 = plane.GetPointSide(p3);

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
        const Plane& plane = planes[static_cast<int32_t>(pos)];
        math::eSide side = plane.GetAabbSide(aabb);
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
        const Plane& plane = planes[static_cast<int32_t>(pos)];
        math::eSide side = plane.GetSphereSide(sphere);
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
             planes[static_cast<int32_t>(ePlanePos::Left)].ToString().c_str(),
             planes[static_cast<int32_t>(ePlanePos::Right)].ToString().c_str(),
             planes[static_cast<int32_t>(ePlanePos::Bottom)].ToString().c_str(),
             planes[static_cast<int32_t>(ePlanePos::Top)].ToString().c_str(),
             planes[static_cast<int32_t>(ePlanePos::Near)].ToString().c_str(),
             planes[static_cast<int32_t>(ePlanePos::Far)].ToString().c_str());
    return buf;
}
} // namespace ho