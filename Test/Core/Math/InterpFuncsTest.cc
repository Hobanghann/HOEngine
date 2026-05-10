#include "Core/Math/InterpFuncs.h"

#include <gtest/gtest.h>

#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"

using namespace ho;
using namespace ho::math;

TEST(InterpFuncsTest, GetBarycentric2D_MidPoint_ReturnsHalfWeights)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(10.0f, 0.0f, 0.0f);
    Vector3 target(5.0f, 0.0f, 0.0f);

    Vector2 bary = GetBarycentric(target, v1, v2);
    EXPECT_NEAR(bary.X, 0.5f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.Y, 0.5f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.X + bary.Y, 1.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, GetBarycentric2D_AtEndpoints_ReturnsBinaryWeights)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(10.0f, 0.0f, 0.0f);

    Vector3 target = v1;
    Vector2 bary = GetBarycentric(target, v1, v2);
    EXPECT_NEAR(bary.X, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.Y, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.X + bary.Y, 1.0f, math::EPSILON_CMP);

    target = v2;
    bary = GetBarycentric(target, v1, v2);
    EXPECT_NEAR(bary.X, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.Y, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.X + bary.Y, 1.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, GetBarycentric2D_SamePoints_ReturnsFallbackWeights)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(0.0f, 0.0f, 0.0f);
    Vector3 target(1.0f, 0.0f, 0.0f);

    Vector2 bary = GetBarycentric(target, v1, v2);
    EXPECT_NEAR(bary.X, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.Y, 0.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, GetBarycentric3D_InsideTriangle_ReturnsCorrectSumOfWeights)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(1.0f, 0.0f, 0.0f);
    Vector3 v3(0.0f, 1.0f, 0.0f);
    Vector3 p(0.25f, 0.25f, 0.0f);

    Vector3 bary = GetBarycentric(p, v1, v2, v3);

    EXPECT_NEAR(bary.X + bary.Y + bary.Z, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.X, 0.5f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.Y, 0.25f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.Z, 0.25f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, GetBarycentric3D_AtVectors_ReturnsBinaryWeights)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(1.0f, 0.0f, 0.0f);
    Vector3 v3(0.0f, 1.0f, 0.0f);

    Vector3 p = v1;
    Vector3 bary = GetBarycentric(p, v1, v2, v3);

    EXPECT_NEAR(bary.X, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.Y, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.Z, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.X + bary.Y + bary.Z, 1.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, GetBarycentric3D_OnEdge_ReturnsOneZeroWeight)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(1.0f, 0.0f, 0.0f);
    Vector3 v3(0.0f, 1.0f, 0.0f);

    Vector3 p(0.5f, 0.0f, 0.0f);
    Vector3 bary = GetBarycentric(p, v1, v2, v3);

    EXPECT_NEAR(bary.X, 0.5f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.Y, 0.5f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.Z, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(bary.X + bary.Y + bary.Z, 1.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, GetBarycentric3D_OverlappedVectors_ReturnsZeroVector)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(0.0f, 0.0f, 0.0f);
    Vector3 v3(0.0f, 0.0f, 0.0f);
    Vector3 p(1.0f, 0.0f, 0.0f);

    Vector3 bary = GetBarycentric(p, v1, v2, v3);
    EXPECT_EQ(bary, Vector3::sZero);
}

TEST(InterpFuncsTest, GetBarycentric3D_CollinearVectors_ReturnsZeroVector)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(1.0f, 0.0f, 0.0f);
    Vector3 v3(2.0f, 0.0f, 0.0f);
    Vector3 p(1.0f, 0.0f, 0.0f);

    Vector3 bary = GetBarycentric(p, v1, v2, v3);
    EXPECT_EQ(bary, Vector3::sZero);
}

TEST(InterpFuncsTest, Lerp2D_ArbitraryWeights_ReturnsInterpolatedVector)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(10.0f, 0.0f, 0.0f);
    Vector2 bary(0.8f, 0.2f);
    Vector3 result = Lerp(v1, v2, bary);
    EXPECT_FLOAT_EQ(result.X, 2.0f);
    EXPECT_FLOAT_EQ(result.Y, 0.0f);
}

TEST(InterpFuncsTest, Lerp2D_BinaryWeights_ReturnsVertexValues)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(10.0f, 0.0f, 0.0f);

    Vector2 bary0(1.0f, 0.0f);
    Vector3 res0 = Lerp(v1, v2, bary0);
    EXPECT_FLOAT_EQ(res0.X, 0.0f);
    EXPECT_FLOAT_EQ(res0.Y, 0.0f);

    Vector2 bary1(0.0f, 1.0f);
    Vector3 res1 = Lerp(v1, v2, bary1);
    EXPECT_FLOAT_EQ(res1.X, 10.0f);
    EXPECT_FLOAT_EQ(res1.Y, 0.0f);
}

TEST(InterpFuncsTest, Lerp2D_IdenticalVectors_ReturnsSameValue)
{
    Vector3 v1(5.0f, 5.0f, 0.0f);
    Vector3 v2(5.0f, 5.0f, 0.0f);
    Vector2 bary(0.3f, 0.7f);
    Vector3 res = Lerp(v1, v2, bary);
    EXPECT_FLOAT_EQ(res.X, 5.0f);
    EXPECT_FLOAT_EQ(res.Y, 5.0f);
}

TEST(InterpFuncsTest, Lerp3D_InsideTriangle_ReturnsInterpolatedVector)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(10.0f, 0.0f, 0.0f);
    Vector3 v3(0.0f, 10.0f, 0.0f);

    Vector3 bary(0.2f, 0.3f, 0.5f);
    Vector3 res = Lerp(v1, v2, v3, bary);

    EXPECT_NEAR(res.X, 3.0f, math::EPSILON_CMP);
    EXPECT_NEAR(res.Y, 5.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, Lerp3D_BinaryWeights_ReturnsVertexValues)
{
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(10.0f, 0.0f, 0.0f);
    Vector3 v3(0.0f, 10.0f, 0.0f);

    Vector3 baryV1(1.0f, 0.0f, 0.0f);
    Vector3 res1 = Lerp(v1, v2, v3, baryV1);
    EXPECT_EQ(res1, v1);

    Vector3 baryV2(0.0f, 1.0f, 0.0f);
    Vector3 res2 = Lerp(v1, v2, v3, baryV2);
    EXPECT_EQ(res2, v2);

    Vector3 baryV3(0.0f, 0.0f, 1.0f);
    Vector3 res3 = Lerp(v1, v2, v3, baryV3);
    EXPECT_EQ(res3, v3);
}

TEST(InterpFuncsTest, Lerp3D_IdenticalVectors_ReturnsSameValue)
{
    Vector3 v(5.0f, 5.0f, 5.0f);
    Vector3 bary(0.2f, 0.3f, 0.5f);
    Vector3 res = Lerp(v, v, v, bary);

    EXPECT_EQ(res, v);
}

TEST(InterpFuncsTest, PcerpW2D_VaryingInvW_ReturnsInterpolatedW)
{
    Vector2 invW(1.0f, 0.5f); // w1=1, w2=2
    Vector2 bary(0.25f, 0.75f);
    // denom = 0.25*1 + 0.75*0.5 = 0.625 -> 1/0.625 = 1.6
    float w = PcerpW(invW, bary);
    EXPECT_NEAR(w, 1.6f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, PcerpW2D_BinaryWeights_ReturnsEndpointW)
{
    Vector2 invW(1.0f, 0.5f); // w1=1, w2=2

    Vector2 bary1(1.0f, 0.0f);
    EXPECT_NEAR(PcerpW(invW, bary1), 1.0f, math::EPSILON_CMP);

    Vector2 bary2(0.0f, 1.0f);
    EXPECT_NEAR(PcerpW(invW, bary2), 2.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, PcerpW2D_ZeroInvW_ReturnsInfOrNaN)
{
    Vector2 invW(0.0f, 0.0f);
    Vector2 bary(0.5f, 0.5f);

    float w = PcerpW(invW, bary);
    EXPECT_TRUE(std::isinf(w) || std::isnan(w));
}

TEST(InterpFuncsTest, PcerpW3D_VaryingInvW_ReturnsCorrectInterpolatedW)
{
    Vector3 invW(1.0f, 0.5f, 0.25f);
    Vector3 bary(0.2f, 0.3f, 0.5f);
    // denom = 0.2*1 + 0.3*0.5 + 0.5*0.25 = 0.475
    // result = 1/0.475 almost 2.10526
    EXPECT_NEAR(PcerpW(invW, bary), 2.10526f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, PcerpW3D_BinaryWeights_ReturnsVertexW)
{
    Vector3 invW(1.0f, 0.5f, 0.25f);

    Vector3 bary1(1.0f, 0.0f, 0.0f);
    EXPECT_NEAR(PcerpW(invW, bary1), 1.0f, math::EPSILON_CMP);

    Vector3 bary2(0.0f, 1.0f, 0.0f);
    EXPECT_NEAR(PcerpW(invW, bary2), 2.0f, math::EPSILON_CMP);

    Vector3 bary3(0.0f, 0.0f, 1.0f);
    EXPECT_NEAR(PcerpW(invW, bary3), 4.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, PcerpW3D_ZeroInvW_ReturnsInfOrNaN)
{
    Vector3 invW(0.0f, 0.0f, 0.0f);
    Vector3 bary(0.3f, 0.3f, 0.4f);
    float w = PcerpW(invW, bary);
    EXPECT_TRUE(std::isinf(w) || std::isnan(w));
}

TEST(InterpFuncsTest, PcerpWithInvW2D_PrecalculatedW_ReturnsCorrectAttributeValue)
{
    float a1 = 10.0f, a2 = 20.0f;
    Vector2 invW(1.0f, 0.5f); // w1=1, w2=2
    Vector2 bary(0.25f, 0.75f);
    float interpolatedW = PcerpW(invW, bary); // = 1 / (0.25*1 + 0.75*0.5) = 1.6

    float result = Pcerp(a1, a2, bary, invW, interpolatedW);

    // expected: 1.6 * (0.25*1*10 + 0.75*0.5*20) = 1.6 * (2.5 + 7.5) = 16
    EXPECT_NEAR(result, 16.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, PcerpWithInvW2D_BinaryWeights_ReturnsEndpointAttribute)
{
    Vector2 invW(1.0f, 0.5f);
    float a1 = 10.0f, a2 = 20.0f;

    Vector2 bary(1.0f, 0.0f);
    float interpolatedW = PcerpW(invW, bary); // = 1/1 = 1
    float result = Pcerp(a1, a2, bary, invW, interpolatedW);
    EXPECT_NEAR(result, a1, math::EPSILON_CMP);

    bary = Vector2(0.0f, 1.0f);
    interpolatedW = PcerpW(invW, bary); // = 1/0.5 = 2
    result = Pcerp(a1, a2, bary, invW, interpolatedW);
    EXPECT_NEAR(result, a2, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, PcerpWithInvW2D_InfOrNaNW_ReturnsInfOrNaNAttribute)
{
    Vector2 invW(0.0f, 0.0f);
    Vector2 bary(0.5f, 0.5f);
    float a1 = 10.0f, a2 = 20.0f;
    float interpolatedW = PcerpW(invW, bary); // denom = inf

    float result = Pcerp(a1, a2, bary, invW, interpolatedW);
    EXPECT_TRUE(std::isnan(result) || std::isinf(result));
}

TEST(InterpFuncsTest, PcerpWithInvW3D_PrecalculatedW_ReturnsCorrectAttributeValue)
{
    Vector3 invW(1.0f, 0.5f, 0.25f);
    Vector3 bary(0.2f, 0.3f, 0.5f);
    float a1 = 10.0f, a2 = 20.0f, a3 = 30.0f;
    float interpolatedW = PcerpW(invW, bary); // 1 / (0.475) almost 2.10526

    float result = Pcerp(a1, a2, a3, bary, invW, interpolatedW);
    // numerator = 0.2*1*10 + 0.3*0.5*20 + 0.5*0.25*30 = 2 + 3 + 3.75 = 8.75
    // result = 2.10526 * 8.75 = 18.42
    EXPECT_NEAR(result, 18.42f, 0.01f);
}

TEST(InterpFuncsTest, PcerpWithInvW3D_BinaryWeights_ReturnsEndpointAttribute)
{
    Vector3 invW(1.0f, 0.5f, 0.25f);
    float a1 = 10.0f, a2 = 20.0f, a3 = 30.0f;

    Vector3 bary(1.0f, 0.0f, 0.0f);
    float w = PcerpW(invW, bary);
    float result = Pcerp(a1, a2, a3, bary, invW, w);
    EXPECT_NEAR(result, 10.0f, math::EPSILON_CMP);

    bary = Vector3(0.0f, 1.0f, 0.0f);
    w = PcerpW(invW, bary);
    result = Pcerp(a1, a2, a3, bary, invW, w);
    EXPECT_NEAR(result, 20.0f, math::EPSILON_CMP);

    bary = Vector3(0.0f, 0.0f, 1.0f);
    w = PcerpW(invW, bary);
    result = Pcerp(a1, a2, a3, bary, invW, w);
    EXPECT_NEAR(result, 30.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, PcerpWithInvW3D_InfOrNaNW_ReturnsInfOrNaNAttribute)
{
    Vector3 invW(0.0f, 0.0f, 0.0f);
    Vector3 bary(0.3f, 0.3f, 0.4f);
    float a1 = 10.0f, a2 = 20.0f, a3 = 30.0f;
    float w = PcerpW(invW, bary);

    float result = Pcerp(a1, a2, a3, bary, invW, w);
    EXPECT_TRUE(std::isinf(result) || std::isnan(result));
}

TEST(InterpFuncsTest, Pcerp2D_Vector4W_ReturnsPerspectiveCorrectValue)
{
    // a1=10, a2=20 -> w1=1, w2=2
    Vector4 v1(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 v2(1.0f, 0.0f, 0.0f, 2.0f);
    Vector2 bary(0.25f, 0.75f);

    float a1 = 10.0f, a2 = 20.0f;
    float res = Pcerp(a1, a2, bary, v1, v2);

    // Perspective-correct interpolation:
    // invW = (1, 0.5)
    // denom = 0.25*1 + 0.75*0.5 = 0.625
    // result = (1/0.625) * (0.25*1*10 + 0.75*0.5*20)
    //         = 1.6 * (2.5 + 7.5) = 16
    EXPECT_NEAR(res, 16.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, Pcerp2D_BinaryWeights_ReturnsEndpointAttribute)
{
    Vector4 v1(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 v2(1.0f, 0.0f, 0.0f, 2.0f);
    float a1 = 10.0f, a2 = 20.0f;

    Vector2 bary1(1.0f, 0.0f);
    Vector2 bary2(0.0f, 1.0f);

    EXPECT_NEAR(Pcerp(a1, a2, bary1, v1, v2), 10.0f, math::EPSILON_CMP);
    EXPECT_NEAR(Pcerp(a1, a2, bary2, v1, v2), 20.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, Pcerp2D_ZeroW_ReturnsInfOrNaN)
{
    Vector4 v1(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4 v2(1.0f, 0.0f, 0.0f, 0.0f);
    float a1 = 10.0f, a2 = 20.0f;
    Vector2 bary(0.5f, 0.5f);

    float res = Pcerp(a1, a2, bary, v1, v2);
    EXPECT_TRUE(std::isnan(res) || std::isinf(res));
}

TEST(InterpFuncsTest, Pcerp3D_Vector4W_ReturnsPerspectiveCorrectValue)
{
    // a1=10, a2=20, a3=30 -> w1=1, w2=2, w3=4
    Vector4 v1(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 v2(1.0f, 0.0f, 0.0f, 2.0f);
    Vector4 v3(0.0f, 1.0f, 0.0f, 4.0f);
    Vector3 bary(0.2f, 0.3f, 0.5f);

    float a1 = 10.0f, a2 = 20.0f, a3 = 30.0f;
    float res = Pcerp(a1, a2, a3, bary, v1, v2, v3);

    // invW = (1, 0.5, 0.25)
    // denom = 0.2*1 + 0.3*0.5 + 0.5*0.25 = 0.475
    // numerator = 0.2*1*10 + 0.3*0.5*20 + 0.5*0.25*30
    //            = 2 + 3 + 3.75 = 8.75
    // result = (1/0.475)*8.75 almost 18.42
    EXPECT_NEAR(res, 18.42f, 0.01f);
}

TEST(InterpFuncsTest, Pcerp3D_BinaryWeights_ReturnsEndpointAttribute)
{
    Vector4 v1(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 v2(1.0f, 0.0f, 0.0f, 2.0f);
    Vector4 v3(0.0f, 1.0f, 0.0f, 4.0f);
    float a1 = 10.0f, a2 = 20.0f, a3 = 30.0f;

    Vector3 b1(1.0f, 0.0f, 0.0f);
    Vector3 b2(0.0f, 1.0f, 0.0f);
    Vector3 b3(0.0f, 0.0f, 1.0f);

    EXPECT_NEAR(Pcerp(a1, a2, a3, b1, v1, v2, v3), 10.0f, math::EPSILON_CMP);
    EXPECT_NEAR(Pcerp(a1, a2, a3, b2, v1, v2, v3), 20.0f, math::EPSILON_CMP);
    EXPECT_NEAR(Pcerp(a1, a2, a3, b3, v1, v2, v3), 30.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, Pcerp3D_ZeroW_ReturnsInfOrNaN)
{
    Vector4 v1(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4 v2(1.0f, 0.0f, 0.0f, 0.0f);
    Vector4 v3(0.0f, 1.0f, 0.0f, 0.0f);
    float a1 = 10.0f, a2 = 20.0f, a3 = 30.0f;
    Vector3 bary(0.3f, 0.3f, 0.4f);

    float res = Pcerp(a1, a2, a3, bary, v1, v2, v3);
    EXPECT_TRUE(std::isnan(res) || std::isinf(res));
}

TEST(InterpFuncsTest, Slerp_StandardRotation_ReturnsMidpointQuaternion)
{
    Quaternion q1 = Quaternion::FromAxisAngle(Vector3::sUnitY, 0.0f);
    Quaternion q2 = Quaternion::FromAxisAngle(Vector3::sUnitY, math::PI * 0.5f);

    Vector2 bary(0.5f, 0.5f);
    Quaternion result = Slerp(q1, q2, bary);

    Quaternion expected = Quaternion::FromAxisAngle(Vector3::sUnitY, math::PI * 0.25f);

    // quaternions may be negated and still represent the same rotation
    EXPECT_NEAR(std::fabs(result.Dot(expected)), 1.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, Slerp_IdenticalInputs_ReturnsSameQuaternion)
{
    Quaternion q1 = Quaternion::FromAxisAngle(Vector3::sUnitX, math::PI * 0.25f);
    Quaternion q2 = q1; // identical

    Vector2 bary(0.5f, 0.5f);
    Quaternion result = Slerp(q1, q2, bary);

    EXPECT_TRUE(result.IsEqualApprox(q1));
}

TEST(InterpFuncsTest, Slerp_NegatedInputs_ReturnsShortestPath)
{
    Quaternion q1 = Quaternion::FromAxisAngle(Vector3::sUnitX, math::PI * 0.25f);
    Quaternion q2 = -q1;

    Vector2 bary(0.5f, 0.5f);
    Quaternion result = Slerp(q1, q2, bary);

    EXPECT_TRUE(result.IsUnitApprox());
    EXPECT_NEAR(std::fabs(result.Dot(q1)), 1.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, Slerp_BinaryWeights_ReturnsEndpointQuaternions)
{
    Quaternion q1 = Quaternion::FromAxisAngle(Vector3::sUnitY, 0.0f);
    Quaternion q2 = Quaternion::FromAxisAngle(Vector3::sUnitY, math::PI * 0.5f);

    Vector2 bary1(1.0f, 0.0f);
    Quaternion result1 = Slerp(q1, q2, bary1);
    EXPECT_TRUE(result1.IsEqualApprox(q1));

    Vector2 bary2(0.0f, 1.0f);
    Quaternion result2 = Slerp(q1, q2, bary2);
    EXPECT_TRUE(result2.IsEqualApprox(q2));
}

TEST(InterpFuncsTest, SlerpLong_StandardRotation_ReturnsLongPathMidpoint)
{
    Quaternion q1 = Quaternion::FromAxisAngle(Vector3::sUnitY, 0.0f);
    Quaternion q2 = Quaternion::FromAxisAngle(Vector3::sUnitY, math::PI * 0.5f);

    Vector2 bary(0.5f, 0.5f);
    Quaternion result = SlerpLong(q1, q2, bary);

    Quaternion expected = Quaternion::FromAxisAngle(Vector3::sUnitY, math::PI + math::PI * 0.25f);

    EXPECT_NEAR(std::fabs(result.Dot(expected)), 1.0f, math::EPSILON_CMP);
    EXPECT_TRUE(result.IsUnitApprox());
}

TEST(InterpFuncsTest, SlerpLong_IdenticalInputs_ReturnsSameQuaternion)
{
    Quaternion q1 = Quaternion::FromAxisAngle(Vector3::sUnitX, math::PI * 0.25f);
    Quaternion q2 = q1; // identical

    Vector2 bary(0.5f, 0.5f);
    Quaternion result = SlerpLong(q1, q2, bary);

    EXPECT_TRUE(result.IsEqualApprox(q1));
}

TEST(InterpFuncsTest, SlerpLong_NegatedInputs_DoesNotFlipShortestPath)
{
    Quaternion q1 = Quaternion::FromAxisAngle(Vector3::sUnitX, 0.0f);
    Quaternion q2 = -q1;

    Vector2 bary(0.5f, 0.5f);
    Quaternion result = SlerpLong(q1, q2, bary);

    EXPECT_TRUE(result.IsUnitApprox());
    EXPECT_NEAR(std::fabs(result.Dot(q1)), 1.0f, math::EPSILON_CMP);
}

TEST(InterpFuncsTest, SlerpLong_BinaryWeights_ReturnsEndpointQuaternions)
{
    Quaternion q1 = Quaternion::FromAxisAngle(Vector3::sUnitZ, 0.0f);
    Quaternion q2 = Quaternion::FromAxisAngle(Vector3::sUnitZ, math::PI);

    Vector2 bary1(1.0f, 0.0f);
    Quaternion res1 = SlerpLong(q1, q2, bary1);
    EXPECT_TRUE(res1.IsEqualApprox(q1));

    Vector2 bary2(0.0f, 1.0f);
    Quaternion res2 = SlerpLong(q1, q2, bary2);
    EXPECT_TRUE(res2.IsEqualApprox(q2));
}

TEST(InterpFuncsTest, SlerpLong_IdenticalQuaternions_ReturnsSameQuaternion)
{
    Quaternion q1 = Quaternion::FromAxisAngle(Vector3::sUnitX, math::PI * 0.25f);
    Quaternion q2 = q1;

    Vector2 bary(0.5f, 0.5f);
    Quaternion result = SlerpLong(q1, q2, bary);

    EXPECT_TRUE(result.IsEqualApprox(q1));
    EXPECT_TRUE(result.IsUnitApprox());
}
