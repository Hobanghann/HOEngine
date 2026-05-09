#include "Core/Math/Frustum.h"

#include <gtest/gtest.h>

#include "Core/Math/AABB.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Sphere.h"
#include "Core/Math/Vector3.h"

using namespace ho;

static Frustum::ePlanePos allPlanePos[] = {Frustum::ePlanePos::Left,
                                           Frustum::ePlanePos::Right,
                                           Frustum::ePlanePos::Bottom,
                                           Frustum::ePlanePos::Top,
                                           Frustum::ePlanePos::Near,
                                           Frustum::ePlanePos::Far};

class FrustumTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        defaultPlane = Plane();
        left = Plane(Vector3(-1.0f, 0.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f));
        right = Plane(Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
        bottom = Plane(Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
        top = Plane(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
        Near = Plane(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f));
        far = Plane(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f));

        defaultFrustum = Frustum();
        f1 = Frustum(left, right, bottom, top, Near, far);
    }

    Frustum defaultFrustum, f1;
    Plane defaultPlane, left, right, bottom, top, Near, far;
};

TEST_F(FrustumTest, Constructor_DefaultAndAllParameters_InitializesCorrectPlanes)
{
    EXPECT_EQ(defaultFrustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Left)], defaultPlane);
    EXPECT_EQ(defaultFrustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Right)], defaultPlane);
    EXPECT_EQ(defaultFrustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Bottom)], defaultPlane);
    EXPECT_EQ(defaultFrustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Top)], defaultPlane);
    EXPECT_EQ(defaultFrustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Near)], defaultPlane);
    EXPECT_EQ(defaultFrustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Far)], defaultPlane);

    EXPECT_EQ(f1.Planes[static_cast<int32_t>(Frustum::ePlanePos::Left)], left);
    EXPECT_EQ(f1.Planes[static_cast<int32_t>(Frustum::ePlanePos::Right)], right);
    EXPECT_EQ(f1.Planes[static_cast<int32_t>(Frustum::ePlanePos::Bottom)], bottom);
    EXPECT_EQ(f1.Planes[static_cast<int32_t>(Frustum::ePlanePos::Top)], top);
    EXPECT_EQ(f1.Planes[static_cast<int32_t>(Frustum::ePlanePos::Near)], Near);
    EXPECT_EQ(f1.Planes[static_cast<int32_t>(Frustum::ePlanePos::Far)], far);
}

TEST_F(FrustumTest, OperatorAssign_AnotherFrustum_CopiesAllPlanes)
{
    defaultFrustum = f1;

    for (auto pos : allPlanePos)
    {
        EXPECT_TRUE(
            defaultFrustum.Planes[static_cast<int32_t>(pos)].IsEqualApprox(f1.Planes[static_cast<int32_t>(pos)]));
    }
    EXPECT_EQ(defaultFrustum, f1);
}

TEST_F(FrustumTest, OperatorComparison_IdenticalAndDifferentFrustums_ReturnsCorrectBoolean)
{
    Frustum f2 = f1;

    EXPECT_TRUE(f1 == f2);
    EXPECT_FALSE(f1 != f2);
}

TEST_F(FrustumTest, IsEqualApprox_VaryingPlaneOffsets_ReturnsCorrectBoolean)
{
    Plane far2(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.000001f));
    Frustum f2(left, right, bottom, top, Near, far2);

    EXPECT_TRUE(f1.IsEqualApprox(f2));
    Plane far3(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 5.0f));
    Frustum f3(left, right, bottom, top, Near, far3);

    EXPECT_FALSE(f1.IsEqualApprox(f3));
    EXPECT_TRUE(f1.IsNotEqualApprox(f3));
}

TEST_F(FrustumTest, GetPointSide_InsideAndOutsidePoints_ReturnsCorrectSide)
{
    Vector3 inside(0.0f, 0.0f, 0.0f);
    Vector3 outsideX(2.0f, 0.0f, 0.0f);
    Vector3 outsideY(0.0f, 2.0f, 0.0f);
    Vector3 outsideZ(0.0f, 0.0f, 2.0f);

    EXPECT_EQ(f1.GetPointSide(inside), math::eSide::Inside);
    EXPECT_EQ(f1.GetPointSide(outsideX), math::eSide::Outside);
    EXPECT_EQ(f1.GetPointSide(outsideY), math::eSide::Outside);
    EXPECT_EQ(f1.GetPointSide(outsideZ), math::eSide::Outside);
}

TEST_F(FrustumTest, GetLineSide_InsideOutsideAndIntersectingLines_ReturnsCorrectSide)
{
    Vector3 inside1(0.0f, 0.0f, 0.0f);
    Vector3 inside2(0.5f, -0.2f, 0.7f);
    Vector3 outsidePosZ(0.0f, 0.0f, 2.0f);
    Vector3 outsidePosX(2.0f, 0.0f, 0.0f);

    EXPECT_EQ(f1.GetLineSide(inside1, inside2), math::eSide::Inside);

    EXPECT_EQ(f1.GetLineSide(inside1, outsidePosZ), math::eSide::Intersect);
    EXPECT_EQ(f1.GetLineSide(inside2, outsidePosX), math::eSide::Intersect);

    EXPECT_EQ(f1.GetLineSide(outsidePosZ, Vector3(0.0f, 0.0f, 3.0f)), math::eSide::Outside);
    EXPECT_EQ(f1.GetLineSide(outsidePosX, Vector3(3.0f, 0.0f, 0.0f)), math::eSide::Outside);
}

TEST_F(FrustumTest, GetTriangleSide_InsideOutsideAndIntersectingTriangles_ReturnsCorrectSide)
{
    Vector3 inA(0.0f, 0.0f, 0.0f);
    Vector3 inB(0.2f, 0.3f, -0.7f);
    Vector3 inC(-0.8f, 0.1f, 0.9f);

    Vector3 outZ1(0.0f, 0.0f, 2.0f);
    Vector3 outZ2(0.5f, 0.2f, 3.0f);
    Vector3 outZ3(-0.4f, -0.3f, 2.5f);

    Vector3 mixOutX(2.0f, 0.0f, 0.0f);

    EXPECT_EQ(f1.GetTriangleSide(inA, inB, inC), math::eSide::Inside);
    EXPECT_EQ(f1.GetTriangleSide(inA, inB, mixOutX), math::eSide::Intersect);
    EXPECT_EQ(f1.GetTriangleSide(inA, outZ1, inC), math::eSide::Intersect);
    EXPECT_EQ(f1.GetTriangleSide(outZ1, outZ2, outZ3), math::eSide::Outside);
}

TEST_F(FrustumTest, GetAabbSide_InsideOutsideAndIntersectingBoxes_ReturnsCorrectSide)
{
    AABB inside(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
    EXPECT_EQ(f1.GetAabbSide(inside), math::eSide::Inside);

    AABB intersect(Vector3(0.5f, -0.5f, -0.5f), Vector3(1.5f, 0.5f, 0.5f));
    EXPECT_EQ(f1.GetAabbSide(intersect), math::eSide::Intersect);

    AABB outside(Vector3(2.0f, 2.0f, 2.0f), Vector3(3.0f, 3.0f, 3.0f));
    EXPECT_EQ(f1.GetAabbSide(outside), math::eSide::Outside);
}

TEST_F(FrustumTest, GetSphereSide_InsideOutsideAndIntersectingSpheres_ReturnsCorrectSide)
{
    Sphere inside(Vector3(0.0f, 0.0f, 0.0f), 0.5f);
    EXPECT_EQ(f1.GetSphereSide(inside), math::eSide::Inside);

    Sphere intersect(Vector3(1.2f, 0.0f, 0.0f), 0.5f);
    EXPECT_EQ(f1.GetSphereSide(intersect), math::eSide::Intersect);

    Sphere outside(Vector3(2.0f, 0.0f, 0.0f), 0.2f);
    EXPECT_EQ(f1.GetSphereSide(outside), math::eSide::Outside);
}

TEST_F(FrustumTest, FromMatrix4x4_IdentityViewProjection_ExtractsStandardUnitFrustum)
{
    Matrix4x4 mat = Matrix4x4::sIdentity;
    Frustum frustum = Frustum::FromMatrix4x4(mat);

    for (auto pos : allPlanePos)
    {
        const Plane& plane = frustum.Planes[static_cast<int32_t>(pos)];
        const Vector3& n = plane.Normal;
        EXPECT_NEAR(n.Magnitude(), 1.0f, math::EPSILON_CMP);
    }

    EXPECT_FALSE(frustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Left)].IsEqualApprox(
        frustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Right)]));
    EXPECT_FALSE(frustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Top)].IsEqualApprox(
        frustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Bottom)]));
    EXPECT_FALSE(frustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Near)].IsEqualApprox(
        frustum.Planes[static_cast<int32_t>(Frustum::ePlanePos::Far)]));

    EXPECT_TRUE(f1.IsEqualApprox(frustum));
}
