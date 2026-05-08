#include "Core/Math/Vector2.h"

#include <gtest/gtest.h>

#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"

using namespace ho;

class Vector2Test : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        zero = Vector2();
        unitX = Vector2(1.0f, 0.0f);
        unitXApprox = Vector2(0.99999f, 0.0f);
        unitY = Vector2(0.0f, 1.0f);
        vec12 = Vector2(1.0f, 2.0f);
        vec12Approx = Vector2(1.0000001f, 2.0000001f);
        vec34 = Vector2(3.0f, 4.0f);
        vec0102 = Vector2(0.1f, 0.2f);
    }

  protected:
    Vector2 zero, unitX, unitXApprox, unitY;
    Vector2 vec12, vec12Approx, vec34, vec0102;
};

TEST_F(Vector2Test, Constructor_DefaultAndAllParameters_InitializesCorrectly)
{
    EXPECT_EQ(zero.X, 0.0f);
    EXPECT_EQ(zero.Y, 0.0f);

    EXPECT_EQ(vec12.X, 1.0f);
    EXPECT_EQ(vec12.Y, 2.0f);

    Vector3 vec123(1.0f, 2.0f, 3.0f);
    Vector2 vec2FromVec3(vec123);
    EXPECT_EQ(vec2FromVec3.X, 1.0f);
    EXPECT_EQ(vec2FromVec3.Y, 2.0f);

    Vector4 vec1234(1.0f, 2.0f, 3.0f, 4.0f);
    Vector2 vec2FromVec4(vec1234);
    EXPECT_EQ(vec2FromVec4.X, 1.0f);
    EXPECT_EQ(vec2FromVec4.Y, 2.0f);
}

TEST_F(Vector2Test, StaticConstants_PredefinedVectors_MatchesExpectedValues)
{
    EXPECT_EQ(Vector2::sUnitX, Vector2(1.0f, 0.0f));
    EXPECT_EQ(Vector2::sUnitY, Vector2(0.0f, 1.0f));
    EXPECT_EQ(Vector2::sZero, Vector2(0.0f, 0.0f));
}

TEST_F(Vector2Test, OperatorAssign_AnotherVector_CopiesValues)
{
    zero = unitX;
    EXPECT_EQ(zero, unitX);
}

TEST_F(Vector2Test, OperatorAdd_TwoVectors_ReturnsSum)
{
    EXPECT_EQ(vec12 + vec34, Vector2(4.0f, 6.0f));

    vec12 += vec34;
    EXPECT_EQ(vec12, Vector2(4.0f, 6.0f));
}

TEST_F(Vector2Test, OperatorSub_TwoVectors_ReturnsDifference)
{
    EXPECT_EQ(vec34 - vec12, Vector2(2.0f, 2.0f));

    vec34 -= vec12;
    EXPECT_EQ(vec34, Vector2(2.0f, 2.0f));
}

TEST_F(Vector2Test, OperatorMul_TwoVectors_ReturnsComponentWiseProduct)
{
    EXPECT_EQ(vec12 * vec34, Vector2(3.0f, 8.0f));

    vec12 *= vec34;
    EXPECT_EQ(vec12, Vector2(3.0f, 8.0f));
}

TEST_F(Vector2Test, OperatorDiv_TwoVectors_ReturnsComponentWiseQuotient)
{
    EXPECT_TRUE((vec34 / vec12).IsEqualApprox(Vector2(3.0f, 2.0f)));

    vec34 /= vec12;
    EXPECT_TRUE(vec34.IsEqualApprox(Vector2(3.0f, 2.0f)));

    vec34 /= zero;
    EXPECT_TRUE(std::isinf(vec34.X));
    EXPECT_TRUE(std::isinf(vec34.Y));
}

TEST_F(Vector2Test, OperatorMul_ScalarValue_ScalesVector)
{
    EXPECT_EQ(vec12 * 2.0f, Vector2(2.0f, 4.0f));
    EXPECT_EQ(2.0f * vec12, Vector2(2.0f, 4.0f));

    vec12 *= 2.0f;
    EXPECT_EQ(vec12, Vector2(2.0f, 4.0f));
}

TEST_F(Vector2Test, OperatorDiv_ScalarValue_ScalesVector)
{
    EXPECT_EQ(vec34 / 2.0f, Vector2(1.5f, 2.0f));

    vec34 /= 2.0f;
    EXPECT_EQ(vec34, Vector2(1.5f, 2.0f));

    vec34 /= 0.0f;
    EXPECT_TRUE(std::isinf(vec34.X));
    EXPECT_TRUE(std::isinf(vec34.Y));
}

TEST_F(Vector2Test, OperatorUnaryMinus_NegativeOne_NegatesComponents)
{
    EXPECT_EQ(-vec12, Vector2(-1.0f, -2.0f));
}

TEST_F(Vector2Test, OperatorComparison_DifferentVectors_ReturnsCorrectBoolean)
{
    Vector2 v2e(1.0f, 2.0f);

    EXPECT_TRUE(vec12 == v2e);
    EXPECT_FALSE(vec12 != v2e);
    EXPECT_TRUE(vec12 != vec34);

    EXPECT_TRUE(vec12 < vec34);
    EXPECT_TRUE(vec12 <= vec34);
    EXPECT_TRUE(vec12 > unitX);
    EXPECT_TRUE(vec12 >= unitX);
}

TEST_F(Vector2Test, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    EXPECT_TRUE(vec12.IsEqualApprox(vec12Approx));
    EXPECT_FALSE(vec12.IsNotEqualApprox(vec12Approx));
}

TEST_F(Vector2Test, IsFinite_VectorHasInfComponent_ReturnsFalse)
{
    Vector2 v(1.0f, std::numeric_limits<float>::infinity());
    EXPECT_FALSE(v.IsFinite());

    Vector2 u(1.0f, 2.0f);
    EXPECT_TRUE(u.IsFinite());
}

TEST_F(Vector2Test, IsUnit_UnitAndNonUnitVectors_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(unitX.IsUnit());
    EXPECT_TRUE(unitX.IsUnitApprox());

    EXPECT_FALSE(unitXApprox.IsUnit());
    EXPECT_TRUE(unitXApprox.IsUnitApprox());
}

TEST_F(Vector2Test, Dot_TwoVectors_ReturnsCorrectScalar)
{
    EXPECT_NEAR(zero.Dot(unitX), 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(vec12.Dot(vec34), 11.0f, math::EPSILON_CMP);
}

TEST_F(Vector2Test, Cross_TwoVectors_ReturnsCorrectZComponent)
{
    EXPECT_NEAR(zero.Cross(unitX), 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(unitX.Cross(unitX), 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(-unitX.Cross(unitX), 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(vec12.Cross(vec34), -2.0f, math::EPSILON_CMP);
}

TEST_F(Vector2Test, Magnitude_TypicalVector_ReturnsLength)
{
    EXPECT_NEAR(vec34.Magnitude(), 5.0f, math::EPSILON_CMP);
    EXPECT_NEAR(vec34.SqrdMagnitude(), 25.0f, math::EPSILON_CMP);
}

TEST_F(Vector2Test, Normalize_NonZeroVector_MakeVectorToUnit)
{
    Vector2 n = vec34.Normalized();
    EXPECT_NEAR(n.Magnitude(), 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(n.X, 0.6f, math::EPSILON_CMP);
    EXPECT_NEAR(n.Y, 0.8f, math::EPSILON_CMP);

    vec34.Normalize();
    EXPECT_NEAR(vec34.Magnitude(), 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(vec34.X, 0.6f, math::EPSILON_CMP);
    EXPECT_NEAR(vec34.Y, 0.8f, math::EPSILON_CMP);

    vec0102.Normalize();
    EXPECT_NEAR(vec0102.Magnitude(), 1.0f, math::EPSILON_CMP);
}

TEST_F(Vector2Test, Normalize_ZeroVector_ResultVectorComponentsAreNaN)
{
    Vector2 n = zero.Normalized();

    EXPECT_TRUE(std::isnan(n.X));
    EXPECT_TRUE(std::isnan(n.Y));

    zero.Normalize();
    EXPECT_TRUE(std::isnan(zero.X));
    EXPECT_TRUE(std::isnan(zero.Y));
}

TEST_F(Vector2Test, Projected_ValidVectors_ReturnsProjectedVector)
{
    Vector2 onto(2.0f, 5.0f);

    float dot = vec34.Dot(onto);
    float denom = onto.SqrdMagnitude();
    Vector2 expected = (dot / denom) * onto;

    Vector2 projected = vec34.Projected(onto);
    EXPECT_TRUE(projected.IsEqualApprox(expected));

    vec34.Project(onto);
    EXPECT_TRUE(vec34.IsEqualApprox(expected));
}

TEST_F(Vector2Test, Projected_ParallelAndPerpendicularCases_ReturnsExpectedVector)
{
    Vector2 a = 2.0f * vec12;
    Vector2 p = a.Projected(vec12);
    EXPECT_TRUE(p.IsEqualApprox(a));

    Vector2 proj = unitX.Projected(unitY);
    EXPECT_TRUE(proj.IsEqualApprox(Vector2::sZero));
}

TEST_F(Vector2Test, Projected_DifferentLengthOntoVectors_ReturnsSameResult)
{
    Vector2 onto1(0.0f, 1.0f);
    Vector2 onto2(0.0f, 10.0f);

    Vector2 p1 = vec34.Projected(onto1);
    Vector2 p2 = vec34.Projected(onto2);

    EXPECT_TRUE(p1.IsEqualApprox(p2));
}

TEST_F(Vector2Test, ToHomogeneous_Vector2_ReturnsVector3WithOne)
{
    EXPECT_EQ(vec12.ToHomogeneous(), Vector3(1.0f, 2.0f, 1.0f));
}
