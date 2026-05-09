#include "Core/Math/Projection.h"

#include <gtest/gtest.h>

#include "Core/Math/MathFuncs.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector4.h"

using namespace ho;

class ProjectionTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        fov = math::DegToRad(60.0f);
        width = 1920.0f;
        height = 1080.0f;
        aspect = width / height;
        nearZ = 0.1f;
        farZ = 100.0f;
    }

    float fov, width, height, aspect, nearZ, farZ;
    Projection proj;
};

TEST_F(ProjectionTest, CreateOrthographic_StandardDimensions_CalculatesCorrectMatrix)
{
    aspect = width / height;

    proj = Projection::CreateOrthographic(width, height, nearZ, farZ);

    float invRange = 1.0f / (farZ - nearZ);

    Matrix4x4 expectedMatrix({2.0f / width, 0.0f, 0.0f, 0.0f},
                             {0.0f, 2.0f / height, 0.0f, 0.0f},
                             {0.0f, 0.0f, -2.0f * invRange, 0.0f},
                             {0.0f, 0.0f, -(farZ + nearZ) * invRange, 1.0f});

    EXPECT_TRUE(proj.GetMatrix().IsEqualApprox(expectedMatrix));
    EXPECT_NEAR(proj.GetFOV(), 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetAspectRatio(), aspect, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetNearDistance(), nearZ, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetFarDistance(), farZ, math::EPSILON_CMP);

    Projection projFromMat = Projection::CreateOrthographic(expectedMatrix);
    EXPECT_TRUE(proj.IsEqualApprox(projFromMat));
}

TEST_F(ProjectionTest, CreatePerspective_StandardFovAndDimensions_CalculatesCorrectMatrix)
{
    proj = Projection::CreatePerspective(fov, width, height, nearZ, farZ);

    float tanHalfFOV = math::Tan(fov * 0.5f);
    float invNearMinusFar = 1.0f / (nearZ - farZ);

    Matrix4x4 expectedMatrix({1.0f / (aspect * tanHalfFOV), 0.0f, 0.0f, 0.0f},
                             {0.0f, 1.0f / tanHalfFOV, 0.0f, 0.0f},
                             {0.0f, 0.0f, (nearZ + farZ) * invNearMinusFar, -1.0f},
                             {0.0f, 0.0f, (2.0f * nearZ * farZ) * invNearMinusFar, 0.0f});

    EXPECT_TRUE(proj.GetMatrix().IsEqualApprox(expectedMatrix));
    EXPECT_NEAR(proj.GetFOV(), fov, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetAspectRatio(), aspect, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetNearDistance(), nearZ, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetFarDistance(), farZ, math::EPSILON_CMP);

    Projection projFromMat = Projection::CreatePerspective(expectedMatrix);
    EXPECT_TRUE(proj.IsEqualApprox(projFromMat));
}

TEST_F(ProjectionTest, OperatorAssign_OrthographicProjection_CopiesStateAccurately)
{
    Projection dst = Projection::CreateOrthographic(100.0f, 100.0f, 0.0f, 100.0f);

    Projection srcSame = Projection::CreateOrthographic(200.0f, 200.0f, 10.0f, 50.0f);

    Projection srcDiff = Projection::CreatePerspective(math::DegToRad(60.0f), 1920.0f, 1080.0f, 0.1f, 1000.0f);

    dst = srcSame;

    EXPECT_EQ(dst, srcSame);

    dst = srcDiff;

    EXPECT_EQ(dst, srcDiff);
}

TEST_F(ProjectionTest, OperatorAssign_PerspectiveProjection_CopiesStateAccurately)
{
    Projection dst = Projection::CreatePerspective(math::DegToRad(45.0f), 800.0f, 600.0f, 0.1f, 100.0f);

    Projection srcSame = Projection::CreatePerspective(math::DegToRad(90.0f), 1920.0f, 1080.0f, 1.0f, 500.0f);

    Projection srcDiff = Projection::CreateOrthographic(100.0f, 100.0f, 0.0f, 100.0f);

    dst = srcSame;

    EXPECT_EQ(dst, srcSame);

    dst = srcDiff;

    EXPECT_EQ(dst, srcDiff);
}

TEST_F(ProjectionTest, Setters_OrthographicProjection_UpdatesSpecificMatrixElements)
{
    proj = Projection::CreateOrthographic(width, height, nearZ, farZ);

    EXPECT_NEAR(proj.GetAspectRatio(), aspect, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetNearDistance(), nearZ, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetFarDistance(), farZ, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetFOV(), 0.0f, math::EPSILON_CMP);

    float originalProjM11 = proj.GetMatrix().Data[1][1];

    proj.SetAspectRatio(200.0f, 100.0f);

    EXPECT_NEAR(proj.GetAspectRatio(), 2.0f, math::EPSILON_CMP);
    EXPECT_EQ(proj.GetMatrix().Data[1][1], originalProjM11);
    EXPECT_NEAR(proj.GetMatrix().Data[0][0], originalProjM11 / 2.0f, math::EPSILON_CMP);

    float newNear = 10.0f;
    float newFar = 50.0f;
    proj.SetNearDistance(newNear);
    proj.SetFarDistance(newFar);

    EXPECT_NEAR(proj.GetNearDistance(), newNear, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetFarDistance(), newFar, math::EPSILON_CMP);

    float expectedM22 = -2.0f / (newFar - newNear);
    EXPECT_NEAR(proj.GetMatrix().Data[2][2], expectedM22, math::EPSILON_CMP);
}

TEST_F(ProjectionTest, Setters_PerspectiveProjection_UpdatesSpecificMatrixElements)
{
    proj = Projection::CreatePerspective(fov, width, height, nearZ, farZ);

    EXPECT_NEAR(proj.GetFOV(), fov, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetAspectRatio(), aspect, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetNearDistance(), nearZ, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetFarDistance(), farZ, math::EPSILON_CMP);

    float newFOV = math::DegToRad(90.0f);
    proj.SetFOV(newFOV);

    EXPECT_NEAR(proj.GetFOV(), newFOV, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetMatrix().Data[1][1], 1.0f / math::Tan(newFOV * 0.5f), math::EPSILON_CMP);

    float fixedM11 = proj.GetMatrix().Data[1][1];

    proj.SetAspectRatio(800.0f, 600.0f);

    EXPECT_NEAR(proj.GetAspectRatio(), 4.0f / 3.0f, math::EPSILON_CMP);
    EXPECT_EQ(proj.GetMatrix().Data[1][1], fixedM11);

    EXPECT_NEAR(proj.GetMatrix().Data[0][0], fixedM11 / (4.0f / 3.0f), math::EPSILON_CMP);

    float newNear = 1.0f;
    float newFar = 500.0f;
    proj.SetNearDistance(newNear);
    proj.SetFarDistance(newFar);

    EXPECT_NEAR(proj.GetNearDistance(), newNear, math::EPSILON_CMP);
    EXPECT_NEAR(proj.GetFarDistance(), newFar, math::EPSILON_CMP);

    float expectedM22 = (newNear + newFar) / (newNear - newFar);
    EXPECT_NEAR(proj.GetMatrix().Data[2][2], expectedM22, math::EPSILON_CMP);
}

TEST_F(ProjectionTest, OperatorComparison_DifferentOrthographicProjections_ReturnsCorrectBoolean)
{
    Projection p1 = Projection::CreateOrthographic(100.0f, 100.0f, 0.0f, 100.0f);

    Projection p2 = Projection::CreateOrthographic(100.0f, 100.0f, 0.0f, 100.0f);

    Projection p3 = Projection::CreateOrthographic(100.0f, 200.0f, 0.0f, 100.0f);

    Projection projPerspective = Projection::CreatePerspective(math::DegToRad(60.0f), 100.0f, 100.0f, 0.1f, 100.0f);

    EXPECT_TRUE(p1 == p2);
    EXPECT_FALSE(p1 != p2);

    EXPECT_FALSE(p1 == p3);
    EXPECT_TRUE(p1 != p3);

    EXPECT_FALSE(p1 == projPerspective);
    EXPECT_TRUE(p1 != projPerspective);
}

TEST_F(ProjectionTest, OperatorComparison_DifferentPerspectiveProjections_ReturnsCorrectBoolean)
{
    Projection p1 = Projection::CreatePerspective(fov, width, height, nearZ, farZ);

    Projection p2 = Projection::CreatePerspective(fov, width, height, nearZ, farZ);

    Projection p3 = Projection::CreatePerspective(math::DegToRad(90.0f), width, height, nearZ, farZ);

    Projection projOrtho = Projection::CreateOrthographic(width, height, nearZ, farZ);

    EXPECT_TRUE(p1 == p2);
    EXPECT_FALSE(p1 != p2);

    EXPECT_FALSE(p1 == p3);
    EXPECT_TRUE(p1 != p3);

    EXPECT_FALSE(p1 == projOrtho);
    EXPECT_TRUE(p1 != projOrtho);
}

TEST_F(ProjectionTest, IsEqualApprox_OrthographicWithinEpsilon_ReturnsCorrectBoolean)
{
    Projection p1 = Projection::CreateOrthographic(100.0f, 100.0f, 0.0f, 100.0f);

    Projection projApprox = Projection::CreateOrthographic(100.0f, 100.0f + math::EPSILON_CMP, 0.0f, 100.0f);

    EXPECT_TRUE(p1.IsEqualApprox(projApprox));
    EXPECT_FALSE(p1.IsNotEqualApprox(projApprox));

    Projection projDiff = Projection::CreateOrthographic(100.0f, 100.1f, 0.0f, 100.0f);

    EXPECT_FALSE(p1.IsEqualApprox(projDiff));
    EXPECT_TRUE(p1.IsNotEqualApprox(projDiff));

    Projection projPerspective = Projection::CreatePerspective(math::DegToRad(60.0f), 100.0f, 100.0f, 0.1f, 100.0f);

    EXPECT_FALSE(p1.IsEqualApprox(projPerspective));
}

TEST_F(ProjectionTest, IsEqualApprox_PerspectiveWithinEpsilon_ReturnsCorrectBoolean)
{
    Projection p1 = Projection::CreatePerspective(fov, width, height, nearZ, farZ);

    Projection projApprox = Projection::CreatePerspective(fov + math::EPSILON_CMP2, width, height, nearZ, farZ);

    EXPECT_TRUE(p1.IsEqualApprox(projApprox));
    EXPECT_FALSE(p1.IsNotEqualApprox(projApprox));

    Projection projDiff = Projection::CreatePerspective(fov + 0.1f, width, height, nearZ, farZ);

    EXPECT_FALSE(p1.IsEqualApprox(projDiff));
    EXPECT_TRUE(p1.IsNotEqualApprox(projDiff));

    Projection projOrtho = Projection::CreateOrthographic(width, height, nearZ, farZ);

    EXPECT_FALSE(p1.IsEqualApprox(projOrtho));
}