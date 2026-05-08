#include "Core/Math/MathFuncs.h"

#include <gtest/gtest.h>

#include "Core/Math/Vector3.h"

using namespace ho;
using namespace ho::math;

TEST(MathFuncsTest, Max_VariousInputs_ReturnsGreaterValue)
{
    EXPECT_EQ(Max(3.0f, 5.0f), 5.0f);
    EXPECT_EQ(Max(-1, -2), -1);
    EXPECT_EQ(Max(1.0, 2.0, 3.0), 3.0);
}

TEST(MathFuncsTest, Min_VariousInputs_ReturnsSmallerValue)
{
    EXPECT_EQ(Min(3.0f, 5.0f), 3.0f);
    EXPECT_EQ(Min(-1, -2), -2);
    EXPECT_EQ(Min(3.0, 2.0, 1.0), 1.0);
}

TEST(MathFuncsTest, Clamp_ValuesInsideAndOutsideRange_ReturnsClampedValue)
{
    EXPECT_EQ(Clamp(5.0f, 1.0f, 10.0f), 5.0f);
    EXPECT_EQ(Clamp(-1, 0, 10), 0);
    EXPECT_EQ(Clamp(20.0, 0.0, 10.0), 10.0);
}

TEST(MathFuncsTest, Fmod_PositiveNegativeAndZeroDivisor_ReturnsCorrectRemainder)
{
    EXPECT_FLOAT_EQ(Fmod(5.5f, 2.0f), 1.5f);
    EXPECT_FLOAT_EQ(Fmod(-5.5f, 2.0f), -1.5f);
    EXPECT_FLOAT_EQ(Fmod(5.5f, -2.0f), 1.5f);
    EXPECT_FLOAT_EQ(Fmod(-5.5f, -2.0f), -1.5f);
    EXPECT_FLOAT_EQ(Fmod(5.5f, 0.0f), 0.0f);
}

TEST(MathFuncsTest, Round_PositiveAndNegativeFloatingPoints_ReturnsNearestInteger)
{
    EXPECT_FLOAT_EQ(Round(3.0f), 3.0f);
    EXPECT_FLOAT_EQ(Round(2.6f), 3.0f);
    EXPECT_FLOAT_EQ(Round(2.4f), 2.0f);
    EXPECT_FLOAT_EQ(Round(2.0f), 2.0f);
    EXPECT_FLOAT_EQ(Round(-3.0f), -3.0f);
    EXPECT_FLOAT_EQ(Round(-2.6f), -3.0f);
    EXPECT_FLOAT_EQ(Round(-2.4f), -2.0f);
    EXPECT_FLOAT_EQ(Round(-2.0f), -2.0f);
}

TEST(MathFuncsTest, Floor_PositiveAndNegativeFloatingPoints_ReturnsSmallerInteger)
{
    EXPECT_FLOAT_EQ(Floor(3.0f), 3.0f);
    EXPECT_FLOAT_EQ(Floor(2.9f), 2.0f);
    EXPECT_FLOAT_EQ(Floor(2.0f), 2.0f);
    EXPECT_FLOAT_EQ(Floor(-3.0f), -3.0f);
    EXPECT_FLOAT_EQ(Floor(-2.9f), -3.0f);
    EXPECT_FLOAT_EQ(Floor(-2.0f), -2.0f);
}

TEST(MathFuncsTest, Ceil_PositiveAndNegativeFloatingPoints_ReturnsGreaterInteger)
{
    EXPECT_FLOAT_EQ(Ceil(3.0f), 3.0f);
    EXPECT_FLOAT_EQ(Ceil(2.1f), 3.0f);
    EXPECT_FLOAT_EQ(Ceil(2.0f), 2.0f);
    EXPECT_FLOAT_EQ(Ceil(-3.0f), -3.0f);
    EXPECT_FLOAT_EQ(Ceil(-2.1f), -2.0f);
    EXPECT_FLOAT_EQ(Ceil(-2.0f), -2.0f);
}

TEST(MathFuncsTest, Abs_ZeroPositiveAndNegativeInputs_ReturnsAbsoluteValue)
{
    EXPECT_FLOAT_EQ(Abs(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Abs(5.0f), 5.0f);
    EXPECT_FLOAT_EQ(Abs(-5.0f), 5.0f);
}

TEST(MathFuncsTest, IsEqualApprox_ValuesWithinAndOutsideEpsilon_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(IsEqualApprox(1.0000001f, 1.0000002f, 0.000001f));
    EXPECT_FALSE(IsEqualApprox(1.0f, 1.1f, 0.000001f));

    EXPECT_TRUE(IsNotEqualApprox(1.0f, 2.0f));
    EXPECT_FALSE(IsNotEqualApprox(1.0f, 1.0f));
}

TEST(MathFuncsTest, IsZeroApprox_NearZeroValues_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(IsZeroApprox(0.0f));
    EXPECT_TRUE(IsZeroApprox(1e-10f));
    EXPECT_FALSE(IsZeroApprox(1e-2f));
}

TEST(MathFuncsTest, IsFinite_NormalAndExtremeValues_DetectsInfinityAndNaN)
{
    EXPECT_TRUE(IsFinite(0.0f));
    EXPECT_TRUE(IsFinite(123.456f));
    EXPECT_TRUE(IsFinite(-9999.99f));
    EXPECT_TRUE(IsFinite(math::FLOAT_MAX));
    EXPECT_TRUE(IsFinite(math::FLOAT_MIN));

    EXPECT_FALSE(IsFinite(math::FLOAT_INF));
    EXPECT_FALSE(IsFinite(-math::FLOAT_INF));
    EXPECT_FALSE(IsFinite(math::FLOAT_NaN));
}

TEST(MathFuncsTest, IsInf_InfiniteAndNormalValues_DetectsOnlyInfinity)
{
    EXPECT_TRUE(IsInf(math::FLOAT_INF));
    EXPECT_TRUE(IsInf(-math::FLOAT_INF));

    EXPECT_FALSE(IsInf(0.0f));
    EXPECT_FALSE(IsInf(1.0f));
    EXPECT_FALSE(IsInf(math::FLOAT_NaN));
    EXPECT_FALSE(IsInf(math::FLOAT_MAX));
    EXPECT_FALSE(IsInf(math::FLOAT_MIN));
}

TEST(MathFuncsTest, IsNaN_NaNAndNormalValues_DetectsOnlyNaN)
{
    EXPECT_TRUE(IsNaN(math::FLOAT_NaN));

    EXPECT_FALSE(IsNaN(0.0f));
    EXPECT_FALSE(IsNaN(123.45f));
    EXPECT_FALSE(IsNaN(math::FLOAT_INF));
    EXPECT_FALSE(IsNaN(-math::FLOAT_INF));
}

TEST(MathFuncsTest, Pow_PositiveNegativeAndExtremeExponents_ReturnsCorrectPower)
{
    EXPECT_FLOAT_EQ(Pow(2.0f, 3.0f), 8.0f);
    EXPECT_FLOAT_EQ(Pow(4.0f, 0.5f), 2.0f);
    EXPECT_FLOAT_EQ(Pow(4.0f, -1.0f), 0.25f);
    EXPECT_FLOAT_EQ(Pow(4.0f, 0.0f), 1.0f);
    EXPECT_FLOAT_EQ(Pow(-2.0f, 3.0f), -8.0f);
    EXPECT_FLOAT_EQ(Pow(-2.0f, 4.0f), 16.0f);
    EXPECT_TRUE(std::isnan(Pow(-2.0f, 0.5f)));
    EXPECT_TRUE(std::isinf(Pow(0.0f, -1.0f)));
    EXPECT_FLOAT_EQ(Pow(-3.5f, 0.0f), 1.0f);
}

TEST(MathFuncsTest, PowFast_IntegerExponents_CalculatesPowerQuickly)
{
    EXPECT_FLOAT_EQ(PowFast(2.0f, 3), 8.0f);
    EXPECT_FLOAT_EQ(PowFast(2.0f, -3), 0.125f);
    EXPECT_FLOAT_EQ(PowFast(5.0f, 0), 1.0f);
    EXPECT_FLOAT_EQ(PowFast(-2.0f, 3), -8.0f);
    EXPECT_FLOAT_EQ(PowFast(-2.0f, 4), 16.0f);
    EXPECT_TRUE(std::isinf(PowFast(0.0f, -1)));
    EXPECT_FLOAT_EQ(PowFast(-3.5f, 0), 1.0f);
}

TEST(MathFuncsTest, SqrtAndInvSqrtFast_PositiveAndNegativeInputs_ReturnsApproximatedValues)
{
    float f = 4.0f;

    EXPECT_NEAR(InvSqrtFast(f), 1.0f / std::sqrt(f), math::EPSILON_CMP);
    EXPECT_TRUE(std::isinf(InvSqrtFast(0.0f)));
    EXPECT_TRUE(std::isnan(InvSqrtFast(-1.0f)));

    EXPECT_NEAR(SqrtFast(f), 2.0f, math::EPSILON_CMP);
    EXPECT_TRUE(std::isnan(SqrtFast(0.0f)));
    EXPECT_TRUE(std::isnan(SqrtFast(-1.0f)));

    EXPECT_FLOAT_EQ(Sqrt(9.0f), 3.0f);
}

TEST(MathFuncsTest, Trigonometry_CommonAngles_ReturnsCorrectRatio)
{
    float rad = PI / 4.0f;
    EXPECT_NEAR(Sin(rad), std::sqrt(2.0f) / 2.0f, math::EPSILON_CMP);
    EXPECT_NEAR(Cos(rad), std::sqrt(2.0f) / 2.0f, math::EPSILON_CMP);
    EXPECT_NEAR(Tan(rad), 1.0f, math::EPSILON_CMP);

    float s, c;
    SinCos(&s, &c, static_cast<float>(PI / 4.0f));
    EXPECT_NEAR(s, std::sqrt(2.0f) / 2.0f, math::EPSILON_CMP);
    EXPECT_NEAR(c, std::sqrt(2.0f) / 2.0f, math::EPSILON_CMP);
}

TEST(MathFuncsTest, AngleConversion_DegreesAndRadians_ReturnsCorrectConversion)
{
    EXPECT_FLOAT_EQ(DegToRad(180.0f), PI);
    EXPECT_FLOAT_EQ(DegToRad(45.0f), PI / 4.0f);
    EXPECT_FLOAT_EQ(RadToDeg(PI), 180.0f);
    EXPECT_FLOAT_EQ(RadToDeg(PI / 4.0f), 45.0f);
}

TEST(MathFuncsTest, Distance_TwoPoints_ReturnsLinearAndSquaredDistance)
{
    Vector3 a(0.0f, 0.0f, 0.0f);
    Vector3 b(3.0f, 4.0f, 0.0f);
    EXPECT_FLOAT_EQ(Distance(a, a), 0.0f);
    EXPECT_FLOAT_EQ(Distance(a, b), 5.0f);
    EXPECT_FLOAT_EQ(SqrdDistance(a, a), 0.0f);
    EXPECT_FLOAT_EQ(SqrdDistance(a, b), 25.0f);
}
