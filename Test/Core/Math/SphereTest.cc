#include "Core/Math/Sphere.h"

#include <gtest/gtest.h>

#include "Core/Math/Vector3.h"

using namespace ho;

TEST(SphereTest, Constructor_DefaultAndAllParameters_InitializesCorrectly)
{
    Sphere s1;
    EXPECT_EQ(s1.Center, Vector3::sZero);
    EXPECT_FLOAT_EQ(s1.Radius, 0.0f);

    Sphere s2(Vector3(1.0f, 2.0f, 3.0f), 5.0f);
    EXPECT_EQ(s2.Center, Vector3(1.0f, 2.0f, 3.0f));
    EXPECT_FLOAT_EQ(s2.Radius, 5.0f);
}

TEST(SphereTest, OperatorAssign_AnotherSphere_CopiesValuesCorrectly)
{
    Sphere a(Vector3(1.0f, 2.0f, 3.0f), 5.0f);
    Sphere b;
    b = a;
    EXPECT_EQ(b.Center, Vector3(1.0f, 2.0f, 3.0f));
    EXPECT_FLOAT_EQ(b.Radius, 5.0f);
}

TEST(SphereTest, OperatorComparison_IdenticalAndDifferentSpheres_ReturnsCorrectBoolean)
{
    Sphere s1(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    Sphere s2(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    Sphere s3(Vector3(0.1f, 0.0f, 0.0f), 1.0f);

    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 != s2);

    EXPECT_FALSE(s1 == s3);
    EXPECT_TRUE(s1 != s3);
}

TEST(SphereTest, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Sphere a(Vector3(1.0f, 1.0f, 1.0f), 2.0f);
    Sphere b(Vector3(1.0000001f, 1.0000001f, 1.0000001f), 2.0000001f);
    Sphere c(Vector3(2.0f, 2.0f, 2.0f), 3.0f);

    EXPECT_TRUE(a.IsEqualApprox(b));
    EXPECT_FALSE(a.IsNotEqualApprox(b));

    EXPECT_FALSE(a.IsEqualApprox(c));
    EXPECT_TRUE(a.IsNotEqualApprox(c));
}

TEST(SphereTest, Merge_TwoSeparatedSpheres_EncapsulatesBothCorrectly)
{
    Sphere s1(Vector3(-2.0f, 0.0f, 0.0f), 1.0f);
    Sphere s2(Vector3(2.0f, 0.0f, 0.0f), 1.0f);

    Sphere merged = Sphere::Merge(s1, s2);

    EXPECT_NEAR(merged.Center.X, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(merged.Center.Y, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(merged.Center.Z, 0.0f, math::EPSILON_CMP);

    EXPECT_NEAR(merged.Radius, 3.0f, math::EPSILON_CMP);
}

TEST(SphereTest, Merge_TwoOverlappingSpheres_EncapsulatesBothCorrectly)
{
    Sphere s1(Vector3(0.0f, 0.0f, 0.0f), 3.0f);
    Sphere s2(Vector3(4.0f, 0.0f, 0.0f), 3.0f);

    Sphere merged = Sphere::Merge(s1, s2);
    EXPECT_NEAR(merged.Center.X, 2.0f, math::EPSILON_CMP);
    EXPECT_NEAR(merged.Radius, 5.0f, math::EPSILON_CMP);
}

TEST(SphereTest, Merge_BigAndSmallContainedSpheres_ReturnsLargerSphere)
{
    Sphere s_big(Vector3(0.0f, 0.0f, 0.0f), 5.0f);
    Sphere s_small(Vector3(1.0f, 0.0f, 0.0f), 1.0f);

    Sphere merged = Sphere::Merge(s_big, s_small);

    EXPECT_TRUE(merged.IsEqualApprox(s_big));
}

TEST(SphereTest, Merge_IdenticalSpheres_ReturnsSameSphere)
{
    Sphere s(Vector3(0.0f, 0.0f, 0.0f), 2.0f);
    Sphere merged = Sphere::Merge(s, s);

    EXPECT_TRUE(merged.IsEqualApprox(s));
}

TEST(SphereTest, FromPositionsCentroid_PointArray_GeneratesAverageCenterSphere)
{
    std::vector<Vector3> points = {
        Vector3(-1.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)};
    Sphere sphere = Sphere::FromPositionsCentroid(points.data(), points.size());

    EXPECT_NEAR(sphere.Center.X, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Center.Y, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Center.Z, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Radius, 1.0f, math::EPSILON_CMP);

    points = {Vector3(1.0f, 1.0f, 1.0f)};
    sphere = Sphere::FromPositionsCentroid(points.data(), points.size());
    EXPECT_NEAR(sphere.Center.X, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Center.Y, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Center.Z, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Radius, 0.0f, math::EPSILON_CMP);
}

TEST(SphereTest, FromPositionsRitter_PointArray_GeneratesApproximatedTightSphere)
{
    std::vector<Vector3> points = {
        Vector3(-1.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f)};
    Sphere sphere = Sphere::FromPositionsRitter(points.data(), points.size());

    EXPECT_NEAR(sphere.Center.X, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Center.Y, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Center.Z, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Radius, 1.0f, math::EPSILON_CMP);

    points = {Vector3(1.0f, 1.0f, 1.0f)};
    sphere = Sphere::FromPositionsRitter(points.data(), points.size());
    EXPECT_NEAR(sphere.Center.X, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Center.Y, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Center.Z, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(sphere.Radius, 0.0f, math::EPSILON_CMP);
}