#include "Core/Math/Color32.h"

#include <gtest/gtest.h>

#include "Core/Math/Color128.h"

using namespace ho;

TEST(Color32Test, Constructor_DefaultAndAllParameters_InitializesWithClampingOrWrap)
{
    Color32 c;
    EXPECT_EQ(c.R, 0);
    EXPECT_EQ(c.G, 0);
    EXPECT_EQ(c.B, 0);
    EXPECT_EQ(c.A, 255);

    Color32 c1(255, 255, 255);
    EXPECT_EQ(c1.R, 255);
    EXPECT_EQ(c1.G, 255);
    EXPECT_EQ(c1.B, 255);
    EXPECT_EQ(c1.A, 255);

    Color32 c2(
        static_cast<uint8_t>(300), static_cast<uint8_t>(300), static_cast<uint8_t>(300), static_cast<uint8_t>(300));
    EXPECT_EQ(c2.R, 44);
    EXPECT_EQ(c2.G, 44);
    EXPECT_EQ(c2.B, 44);
    EXPECT_EQ(c2.A, 44);

    Color32 c3(
        static_cast<uint8_t>(-10), static_cast<uint8_t>(-10), static_cast<uint8_t>(-10), static_cast<uint8_t>(-10));
    EXPECT_EQ(c3.R, 246);
    EXPECT_EQ(c3.G, 246);
    EXPECT_EQ(c3.B, 246);
    EXPECT_EQ(c3.A, 246);

    Color128 c128(0.5f, 0.5f, 0.5f, 1.0f);
    c = Color32(c128);
    EXPECT_EQ(c.R, 127);
    EXPECT_EQ(c.G, 127);
    EXPECT_EQ(c.B, 127);
    EXPECT_EQ(c.A, 255);

    c128 = Color128(2.0f, 2.0f, 2.0f, 1.0f);
    c = Color32(c128);
    EXPECT_EQ(c.R, 255);
    EXPECT_EQ(c.G, 255);
    EXPECT_EQ(c.B, 255);
    EXPECT_EQ(c.A, 255);
}

TEST(Color32Test, OperatorAssign_AnotherColor_CopiesValues)
{
    Color32 c(10, 20, 30, 40);
    Color32 c1;
    c1 = c;
    EXPECT_EQ(c1.R, 10);
    EXPECT_EQ(c1.G, 20);
    EXPECT_EQ(c1.B, 30);
    EXPECT_EQ(c1.A, 40);
}

TEST(Color32Test, OperatorAdd_TwoColors_ReturnsClampedSum)
{
    Color32 a(100, 150, 200, 250);
    Color32 b(110, 100, 100, 10);
    Color32 sum = a + b;

    EXPECT_EQ(sum, Color32(210, 250, 255, 255)); // clamped to 255

    a += b;
    EXPECT_EQ(a, sum);
}

TEST(Color32Test, OperatorSub_TwoColors_ReturnsFlooredDifference)
{
    Color32 a(100, 150, 200, 250);
    Color32 b(110, 100, 220, 10);
    EXPECT_EQ(a - b, Color32(0, 50, 0, 240)); // floor to 0

    a -= b;
    EXPECT_EQ(a, Color32(0, 50, 0, 240)); // not same!
}

TEST(Color32Test, OperatorMul_TwoColors_ReturnsClampedProduct)
{
    Color32 a(10, 20, 30, 40);
    Color32 b(2, 4, 8, 10);
    Color32 mul = a * b;
    EXPECT_EQ(mul, Color32(20, 80, 240, 255)); // 40*10=400 -> clamped

    a *= b;
    EXPECT_EQ(a, mul);
}

TEST(Color32Test, OperatorDiv_TwoColors_HandlesZeroDivisionByReturningMax)
{
    Color32 div = Color32(100, 100, 100, 100) / Color32(10, 5, 2, 0);
    // Division by zero -> 255
    EXPECT_EQ(div, Color32(10, 20, 50, 255));
}

TEST(Color32Test, OperatorMul_ScalarValue_ReturnsClampedColor)
{
    Color32 a(10, 20, 30, 40);
    Color32 mul = a * 5;
    EXPECT_EQ(mul, Color32(50, 100, 150, 200));

    a *= 10;
    EXPECT_EQ(a, Color32(100, 200, 255, 255)); // clamped

    Color32 left = 2 * Color32(5, 6, 7, 8);
    EXPECT_EQ(left, Color32(10, 12, 14, 16));
}

TEST(Color32Test, OperatorDiv_ScalarValue_ReturnsCorrectIntegerQuotient)
{
    Color32 a(10, 20, 30, 40);
    Color32 div = a / 2;
    EXPECT_EQ(div, Color32(5, 10, 15, 20));

    Color32 div_zero = a / 0;
    EXPECT_EQ(div_zero, Color32(255, 255, 255, 255));

    a /= 10;
    EXPECT_EQ(a, Color32(1, 2, 3, 4)); // integer division
}

TEST(Color32Test, OperatorComparison_DifferentColors_ReturnsCorrectBoolean)
{
    Color32 a(10, 20, 30, 40);
    Color32 b(10, 20, 30, 40);
    Color32 c(10, 20, 31, 40);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a == c);
}

TEST(Color32Test, Luminance_DefaultAndCustomWeights_ReturnsCorrectBrightness)
{
    Color32 c(100, 150, 200, 255);
    float defaultLuminance = c.Luminance();
    float manualLuminance = 0.2126f * 100 + 0.7152f * 150 + 0.0722f * 200;
    EXPECT_NEAR(defaultLuminance, manualLuminance, math::EPSILON_CMP);

    float customLuminance = c.Luminance(0.3f, 0.59f, 0.11f);
    EXPECT_NEAR(customLuminance, 0.3f * 100 + 0.59f * 150 + 0.11f * 200, math::EPSILON_CMP);
}

TEST(Color32Test, ToR8_Color32_ReturnsRedComponent)
{
    Color32 c(200, 150, 100, 50);
    EXPECT_EQ(c.ToR8(), 200);
}

TEST(Color32Test, ToRG8_Color32_PacksRedAndGreen)
{
    Color32 c(1, 2, 3, 4);
    uint16_t packed = c.ToRG8();
    uint8_t bytes[2];
    std::memcpy(bytes, &packed, 2);
    EXPECT_EQ(bytes[0], 1);
    EXPECT_EQ(bytes[1], 2);
}

TEST(Color32Test, ToRGB8_Color32_PacksRGBWithZeroAlpha)
{
    Color32 c(10, 20, 30, 40);
    uint32_t packed = c.ToRGB8();
    uint8_t bytes[4];
    std::memcpy(bytes, &packed, 4);

    EXPECT_EQ(bytes[0], 10);
    EXPECT_EQ(bytes[1], 20);
    EXPECT_EQ(bytes[2], 30);
    EXPECT_EQ(bytes[3], 0); // alpha dummy 0
}

TEST(Color32Test, ToRGBA8_Color32_PacksRGBAInOrder)
{
    Color32 c(10, 20, 30, 40);
    uint32_t packed = c.ToRGBA8();
    uint8_t bytes[4];
    std::memcpy(bytes, &packed, 4);

    EXPECT_EQ(bytes[0], 10);
    EXPECT_EQ(bytes[1], 20);
    EXPECT_EQ(bytes[2], 30);
    EXPECT_EQ(bytes[3], 40);
}

TEST(Color32Test, ToBGRA8_Color32_PacksInReverseOrder)
{
    Color32 c(10, 20, 30, 40);
    uint32_t packed = c.ToBGRA8();
    uint8_t bytes[4];
    std::memcpy(bytes, &packed, 4);

    EXPECT_EQ(bytes[0], 30);
    EXPECT_EQ(bytes[1], 20);
    EXPECT_EQ(bytes[2], 10);
    EXPECT_EQ(bytes[3], 40);
}

TEST(Color32Test, ToARGB8_Color32_PacksAlphaFirst)
{
    Color32 c(10, 20, 30, 40);
    uint32_t packed = c.ToARGB8();
    uint8_t bytes[4];
    std::memcpy(bytes, &packed, 4);

    EXPECT_EQ(bytes[0], 40);
    EXPECT_EQ(bytes[1], 10);
    EXPECT_EQ(bytes[2], 20);
    EXPECT_EQ(bytes[3], 30);
}

TEST(Color32Test, ToABGR8_Color32_PacksAlphaThenBGR)
{
    Color32 c(10, 20, 30, 40);
    uint32_t packed = c.ToABGR8();
    uint8_t bytes[4];
    std::memcpy(bytes, &packed, 4);

    EXPECT_EQ(bytes[0], 40);
    EXPECT_EQ(bytes[1], 30);
    EXPECT_EQ(bytes[2], 20);
    EXPECT_EQ(bytes[3], 10);
}

TEST(Color32Test, ToL8_Color32_ReturnsLuminanceAsRed)
{
    Color32 c(128, 64, 32, 255);
    EXPECT_EQ(c.ToL8(), 128);
}

TEST(Color32Test, ToLA8_Color32_PacksLuminanceAsRedAndAlpha)
{
    Color32 c(128, 64, 32, 255);
    uint16_t packed = c.ToLA8();
    uint8_t bytes[2];
    std::memcpy(bytes, &packed, 2);
    EXPECT_EQ(bytes[0], 128);
    EXPECT_EQ(bytes[1], 255);
}

TEST(Color32Test, ToRGBA8_PackAndUnpack_MatchesOriginalColor)
{
    Color32 c(1, 2, 3, 4);
    uint32_t packed = c.ToRGBA8();

    uint8_t bytes[4];
    std::memcpy(bytes, &packed, 4);

    Color32 recovered(bytes[0], bytes[1], bytes[2], bytes[3]);
    EXPECT_TRUE(c == recovered);
}
