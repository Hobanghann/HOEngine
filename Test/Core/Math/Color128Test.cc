#include "Core/Math/Color128.h"

#include <gtest/gtest.h>

#include "Core/Math/Color32.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"

using namespace ho;

TEST(Color128Test, Constructor_DefaultAndAllParameters_InitializesFloatComponentsCorrectly)
{
    Color128 c;
    EXPECT_FLOAT_EQ(c.R, 0.0f);
    EXPECT_FLOAT_EQ(c.G, 0.0f);
    EXPECT_FLOAT_EQ(c.B, 0.0f);
    EXPECT_FLOAT_EQ(c.A, 1.0f);

    Color128 c1(0.5f, 0.25f, 0.75f);
    EXPECT_FLOAT_EQ(c1.R, 0.5f);
    EXPECT_FLOAT_EQ(c1.G, 0.25f);
    EXPECT_FLOAT_EQ(c1.B, 0.75f);
    EXPECT_FLOAT_EQ(c1.A, 1.0f);

    Color128 c2(1.2f, -0.5f, 2.0f, 0.0f);
    EXPECT_FLOAT_EQ(c2.R, 1.2f);
    EXPECT_FLOAT_EQ(c2.G, -0.5f);
    EXPECT_FLOAT_EQ(c2.B, 2.0f);
    EXPECT_FLOAT_EQ(c2.A, 0.0f);

    Color32 c32(128, 64, 32, 255);
    Color128 from32(c32);
    EXPECT_FLOAT_EQ(from32.R, 128.0f / 255.0f);
    EXPECT_FLOAT_EQ(from32.G, 64.0f / 255.0f);
    EXPECT_FLOAT_EQ(from32.B, 32.0f / 255.0f);
    EXPECT_FLOAT_EQ(from32.A, 255.0f / 255.0f);

    Vector4 vec4(1.2f, -0.5f, 2.0f, 0.0f);
    Color128 fromVec4(vec4);
    EXPECT_FLOAT_EQ(fromVec4.R, 1.2f);
    EXPECT_FLOAT_EQ(fromVec4.G, -0.5f);
    EXPECT_FLOAT_EQ(fromVec4.B, 2.0f);
    EXPECT_FLOAT_EQ(fromVec4.A, 0.0f);
}

TEST(Color128Test, OperatorAssign_AnotherColor_CopiesFloatValues)
{
    Color128 c(0.1f, 0.2f, 0.3f, 0.4f);
    Color128 c1;
    c1 = c;
    EXPECT_FLOAT_EQ(c1.R, 0.1f);
    EXPECT_FLOAT_EQ(c1.G, 0.2f);
    EXPECT_FLOAT_EQ(c1.B, 0.3f);
    EXPECT_FLOAT_EQ(c1.A, 0.4f);
}

TEST(Color128Test, OperatorAdd_TwoColors_ReturnsFloatingPointSum)
{
    Color128 a(0.2f, 0.2f, 0.4f, 0.5f);
    Color128 b(0.1f, 0.3f, 0.8f, 0.5f);

    Color128 sum = a + b;

    EXPECT_FLOAT_EQ(sum.R, 0.3f);
    EXPECT_FLOAT_EQ(sum.G, 0.5f);
    EXPECT_FLOAT_EQ(sum.B, 1.2f);
    EXPECT_FLOAT_EQ(sum.A, 1.0f);

    a += b;
    EXPECT_TRUE(a.IsEqualApprox(sum));
}

TEST(Color128Test, OperatorSub_TwoColors_ReturnsCorrectDifference)
{
    Color128 a(0.2f, 0.2f, 0.4f, 0.5f);
    Color128 b(0.1f, 0.3f, 0.8f, 0.5f);

    Color128 diff = a - b;
    EXPECT_FLOAT_EQ(diff.R, 0.1f);
    EXPECT_FLOAT_EQ(diff.G, -0.1f);
    EXPECT_FLOAT_EQ(diff.B, -0.4f);
    EXPECT_FLOAT_EQ(diff.A, 0.0f);

    a -= b;
    EXPECT_TRUE(a.IsEqualApprox(Color128(0.1f, -0.1f, -0.4f, 0.0f)));
}

TEST(Color128Test, OperatorMul_TwoColors_ReturnsLinearComponentWiseProduct)
{
    Color128 a(0.2f, 0.4f, 0.6f, 0.8f);
    Color128 b(0.5f, 0.25f, 0.1f, 0.5f);

    Color128 mul = a * b;
    EXPECT_FLOAT_EQ(mul.R, 0.1f);
    EXPECT_FLOAT_EQ(mul.G, 0.1f);
    EXPECT_FLOAT_EQ(mul.B, 0.06f);
    EXPECT_FLOAT_EQ(mul.A, 0.4f);

    a *= b;
    EXPECT_TRUE(a.IsEqualApprox(mul));
}

TEST(Color128Test, OperatorDiv_TwoColors_HandlesInfinityOnZeroDivision)
{
    Color128 div = Color128(0.5f, 0.6f, 0.8f, 1.0f) / Color128(0.5f, 0.2f, 0.4f, 0.0f);
    EXPECT_FLOAT_EQ(div.R, 1.0f);
    EXPECT_FLOAT_EQ(div.G, 3.0f);
    EXPECT_FLOAT_EQ(div.B, 2.0f);
    EXPECT_TRUE(std::isinf(div.A));
}

TEST(Color128Test, OperatorMul_ScalarValue_ScalesFloatingPointComponents)
{
    Color128 a(0.1f, 0.2f, 0.3f, 0.4f);

    Color128 mul = a * 2.0f;
    EXPECT_FLOAT_EQ(mul.R, 0.2f);
    EXPECT_FLOAT_EQ(mul.G, 0.4f);
    EXPECT_FLOAT_EQ(mul.B, 0.6f);
    EXPECT_FLOAT_EQ(mul.A, 0.8f);

    a *= 10.0f;
    EXPECT_FLOAT_EQ(a.R, 1.0f);
    EXPECT_FLOAT_EQ(a.G, 2.0f);
    EXPECT_FLOAT_EQ(a.B, 3.0f);
    EXPECT_FLOAT_EQ(a.A, 4.0f);

    Color128 left = 2.0f * Color128(0.2f, 0.3f, 0.4f, 0.5f);
    EXPECT_FLOAT_EQ(left.R, 0.4f);
    EXPECT_FLOAT_EQ(left.G, 0.6f);
    EXPECT_FLOAT_EQ(left.B, 0.8f);
    EXPECT_FLOAT_EQ(left.A, 1.0f);
}

TEST(Color128Test, OperatorDiv_ScalarValue_ReturnsCorrectQuotientAndHandlesInf)
{
    Color128 a(0.1f, 0.2f, 0.3f, 0.4f);

    Color128 div = a / 0.5f;
    EXPECT_FLOAT_EQ(div.R, 0.2f);
    EXPECT_FLOAT_EQ(div.G, 0.4f);
    EXPECT_FLOAT_EQ(div.B, 0.6f);
    EXPECT_FLOAT_EQ(div.A, 0.8f);

    div = a / 0.0f;
    EXPECT_TRUE(std::isinf(div.R));
    EXPECT_TRUE(std::isinf(div.G));
    EXPECT_TRUE(std::isinf(div.B));
    EXPECT_TRUE(std::isinf(div.A));

    a /= 2.0f;
    EXPECT_FLOAT_EQ(a.R, 0.1f / 2.0f);
    EXPECT_FLOAT_EQ(a.G, 0.2f / 2.0f);
    EXPECT_FLOAT_EQ(a.B, 0.3f / 2.0f);
    EXPECT_FLOAT_EQ(a.A, 0.4f / 2.0f);
}

TEST(Color128Test, OperatorComparison_DifferentValues_ReturnsCorrectBoolean)
{
    Color128 a(0.1f, 0.2f, 0.3f, 0.4f);
    Color128 b(0.1f, 0.2f, 0.3f, 0.4f);
    Color128 c(0.1f, 0.2f, 0.31f, 0.4f);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a == c);
}

TEST(Color128Test, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Color128 a(0.1f, 0.2f, 0.3f, 0.4f);
    Color128 b(0.1f, 0.2f, 0.3f, 0.4f);
    Color128 c(0.100001f, 0.199999f, 0.3f, 0.4f);

    EXPECT_TRUE(a.IsEqualApprox(b));
    EXPECT_FALSE(a.IsNotEqualApprox(b));
    EXPECT_TRUE(a.IsEqualApprox(c));
}

TEST(Color128Test, Luminance_LinearWeights_ReturnsCorrectIntensity)
{
    Color128 c(0.4f, 0.5f, 0.6f);
    float defaultLuminance = c.Luminance();
    float manualLuminance = 0.2126f * 0.4f + 0.7152f * 0.5f + 0.0722f * 0.6f;
    EXPECT_NEAR(defaultLuminance, manualLuminance, math::EPSILON_CMP);

    float lum_custom = c.Luminance(0.3f, 0.59f, 0.11f);
    EXPECT_NEAR(lum_custom, 0.3f * 0.4f + 0.59f * 0.5f + 0.11f * 0.6f, math::EPSILON_CMP);
}

TEST(Color128Test, Over_ForegroundAndBackground_ReturnsAlphaBlendedResult)
{
    Color128 foreground(1.0f, 0.0f, 0.0f, 0.5f);
    Color128 background(0.0f, 0.0f, 1.0f, 1.0f);

    Color128 result = foreground.Over(background);
    EXPECT_NEAR(result.A, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(result.R, 0.5f * 1.0f + 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(result.G, 0.0f, math::EPSILON_CMP);
}

TEST(Color128Test, Under_BackgroundAndForeground_ReturnsCorrectAlphaCompositing)
{
    Color128 background(0.0f, 1.0f, 0.0f, 1.0f);
    Color128 foreground(1.0f, 0.0f, 0.0f, 0.5f);

    Color128 result = background.Under(foreground);
    EXPECT_NEAR(result.A, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(result.R, 0.5f, math::EPSILON_CMP);
    EXPECT_NEAR(result.G, 0.5f, math::EPSILON_CMP);
    EXPECT_NEAR(result.B, 0.0f, math::EPSILON_CMP);
}

TEST(Color128Test, ColorSpaceConversion_RoundTrip_MatchesOriginalWithApproximation)
{
    Color128 srgb(0.5f, 0.5f, 0.5f, 1.0f);
    Color128 linear = srgb.sRGBToLinear();
    Color128 reconv = linear.LinearTosRGB();

    // Round-trip must approximately equal original
    EXPECT_NEAR(srgb.R, reconv.R, math::EPSILON_CMP);
    EXPECT_NEAR(srgb.G, reconv.G, math::EPSILON_CMP);
    EXPECT_NEAR(srgb.B, reconv.B, math::EPSILON_CMP);
    EXPECT_NEAR(srgb.A, reconv.A, math::EPSILON_CMP);

    Color128 black(0.0f, 0.0f, 0.0f, 1.0f);
    Color128 white(1.0f, 1.0f, 1.0f, 1.0f);

    Color128 blackLinear = black.sRGBToLinear();
    Color128 whiteLinear = white.sRGBToLinear();

    EXPECT_NEAR(blackLinear.R, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(whiteLinear.R, 1.0f, math::EPSILON_CMP);

    Color128 mid(0.5f, 0.5f, 0.5f, 1.0f);
    Color128 midLinear = mid.sRGBToLinear();

    EXPECT_NEAR(midLinear.R, 0.214041f, math::EPSILON_CMP);

    Color128 veryDarkSrgb(0.03f, 0.03f, 0.03f, 1.0f);
    Color128 veryDarkLinear = veryDarkSrgb.sRGBToLinear();

    EXPECT_NEAR(veryDarkLinear.R, 0.002322f, math::EPSILON_CMP);
}
