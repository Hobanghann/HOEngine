#include "Core/Math/Vector4.h"

#include <gtest/gtest.h>
#include <limits>

#include "Core/Math/Color128.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"

using namespace ho;

class Vector4Test : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        zero = Vector4();
        unitX = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
        unitY = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
        unitZ = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
        unitW = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
        vec1234 = Vector4(1.0f, 2.0f, 3.0f, 4.0f);
        vec3456 = Vector4(3.0f, 4.0f, 5.0f, 6.0f);
        vec1234Approx = Vector4(1.0000001f, 2.0000001f, 3.0000001f, 4.0000001f);
        vec01020304 = Vector4(0.1f, 0.2f, 0.3f, 0.4f);
    }

  protected:
    Vector4 zero, unitX, unitY, unitZ, unitW;
    Vector4 vec1234, vec3456, vec1234Approx, vec01020304;
};

TEST_F(Vector4Test, Constructor_DefaultAndAllParameters_InitializesCorrectly)
{
    EXPECT_EQ(zero.X, 0.0f);
    EXPECT_EQ(zero.Y, 0.0f);
    EXPECT_EQ(zero.Z, 0.0f);
    EXPECT_EQ(zero.W, 0.0f);

    EXPECT_EQ(vec1234.X, 1.0f);
    EXPECT_EQ(vec1234.Y, 2.0f);
    EXPECT_EQ(vec1234.Z, 3.0f);
    EXPECT_EQ(vec1234.W, 4.0f);

    Vector2 v2(1.0f, 2.0f);
    Vector4 from2(v2);
    EXPECT_EQ(from2.X, 1.0f);
    EXPECT_EQ(from2.Y, 2.0f);
    EXPECT_EQ(from2.Z, 0.0f);
    EXPECT_EQ(from2.W, 0.0f);

    Vector3 v3(1.0f, 2.0f, 3.0f);
    Vector4 from3(v3);
    EXPECT_EQ(from3.X, 1.0f);
    EXPECT_EQ(from3.Y, 2.0f);
    EXPECT_EQ(from3.Z, 3.0f);
    EXPECT_EQ(from3.W, 0.0f);

    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 fromQ(q);
    EXPECT_EQ(fromQ.X, 1.0f);
    EXPECT_EQ(fromQ.Y, 2.0f);
    EXPECT_EQ(fromQ.Z, 3.0f);
    EXPECT_EQ(fromQ.W, 4.0f);

    Color128 c(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 fromC(c);
    EXPECT_EQ(fromC.X, 1.0f);
    EXPECT_EQ(fromC.Y, 2.0f);
    EXPECT_EQ(fromC.Z, 3.0f);
    EXPECT_EQ(fromC.W, 4.0f);
}

TEST_F(Vector4Test, StaticConstants_PredefinedVectors_MatchesExpectedValues)
{
    EXPECT_EQ(Vector4::sUnitX, Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    EXPECT_EQ(Vector4::sUnitY, Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    EXPECT_EQ(Vector4::sUnitZ, Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    EXPECT_EQ(Vector4::sUnitW, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    EXPECT_EQ(Vector4::sZero, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
}

TEST_F(Vector4Test, OperatorAssign_AnotherVector_CopiesValues)
{
    zero = unitX;
    EXPECT_EQ(zero, unitX);
}

TEST_F(Vector4Test, OperatorAdd_TwoVectors_ReturnsSum)
{
    EXPECT_EQ(vec1234 + vec3456, Vector4(4.0f, 6.0f, 8.0f, 10.0f));
    vec1234 += vec3456;
    EXPECT_EQ(vec1234, Vector4(4.0f, 6.0f, 8.0f, 10.0f));
}

TEST_F(Vector4Test, OperatorSub_TwoVectors_ReturnsDifference)
{
    EXPECT_EQ(vec3456 - vec1234, Vector4(2.0f, 2.0f, 2.0f, 2.0f));
    vec3456 -= vec1234;
    EXPECT_EQ(vec3456, Vector4(2.0f, 2.0f, 2.0f, 2.0f));
}

TEST_F(Vector4Test, OperatorMul_TwoVectors_ReturnsComponentWiseProduct)
{
    EXPECT_EQ(vec1234 * vec3456, Vector4(3.0f, 8.0f, 15.0f, 24.0f));
    vec1234 *= vec3456;
    EXPECT_EQ(vec1234, Vector4(3.0f, 8.0f, 15.0f, 24.0f));
}

TEST_F(Vector4Test, OperatorDiv_TwoVectors_ReturnsComponentWiseQuotient)
{
    EXPECT_EQ(vec3456 / vec1234, Vector4(3.0f, 2.0f, 5.0f / 3.0f, 1.5f));
    vec3456 /= vec1234;
    EXPECT_EQ(vec3456, Vector4(3.0f, 2.0f, 5.0f / 3.0f, 1.5f));

    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    v /= zero;
    EXPECT_TRUE(std::isinf(v.X));
    EXPECT_TRUE(std::isinf(v.Y));
    EXPECT_TRUE(std::isinf(v.Z));
    EXPECT_TRUE(std::isinf(v.W));
}

TEST_F(Vector4Test, OperatorMul_ScalarValue_ScalesVector)
{
    EXPECT_EQ(vec1234 * 2.0f, Vector4(2.0f, 4.0f, 6.0f, 8.0f));
    EXPECT_EQ(2.0f * vec1234, Vector4(2.0f, 4.0f, 6.0f, 8.0f));
    vec1234 *= 2.0f;
    EXPECT_EQ(vec1234, Vector4(2.0f, 4.0f, 6.0f, 8.0f));
}

TEST_F(Vector4Test, OperatorDiv_ScalarValue_ScalesVector)
{
    EXPECT_EQ(vec3456 / 2.0f, Vector4(1.5f, 2.0f, 2.5f, 3.0f));
    vec3456 /= 2.0f;
    EXPECT_EQ(vec3456, Vector4(1.5f, 2.0f, 2.5f, 3.0f));

    vec3456 /= 0.0f;
    EXPECT_TRUE(std::isinf(vec3456.X));
    EXPECT_TRUE(std::isinf(vec3456.Y));
    EXPECT_TRUE(std::isinf(vec3456.Z));
    EXPECT_TRUE(std::isinf(vec3456.W));
}

TEST_F(Vector4Test, OperatorUnaryMinus_NegativeOne_NegatesComponents)
{
    EXPECT_EQ(-vec1234, Vector4(-1.0f, -2.0f, -3.0f, -4.0f));
}

TEST_F(Vector4Test, OperatorComparison_DifferentVectors_ReturnsCorrectBoolean)
{
    Vector4 vecEqual(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(vec1234 == vecEqual);
    EXPECT_FALSE(vec1234 != vecEqual);
    EXPECT_TRUE(vec1234 != vec3456);

    EXPECT_TRUE(vec1234 < vec3456);
    EXPECT_TRUE(vec1234 <= vec3456);
    EXPECT_TRUE(vec1234 > unitX);
    EXPECT_TRUE(vec1234 >= unitX);
}

TEST_F(Vector4Test, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    EXPECT_TRUE(vec1234.IsEqualApprox(vec1234Approx));
    EXPECT_FALSE(vec1234.IsNotEqualApprox(vec1234Approx));
}

TEST_F(Vector4Test, IsFinite_VectorHasInfComponent_ReturnsFalse)
{
    Vector4 v(1.0f, std::numeric_limits<float>::infinity(), 3.0f, 4.0f);
    EXPECT_FALSE(v.IsFinite());

    Vector4 u(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_TRUE(u.IsFinite());
}

TEST_F(Vector4Test, IsUnit_UnitAndNonUnitVectors_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(unitX.IsUnit());
    EXPECT_TRUE(unitX.IsUnitApprox());

    Vector4 nearUnit(0.9999f, 0.0f, 0.0f, 0.0f);
    EXPECT_FALSE(nearUnit.IsUnit());
    EXPECT_TRUE(nearUnit.IsUnitApprox());
}

TEST_F(Vector4Test, Dot_TwoVectors_ReturnsCorrectScalar)
{
    EXPECT_NEAR(zero.Dot(unitX), 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(vec1234.Dot(vec3456), 50.0f,
                math::EPSILON_CMP); // 1*3+2*4+3*5+4*6
}

TEST_F(Vector4Test, Cross_TwoVectors_ReturnsCorrectVector)
{
    EXPECT_NEAR(vec3456.Magnitude(), std::sqrt(86.0f), math::EPSILON_CMP);
    EXPECT_NEAR(vec3456.SqrdMagnitude(), 86.0f, math::EPSILON_CMP);
}

TEST_F(Vector4Test, Magnitude_TypicalVector_ReturnsLength)
{
    Vector4 n = vec3456.Normalized();
    EXPECT_NEAR(n.Magnitude(), 1.0f, math::EPSILON_CMP);

    vec3456.Normalize();
    EXPECT_NEAR(vec3456.Magnitude(), 1.0f, math::EPSILON_CMP);

    vec01020304.Normalize();
    EXPECT_NEAR(vec01020304.Magnitude(), 1.0f, math::EPSILON_CMP);
}

TEST_F(Vector4Test, Normalize_NonZeroVector_MakeVectorToUnit)
{
    Vector4 n = zero.Normalized();
    EXPECT_TRUE(std::isnan(n.X));
    EXPECT_TRUE(std::isnan(n.Y));
    EXPECT_TRUE(std::isnan(n.Z));
    EXPECT_TRUE(std::isnan(n.W));

    zero.Normalize();
    EXPECT_TRUE(std::isnan(zero.X));
    EXPECT_TRUE(std::isnan(zero.Y));
    EXPECT_TRUE(std::isnan(zero.Z));
    EXPECT_TRUE(std::isnan(zero.W));
}

TEST_F(Vector4Test, Projected_ValidVectors_ReturnsProjectedVector)
{
    Vector4 onto(2.0f, 5.0f, 1.0f, 3.0f);
    float dot = vec3456.Dot(onto);
    float denom = onto.SqrdMagnitude();
    Vector4 expected = (dot / denom) * onto;

    Vector4 projected = vec3456.Projected(onto);
    EXPECT_TRUE(projected.IsEqualApprox(expected));

    vec3456.Project(onto);
    EXPECT_TRUE(vec3456.IsEqualApprox(expected));
}

TEST_F(Vector4Test, Projected_ParallelAndPerpendicularCases_ReturnsExpectedVector)
{
    Vector4 a = 2.0f * vec1234;
    Vector4 p = a.Projected(vec1234);
    EXPECT_TRUE(p.IsEqualApprox(a));

    Vector4 proj = unitX.Projected(unitY);
    EXPECT_TRUE(proj.IsEqualApprox(Vector4::sZero));
}

TEST_F(Vector4Test, Projected_DifferentLengthOntoVectors_ReturnsSameResult)
{
    Vector4 onto1(0.0f, 1.0f, 0.0f, 0.0f);
    Vector4 onto2(0.0f, 10.0f, 0.0f, 0.0f);

    Vector4 p1 = vec3456.Projected(onto1);
    Vector4 p2 = vec3456.Projected(onto2);

    EXPECT_TRUE(p1.IsEqualApprox(p2));
}

TEST_F(Vector4Test, ToCartesian_Vector4_ReturnsVector3WithDividedByW)
{
    Vector3 c = vec1234.ToCartesian();

    EXPECT_TRUE(c.IsEqualApprox(Vector3(1.0f / 4.0f, 2.0f / 4.0f, 3.0f / 4.0f)));

    Vector4 v(3.0f, 4.0f, 5.0f, 0.0f);
    c = v.ToCartesian();
    EXPECT_TRUE(std::isinf(c.X));
    EXPECT_TRUE(std::isinf(c.Y));
    EXPECT_TRUE(std::isinf(c.Z));
}
