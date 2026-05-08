#pragma once

#include <string>

#include "MathDefs.h"
#include "Plane.h"

// frustum defined by 6 planes in view, world, local space
// 6 planes that define frustum defined by affine_matrix
namespace ho
{
struct Vector3;
struct Matrix4x4;
struct Plane;
struct AABB;
struct Sphere;

struct Frustum
{
    enum class ePlanePos
    {
        Left = 0,
        Right = 1,
        Bottom = 2,
        Top = 3,
        Near = 4,
        Far = 5
    };

    [[nodiscard]] static Frustum FromMatrix4x4(const Matrix4x4& m);

    Frustum() = default;
    Frustum(const Frustum& frustum);
    Frustum(const Plane& left,
            const Plane& right,
            const Plane& bottom,
            const Plane& top,
            const Plane& near,
            const Plane& far);
    Frustum& operator=(const Frustum& rhs);
    ~Frustum() = default;

    [[nodiscard]] bool operator==(const Frustum& rhs) const;
    [[nodiscard]] bool operator!=(const Frustum& rhs) const;

    [[nodiscard]] bool IsEqualApprox(const Frustum& rhs) const;
    [[nodiscard]] bool IsNotEqualApprox(const Frustum& rhs) const;

    [[nodiscard]] math::eSide GetPointSide(const Vector3& p) const;
    [[nodiscard]] math::eSide GetLineSide(const Vector3& p1, const Vector3& p2) const;
    [[nodiscard]] math::eSide GetTriangleSide(const Vector3& p1, const Vector3& p2, const Vector3& p3) const;
    [[nodiscard]] math::eSide GetAabbSide(const AABB& aabb) const;
    [[nodiscard]] math::eSide GetSphereSide(const Sphere& sphere) const;

    std::string ToString() const;

    Plane planes[6] = {Plane()};
};
} // namespace ho
