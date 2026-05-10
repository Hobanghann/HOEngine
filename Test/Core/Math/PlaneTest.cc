#include "Core/Math/Plane.h"

#include <gtest/gtest.h>

#include "Core/Math/AABB.h"
#include "Core/Math/MathFuncs.h"
#include "Core/Math/Sphere.h"
#include "Core/Math/Vector3.h"

using namespace ho;

TEST(PlaneTest, Constructor_DefaultAndAllParameters_InitializesNormalizedNormalAndD)
{
    Plane p;
    EXPECT_FLOAT_EQ(p.Normal.X, 0.0f);
    EXPECT_FLOAT_EQ(p.Normal.Y, 0.0f);
    EXPECT_FLOAT_EQ(p.Normal.Z, 0.0f);
    EXPECT_FLOAT_EQ(p.D, 0.0f);

    Plane p1(1.0f, 2.0f, 3.0f, -6.0f);
    const float mag = Vector3(1.0f, 2.0f, 3.0f).Magnitude();
    EXPECT_NEAR(p1.Normal.X, 1.0f / mag, math::EPSILON_CMP);
    EXPECT_NEAR(p1.Normal.Y, 2.0f / mag, math::EPSILON_CMP);
    EXPECT_NEAR(p1.Normal.Z, 3.0f / mag, math::EPSILON_CMP);
    EXPECT_NEAR(p1.D, -6.0f / mag, math::EPSILON_CMP);

    Vector3 normal(0.0f, 1.0f, 0.0f);
    Vector3 point(0.0f, 5.0f, 0.0f);
    Plane p2(normal, point);
    EXPECT_TRUE(p2.Normal.IsEqualApprox(Vector3(0.0f, 1.0f, 0.0f)));
    EXPECT_NEAR(p2.D, -5.0f, math::EPSILON_CMP);

    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(1.0f, 0.0f, 0.0f);
    Vector3 v3(0.0f, 1.0f, 0.0f);

    Plane p3(v1, v2, v3, math::eClockDirection::CounterClockWise);
    EXPECT_TRUE(p3.Normal.IsEqualApprox(Vector3(0.0f, 0.0f, 1.0f)));
    EXPECT_NEAR(p3.D, 0.0f, math::EPSILON_CMP);

    Plane p4(v1, v2, v3, math::eClockDirection::ClockWise);
    EXPECT_TRUE(p4.Normal.IsEqualApprox(Vector3(0.0f, 0.0f, -1.0f)));
    EXPECT_NEAR(p4.D, 0.0f, math::EPSILON_CMP);
}

TEST(PlaneTest, OperatorAssign_AnotherPlane_CopiesValues)
{
    Plane p(1.0f, 2.0f, 3.0f, -6.0f);
    Plane p1;
    p1 = p;

    EXPECT_EQ(p, p1);
}

TEST(PlaneTest, OperatorComparison_DifferentPlanes_ReturnsCorrectBoolean)
{
    Plane p1(0.0f, 1.0f, 0.0f, 5.0f);
    Plane p2(0.0f, 1.0f, 0.0f, 5.0f);
    Plane p3(0.0f, 1.0f, 0.0f, 6.0f);

    EXPECT_TRUE(p1 == p2);
    EXPECT_FALSE(p1 != p2);
    EXPECT_FALSE(p1 == p3);
    EXPECT_TRUE(p1 != p3);
}

TEST(PlaneTest, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Plane p1(0.0f, 1.0f, 0.0f, 5.0f);
    Plane p2(0.0f, 1.000001f, 0.0f, 5.000001f);

    EXPECT_TRUE(p1.IsEqualApprox(p2));
    EXPECT_FALSE(p1.IsNotEqualApprox(p2));
}

TEST(PlaneTest, GetDistToPoint_PointsInDifferentSpaces_ReturnsSignedDistance)
{
    Plane p(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 5.0f, 0.0f));

    Vector3 front(0.0f, 10.0f, 0.0f);
    Vector3 on(0.0f, 5.0f, 0.0f);
    Vector3 back(0.0f, 0.0f, 0.0f);

    EXPECT_NEAR(p.GetDistToPoint(front), 5.0f, math::EPSILON_CMP);
    EXPECT_NEAR(p.GetDistToPoint(on), 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(p.GetDistToPoint(back), -5.0f, math::EPSILON_CMP);
}

TEST(PlaneTest, Project_PointToPlane_ReturnsClosestPointOnPlane)
{
    Plane p(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 5.0f, 0.0f));

    Vector3 front(0.0f, 10.0f, 0.0f);
    Vector3 on(0.0f, 5.0f, 0.0f);
    Vector3 back(0.0f, 0.0f, 0.0f);

    EXPECT_TRUE(p.Project(front).IsEqualApprox(Vector3(0.0f, 5.0f, 0.0f)));
    EXPECT_TRUE(p.Project(on).IsEqualApprox(Vector3(0.0f, 5.0f, 0.0f)));
    EXPECT_TRUE(p.Project(back).IsEqualApprox(Vector3(0.0f, 5.0f, 0.0f)));
}

TEST(PlaneTest, GetPointSide_PointsInFrontAndBack_ReturnsCorrectSide)
{
    Plane p(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 5.0f, 0.0f));

    Vector3 front(0.0f, 10.0f, 0.0f);
    Vector3 on(0.0f, 5.0f, 0.0f);
    Vector3 back(0.0f, 0.0f, 0.0f);

    EXPECT_EQ(p.GetPointSide(front), math::eSide::Outside);
    EXPECT_EQ(p.GetPointSide(on), math::eSide::On);
    EXPECT_EQ(p.GetPointSide(back), math::eSide::Inside);
}

TEST(PlaneTest, GetLineSide_PointsAcrossPlane_ReturnsIntersectOrCorrectSide)
{
    Plane p(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 5.0f, 0.0f));

    Vector3 front1(0.0f, 10.0f, 0.0f); // Outside
    Vector3 back(0.0f, 0.0f, 0.0f);    // Inside
    Vector3 front2(0.0f, 6.0f, 0.0f);  // Outside
    Vector3 on(0.0f, 5.0f, 0.0f);

    EXPECT_EQ(p.GetLineSide(front1, front2), math::eSide::Outside);
    EXPECT_EQ(p.GetLineSide(back, on), math::eSide::Inside);
    EXPECT_EQ(p.GetLineSide(front1, back), math::eSide::Intersect);
}

TEST(PlaneTest, GetTriangleSide_PointsInVariousPositions_ReturnsCorrectSide)
{
    Plane p(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 5.0f, 0.0f));

    Vector3 front1(0.0f, 10.0f, 0.0f);
    Vector3 front2(1.0f, 8.0f, 0.0f);
    Vector3 front3(-1.0f, 6.0f, 0.0f);
    EXPECT_EQ(p.GetTriangleSide(front1, front2, front3), math::eSide::Outside);

    Vector3 on1(0.0f, 5.0f, 0.0f);
    Vector3 on2(1.0f, 5.0f, 0.0f);
    Vector3 on3(-1.0f, 5.0f, 0.0f);
    EXPECT_EQ(p.GetTriangleSide(on1, on2, on3), math::eSide::Inside);

    Vector3 front(0.0f, 10.0f, 0.0f);
    Vector3 on(0.0f, 5.0f, 0.0f);
    Vector3 back(0.0f, 0.0f, 0.0f);
    EXPECT_EQ(p.GetTriangleSide(front, on, back), math::eSide::Intersect);
}

TEST(PlaneTest, GetAabbSide_BoxesInVariousPositions_ReturnsCorrectSide)
{
    Plane p(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 5.0f, 0.0f));

    AABB above(Vector3(-1.0f, 6.0f, -1.0f), Vector3(1.0f, 7.0f, 1.0f));
    EXPECT_EQ(p.GetAabbSide(above), math::eSide::Outside);

    AABB below(Vector3(-1.0f, 3.0f, -1.0f), Vector3(1.0f, 4.0f, 1.0f));
    EXPECT_EQ(p.GetAabbSide(below), math::eSide::Inside);

    AABB intersect(Vector3(-1.0f, 4.0f, -1.0f), Vector3(1.0f, 6.0f, 1.0f));
    EXPECT_EQ(p.GetAabbSide(intersect), math::eSide::Intersect);
}

TEST(PlaneTest, GetSphereSide_SpheresInVariousPositions_ReturnsCorrectSide)
{
    Plane p(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 5.0f, 0.0f));

    Sphere out(Vector3(0.0f, 8.0f, 0.0f), 1.0f);
    EXPECT_EQ(p.GetSphereSide(out), math::eSide::Outside);

    Sphere in(Vector3(0.0f, 2.0f, 0.0f), 1.0f);
    EXPECT_EQ(p.GetSphereSide(in), math::eSide::Inside);

    Sphere inter(Vector3(0.0f, 6.0f, 0.0f), 2.0f);
    EXPECT_EQ(p.GetSphereSide(inter), math::eSide::Intersect);

    Sphere on(Vector3(0.0f, 5.0f, 0.0f), 1.0f);
    EXPECT_EQ(p.GetSphereSide(on), math::eSide::Intersect);
}