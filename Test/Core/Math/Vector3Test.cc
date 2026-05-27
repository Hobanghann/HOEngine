#include "Core/Math/Vector3.h"

#include <gtest/gtest.h>
#include <limits>

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

using namespace ho;

class Vector3Test : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        zero = Vector3();
        unitX = Vector3(1.0f, 0.0f, 0.0f);
        unitY = Vector3(0.0f, 1.0f, 0.0f);
        unitZ = Vector3(0.0f, 0.0f, 1.0f);
        vec123 = Vector3(1.0f, 2.0f, 3.0f);
        vec435 = Vector3(3.0f, 4.0f, 5.0f);
        vec123Approx = Vector3(1.000001f, 2.000001f, 3.000001f);
        vec010203 = Vector3(0.1f, 0.2f, 0.3f);
    }

  protected:
    Vector3 zero, unitX, unitY, unitZ;
    Vector3 vec123, vec123Approx, vec435, vec010203;
};

TEST_F(Vector3Test, Constructor_DefaultAndAllParameters_InitializesCorrectly)
{
    EXPECT_EQ(zero.X, 0.0f);
    EXPECT_EQ(zero.Y, 0.0f);
    EXPECT_EQ(zero.Z, 0.0f);

    EXPECT_EQ(vec123.X, 1.0f);
    EXPECT_EQ(vec123.Y, 2.0f);
    EXPECT_EQ(vec123.Z, 3.0f);

    Vector2 v2(1.0f, 2.0f);
    Vector3 from2(v2);
    EXPECT_EQ(from2.X, 1.0f);
    EXPECT_EQ(from2.Y, 2.0f);
    EXPECT_EQ(from2.Z, 0.0f);

    Vector4 v4(1.0f, 2.0f, 3.0f, 4.0f);
    Vector3 from4(v4);
    EXPECT_EQ(from4.X, 1.0f);
    EXPECT_EQ(from4.Y, 2.0f);
    EXPECT_EQ(from4.Z, 3.0f);
}

TEST_F(Vector3Test, StaticConstants_PredefinedVectors_MatchesExpectedValues)
{
    EXPECT_EQ(Vector3::sUnitX, Vector3(1.0f, 0.0f, 0.0f));
    EXPECT_EQ(Vector3::sUnitY, Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_EQ(Vector3::sUnitZ, Vector3(0.0f, 0.0f, 1.0f));
    EXPECT_EQ(Vector3::sZero, Vector3(0.0f, 0.0f, 0.0f));
}

TEST_F(Vector3Test, OperatorAssign_AnotherVector_CopiesValues)
{
    zero = unitX;
    EXPECT_EQ(zero, unitX);
}

TEST_F(Vector3Test, OperatorAdd_TwoVectors_ReturnsSum)
{
    EXPECT_EQ(vec123 + vec435, Vector3(4.0f, 6.0f, 8.0f));
    vec123 += vec435;
    EXPECT_EQ(vec123, Vector3(4.0f, 6.0f, 8.0f));
}

TEST_F(Vector3Test, OperatorSub_TwoVectors_ReturnsDifference)
{
    EXPECT_EQ(vec435 - vec123, Vector3(2.0f, 2.0f, 2.0f));
    vec435 -= vec123;
    EXPECT_EQ(vec435, Vector3(2.0f, 2.0f, 2.0f));
}

TEST_F(Vector3Test, OperatorMul_TwoVectors_ReturnsComponentWiseProduct)
{
    EXPECT_EQ(vec123 * vec435, Vector3(3.0f, 8.0f, 15.0f));
    vec123 *= vec435;
    EXPECT_EQ(vec123, Vector3(3.0f, 8.0f, 15.0f));
}

TEST_F(Vector3Test, OperatorDiv_TwoVectors_ReturnsComponentWiseQuotient)
{
    EXPECT_EQ(vec435 / vec123, Vector3(3.0f, 2.0f, 5.0f / 3.0f));
    vec435 /= vec123;
    EXPECT_EQ(vec435, Vector3(3.0f, 2.0f, 5.0f / 3.0f));

    Vector3 v(1.0f, 2.0f, 3.0f);
    v /= zero;
    EXPECT_TRUE(std::isinf(v.X));
    EXPECT_TRUE(std::isinf(v.Y));
    EXPECT_TRUE(std::isinf(v.Z));
}

TEST_F(Vector3Test, OperatorMul_ScalarValue_ScalesVector)
{
    EXPECT_EQ(vec123 * 2.0f, Vector3(2.0f, 4.0f, 6.0f));
    EXPECT_EQ(2.0f * vec123, Vector3(2.0f, 4.0f, 6.0f));
    vec123 *= 2.0f;
    EXPECT_EQ(vec123, Vector3(2.0f, 4.0f, 6.0f));
}

TEST_F(Vector3Test, OperatorDiv_ScalarValue_ScalesVector)
{
    EXPECT_EQ(vec435 / 2.0f, Vector3(1.5f, 2.0f, 2.5f));

    vec435 /= 2.0f;
    EXPECT_EQ(vec435, Vector3(1.5f, 2.0f, 2.5f));

    vec435 /= 0.0f;
    EXPECT_TRUE(std::isinf(vec435.X));
    EXPECT_TRUE(std::isinf(vec435.Y));
    EXPECT_TRUE(std::isinf(vec435.Z));
}

TEST_F(Vector3Test, OperatorUnaryMinus_NegativeOne_NegatesComponents)
{
    EXPECT_EQ(-vec123, Vector3(-1.0f, -2.0f, -3.0f));
}

TEST_F(Vector3Test, OperatorComparison_DifferentVectors_ReturnsCorrectBoolean)
{
    Vector3 vecEqual(1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(vec123 == vecEqual);
    EXPECT_FALSE(vec123 != vecEqual);
    EXPECT_TRUE(vec123 != vec435);

    EXPECT_TRUE(vec123 < vec435);
    EXPECT_TRUE(vec123 <= vec435);
    EXPECT_TRUE(vec123 > unitX);
    EXPECT_TRUE(vec123 >= unitX);
}

TEST_F(Vector3Test, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    EXPECT_TRUE(vec123.IsEqualApprox(vec123Approx));
    EXPECT_FALSE(vec123.IsNotEqualApprox(vec123Approx));
}

TEST_F(Vector3Test, IsFinite_VectorHasInfComponent_ReturnsFalse)
{
    Vector3 v(1.0f, std::numeric_limits<float>::infinity(), 3.0f);
    EXPECT_FALSE(v.IsFinite());

    Vector3 u(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(u.IsFinite());
}

TEST_F(Vector3Test, IsUnit_UnitAndNonUnitVectors_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(unitX.IsUnit());
    EXPECT_TRUE(unitX.IsUnitApprox());

    Vector3 nearUnit(0.9999f, 0.0f, 0.0f);
    EXPECT_FALSE(nearUnit.IsUnit());
    EXPECT_TRUE(nearUnit.IsUnitApprox());
}

TEST_F(Vector3Test, Dot_TwoVectors_ReturnsCorrectScalar)
{
    EXPECT_NEAR(zero.Dot(unitX), 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(vec123.Dot(vec435), 26.0f, math::EPSILON_CMP); // 1*3 + 2*4 + 3*5
}

TEST_F(Vector3Test, Cross_TwoVectors_ReturnsCorrectVector)
{
    EXPECT_TRUE(zero.Cross(vec123).IsEqualApprox(Vector3(0.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(unitX.Cross(unitX).IsEqualApprox(Vector3(0.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(unitX.Cross(-unitX).IsEqualApprox(Vector3(0.0f, 0.0f, 0.0f)));
    Vector3 c = vec123.Cross(vec435);
    EXPECT_TRUE(c.IsEqualApprox(Vector3(-2.0f, 4.0f, -2.0f)));
    EXPECT_TRUE(unitX.Cross(unitY).IsEqualApprox(Vector3(0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(unitY.Cross(unitX).IsEqualApprox(Vector3(0.0f, 0.0f, -1.0f)));
}

TEST_F(Vector3Test, Magnitude_TypicalVector_ReturnsLength)
{
    EXPECT_NEAR(vec435.Magnitude(), std::sqrt(50.0f), math::EPSILON_CMP);
    EXPECT_NEAR(vec435.SqrdMagnitude(), 50.0f, math::EPSILON_CMP);
}

TEST_F(Vector3Test, Normalize_NonZeroVector_MakeVectorToUnit)
{
    Vector3 n = vec435.Normalized();
    EXPECT_NEAR(n.Magnitude(), 1.0f, math::EPSILON_CMP);

    vec435.Normalize();
    EXPECT_NEAR(vec435.Magnitude(), 1.0f, math::EPSILON_CMP);

    vec010203.Normalize();
    EXPECT_NEAR(vec010203.Magnitude(), 1.0f, math::EPSILON_CMP);
}

TEST_F(Vector3Test, Normalize_ZeroVector_ResultVectorComponentsAreNaN)
{
    Vector3 n = zero.Normalized();
    EXPECT_TRUE(std::isnan(n.X));
    EXPECT_TRUE(std::isnan(n.Y));
    EXPECT_TRUE(std::isnan(n.Z));

    zero.Normalize();
    EXPECT_TRUE(std::isnan(zero.X));
    EXPECT_TRUE(std::isnan(zero.Y));
    EXPECT_TRUE(std::isnan(zero.Z));
}

TEST_F(Vector3Test, Projected_ValidVectors_ReturnsProjectedVector)
{
    Vector3 onto(2.0f, 5.0f, 1.0f);
    float dot = vec435.Dot(onto);
    float denom = onto.SqrdMagnitude();
    Vector3 expected = (dot / denom) * onto;

    Vector3 projected = vec435.Projected(onto);
    EXPECT_TRUE(projected.IsEqualApprox(expected));

    vec435.Project(onto);
    EXPECT_TRUE(vec435.IsEqualApprox(expected));
}

TEST_F(Vector3Test, Projected_ParallelAndPerpendicularCases_ReturnsExpectedVector)
{
    Vector3 a = 2.0f * vec123;
    Vector3 p = a.Projected(vec123);
    EXPECT_TRUE(p.IsEqualApprox(a));

    Vector3 proj = unitX.Projected(unitY);
    EXPECT_TRUE(proj.IsEqualApprox(Vector3::sZero));
}

TEST_F(Vector3Test, Projected_DifferentLengthOntoVectors_ReturnsSameResult)
{
    Vector3 onto1(0.0f, 1.0f, 0.0f);
    Vector3 onto2(0.0f, 10.0f, 0.0f);

    Vector3 p1 = vec435.Projected(onto1);
    Vector3 p2 = vec435.Projected(onto2);

    EXPECT_TRUE(p1.IsEqualApprox(p2));
}

TEST_F(Vector3Test, ToCartesian_Vector3_ReturnsVector2WithDividedByZ)
{
    Vector2 c = vec123.ToCartesian();
    EXPECT_TRUE(c.IsEqualApprox(Vector2(1.0f / 3.0f, 2.0f / 3.0f)));

    Vector3 v = Vector3(3.0f, 4.0f, 0.0f);
    c = v.ToCartesian();
    EXPECT_TRUE(std::isinf(c.X));
    EXPECT_TRUE(std::isinf(c.Y));
}

TEST_F(Vector3Test, ToHomogeneous_Vector3_ReturnsVector4WithOne)
{
    EXPECT_EQ(vec123.ToHomogeneous(), Vector4(1.0f, 2.0f, 3.0f, 1.0f));
}
