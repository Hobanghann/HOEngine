#include "Core/Math/Quaternion.h"

#include <gtest/gtest.h>

#include "Core/Math/MathFuncs.h"
#include "Core/Math/Vector3.h"

using namespace ho;

TEST(QuaternionTest, Constructor_DefaultAndAllParameters_InitializesCorrectly)
{
    Quaternion q;
    EXPECT_FLOAT_EQ(q.X, 0.0f);
    EXPECT_FLOAT_EQ(q.Y, 0.0f);
    EXPECT_FLOAT_EQ(q.Z, 0.0f);
    EXPECT_FLOAT_EQ(q.W, 1.0f);

    q = Quaternion(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(q.X, 1.0f);
    EXPECT_FLOAT_EQ(q.Y, 2.0f);
    EXPECT_FLOAT_EQ(q.Z, 3.0f);
    EXPECT_FLOAT_EQ(q.W, 4.0f);

    Vector3 v(1.0f, 2.0f, 3.0f);
    Quaternion q2(v, 9.0f);
    EXPECT_EQ(q2.GetVectorPart(), v);
    EXPECT_FLOAT_EQ(q2.GetScalarPart(), 9.0f);
}

TEST(QuaternionTest, OperatorAssign_AnotherQuaternion_CopiesValues)
{
    Quaternion a(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion b;
    b = a;
    EXPECT_EQ(a, b);
}

TEST(QuaternionTest, Accessors_AxisAndAngle_ReturnsCorrectValues)
{
    const Vector3 axis = Vector3::sUnitY;
    const float angle = math::PI / 2.0f;
    Quaternion q = Quaternion::FromAxisAngle(axis, angle);

    EXPECT_TRUE(q.GetVectorPart().IsEqualApprox(Vector3(q.X, q.Y, q.Z)));
    EXPECT_NEAR(q.GetScalarPart(), q.W, math::EPSILON_CMP);
    EXPECT_TRUE(q.GetAxis().IsEqualApprox(axis));
    EXPECT_NEAR(q.GetAngle(), angle, math::EPSILON_CMP);

    q.SetVectorPart(axis);
    q.SetScalarPart(angle);
    EXPECT_TRUE(q.GetVectorPart().IsEqualApprox(axis));
    EXPECT_NEAR(q.GetScalarPart(), angle, math::EPSILON_CMP);

    q.SetAxis(axis);
    q.SetAngle(angle);
    EXPECT_TRUE(q.GetAxis().IsEqualApprox(axis));
    EXPECT_NEAR(q.GetAngle(), angle, math::EPSILON_CMP);
}

TEST(QuaternionTest, OperatorAdd_TwoQuaternions_ReturnsSum)
{
    Quaternion a(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion b(4.0f, 5.0f, 6.0f, 7.0f);

    EXPECT_EQ(a + b, Quaternion(5.0f, 7.0f, 9.0f, 11.0f));

    a += b;
    EXPECT_EQ(a, Quaternion(5.0f, 7.0f, 9.0f, 11.0f));
}

TEST(QuaternionTest, OperatorSub_TwoQuaternions_ReturnsDifference)
{
    Quaternion a(5.0f, 7.0f, 9.0f, 11.0f);
    Quaternion b(4.0f, 5.0f, 6.0f, 7.0f);
    EXPECT_EQ(a - b, Quaternion(1.0f, 2.0f, 3.0f, 4.0f));

    a -= b;
    EXPECT_EQ(a, Quaternion(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(QuaternionTest, OperatorMul_IdentitiesAndImaginaryUnits_FollowsHamiltonRules)
{
    Quaternion id;
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_TRUE((id * q).IsEqualApprox(q));
    EXPECT_TRUE((q * id).IsEqualApprox(q));

    id *= q;
    EXPECT_TRUE((id).IsEqualApprox(q));

    // Pure imaginary: (0, 1, 0, 0)
    Quaternion i(1.0f, 0.0f, 0.0f, 0.0f);
    Quaternion j(0.0f, 1.0f, 0.0f, 0.0f);
    Quaternion k = i * j;
    EXPECT_TRUE(k == Quaternion(0.0f, 0.0f, 1.0f, 0.0f));
}

TEST(QuaternionTest, OperatorMul_ScalarValue_ScalesComponents)
{
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_EQ(q * 2.0f, Quaternion(2.0f, 4.0f, 6.0f, 8.0f));
    EXPECT_EQ(2.0f * q, Quaternion(2.0f, 4.0f, 6.0f, 8.0f));

    q *= 2.0f;
    EXPECT_EQ(q, Quaternion(2.0f, 4.0f, 6.0f, 8.0f));
}

TEST(QuaternionTest, OperatorDiv_ScalarValue_ScalesComponents)
{
    Quaternion q(2.0f, 4.0f, 6.0f, 8.0f);

    EXPECT_TRUE((q / 2.0f).IsEqualApprox(Quaternion(2.0f / 2.0f, 4.0f / 2.0f, 6.0f / 2.0f, 8.0f / 2.0f)));

    q /= 2.0f;
    EXPECT_TRUE(q.IsEqualApprox(Quaternion(2.0f / 2.0f, 4.0f / 2.0f, 6.0f / 2.0f, 8.0f / 2.0f)));

    q /= 0.0f;
    EXPECT_TRUE(std::isinf(q.X));
    EXPECT_TRUE(std::isinf(q.Y));
    EXPECT_TRUE(std::isinf(q.Z));
    EXPECT_TRUE(std::isinf(q.W));
}

TEST(QuaternionTest, OperatorUnaryMinus_ValidQuaternion_NegatesAllComponents)
{
    Quaternion q(1.0f, -2.0f, 3.0f, -4.0f);
    EXPECT_EQ(-q, Quaternion(-1.0f, 2.0f, -3.0f, 4.0f));
}

TEST(QuaternionTest, OperatorComparison_EqualQuaternions_ReturnsCorrectBoolean)
{
    Quaternion a(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion b(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(QuaternionTest, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Quaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion q2(1.000001f, 2.000001f, 3.000001f, 4.000001f);
    EXPECT_TRUE(q1.IsEqualApprox(q2));
    EXPECT_FALSE(q1.IsNotEqualApprox(q2));
}

TEST(QuaternionTest, IsPurelyImaginary_VaryingWComponents_ReturnsCorrectBoolean)
{
    Quaternion q1(1.0f, 2.0f, 3.0f, 0.0f);
    Quaternion q2(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion q3(0.0f, 0.0f, 0.0f, 5.0f);

    EXPECT_TRUE(q1.IsPurelyImaginary());
    EXPECT_FALSE(q2.IsPurelyImaginary());
    EXPECT_FALSE(q3.IsPurelyImaginary());
}

TEST(QuaternionTest, IsUnit_UnitAndNonUnitQuaternions_ReturnsCorrectBoolean)
{
    Quaternion q1(1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_TRUE(q1.IsUnit());
    EXPECT_TRUE(q1.IsUnitApprox());

    Quaternion q2(1.0f, 1.0f, 1.0f, 1.0f);
    EXPECT_FALSE(q2.IsUnit());
    EXPECT_FALSE(q2.IsUnitApprox());
}

TEST(QuaternionTest, Dot_TwoQuaternions_ReturnsCorrectScalar)
{
    Quaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion q2(5.0f, 6.0f, 7.0f, 8.0f);
    const float dot = q1.Dot(q2);
    EXPECT_NEAR(dot, 1.0f * 5.0f + 2.0f * 6.0f + 3.0f * 7.0f + 4.0f * 8.0f, math::EPSILON_CMP);
}

TEST(QuaternionTest, Magnitude_TypicalQuaternion_ReturnsLength)
{
    Quaternion q(2.0f, 2.0f, 2.0f, 2.0f);

    EXPECT_NEAR(q.SqrdMagnitude(), 16.0f, math::EPSILON_CMP);
    EXPECT_NEAR(q.Magnitude(), 4.0f, math::EPSILON_CMP);
}

TEST(QuaternionTest, Normalize_NonZeroQuaternion_ResizesToUnitLength)
{
    Quaternion q(2.0f, 2.0f, 2.0f, 2.0f);
    const float invMag = 1.0f / 4.0f;
    const float e = 2.0f * invMag;

    Quaternion n = q.Normalized();
    EXPECT_NEAR(n.Magnitude(), 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(n.X, e, math::EPSILON_CMP);
    EXPECT_NEAR(n.Y, e, math::EPSILON_CMP);
    EXPECT_NEAR(n.Z, e, math::EPSILON_CMP);
    EXPECT_NEAR(n.W, e, math::EPSILON_CMP);

    q.Normalize();
    EXPECT_NEAR(q.Magnitude(), 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(q.X, e, math::EPSILON_CMP);
    EXPECT_NEAR(q.Y, e, math::EPSILON_CMP);
    EXPECT_NEAR(q.Z, e, math::EPSILON_CMP);
    EXPECT_NEAR(q.W, e, math::EPSILON_CMP);
}

TEST(QuaternionTest, Normalize_ZeroQuaternion_ResultsInNaN)
{
    Quaternion v(0.0f, 0.0f, 0.0f, 0.0f);
    Quaternion n = v.Normalized();

    EXPECT_TRUE(std::isnan(n.X));
    EXPECT_TRUE(std::isnan(n.Y));
    EXPECT_TRUE(std::isnan(n.Z));
    EXPECT_TRUE(std::isnan(n.W));

    v.Normalize();
    EXPECT_TRUE(std::isnan(v.X));
    EXPECT_TRUE(std::isnan(v.Y));
    EXPECT_TRUE(std::isnan(v.Z));
    EXPECT_TRUE(std::isnan(v.W));
}

TEST(QuaternionTest, Conjugate_ValidQuaternion_FlipsVectorPart)
{
    Quaternion q = Quaternion::FromAxisAngle(Vector3::sUnitY, math::PI / 4.0f);
    Quaternion conj = q.Conjugated();

    EXPECT_NEAR(conj.X, -q.X, math::EPSILON_CMP);
    EXPECT_NEAR(conj.Y, -q.Y, math::EPSILON_CMP);
    EXPECT_NEAR(conj.Z, -q.Z, math::EPSILON_CMP);
    EXPECT_NEAR(conj.W, q.W, math::EPSILON_CMP);

    conj = q;
    conj.Conjugate();
    EXPECT_NEAR(conj.X, -q.X, math::EPSILON_CMP);
    EXPECT_NEAR(conj.Y, -q.Y, math::EPSILON_CMP);
    EXPECT_NEAR(conj.Z, -q.Z, math::EPSILON_CMP);
    EXPECT_NEAR(conj.W, q.W, math::EPSILON_CMP);
}

TEST(QuaternionTest, Invert_UnitAndNonUnitQuaternions_ReturnsInverse)
{
    Quaternion q = Quaternion::FromAxisAngle(Vector3::sUnitY, math::PI / 4.0f);
    q.Normalize();

    Quaternion conj = q.Conjugated();
    Quaternion inv = q.Inverse();

    // only true if |q| == 1
    EXPECT_TRUE(inv.IsEqualApprox(conj));

    Quaternion id = q * inv;
    EXPECT_TRUE(id.IsUnitApprox());
    EXPECT_NEAR(id.W, 1.0f, math::EPSILON_CMP);

    inv = q;
    inv.Invert();
    EXPECT_TRUE(inv.IsEqualApprox(conj));
    id = q * inv;
    EXPECT_TRUE(id.IsUnitApprox());
    EXPECT_NEAR(id.W, 1.0f, math::EPSILON_CMP);
}

TEST(QuaternionTest, Transform_VectorByQuaternion_RotatesCorrectly)
{
    Quaternion q = Quaternion::FromAxisAngle(Vector3::sUnitZ, math::PI / 2.0f);
    Vector3 v(1.0f, 0.0f, 0.0f);

    Vector3 rotated = q.Transform(v);
    EXPECT_NEAR(rotated.X, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(rotated.Y, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(rotated.Z, 0.0f, math::EPSILON_CMP);

    Vector3 inv = q.InvTransform(rotated);
    EXPECT_TRUE(inv.IsEqualApprox(v));
}

TEST(QuaternionTest, romAxisAngle_AxisAndAngle_CreatesUnitQuaternion)
{
    Vector3 axis = Vector3::sUnitY;
    Quaternion q = Quaternion::FromAxisAngle(axis, math::PI / 2.0f);
    EXPECT_TRUE(q.IsUnitApprox());

    Vector3 v(1.0f, 0.0f, 0.0f);
    Vector3 rotated = q.Transform(v);

    EXPECT_NEAR(rotated.X, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(rotated.Y, 0.0f, math::EPSILON_CMP);
    EXPECT_NEAR(rotated.Z, -1.0f, math::EPSILON_CMP);
}

TEST(QuaternionTest, FromEuler_DifferentRotationOrders_MatchesCompositeMultiplication)
{
    const float pitch = math::DegToRad(10.0f);
    const float yaw = math::DegToRad(20.0f);
    const float roll = math::DegToRad(30.0f);

    Quaternion qx = Quaternion::FromAxisAngle(Vector3::sUnitX, pitch);
    Quaternion qy = Quaternion::FromAxisAngle(Vector3::sUnitY, yaw);
    Quaternion qz = Quaternion::FromAxisAngle(Vector3::sUnitZ, roll);

    Quaternion expected_xyz = qx * qy * qz;
    Quaternion expected_yxz = qy * qx * qz;
    Quaternion expected_zyx = qz * qy * qx;
    Quaternion expected_xzy = qx * qz * qy;
    Quaternion expected_yzx = qy * qz * qx;
    Quaternion expected_zxy = qz * qx * qy;

    Quaternion test_yxz = Quaternion::FromEuler(pitch, yaw, roll, math::eEulerOrder::YXZ);
    Quaternion test_xyz = Quaternion::FromEuler(pitch, yaw, roll, math::eEulerOrder::XYZ);
    Quaternion test_zyx = Quaternion::FromEuler(pitch, yaw, roll, math::eEulerOrder::ZYX);
    Quaternion test_xzy = Quaternion::FromEuler(pitch, yaw, roll, math::eEulerOrder::XZY);
    Quaternion test_yzx = Quaternion::FromEuler(pitch, yaw, roll, math::eEulerOrder::YZX);
    Quaternion test_zxy = Quaternion::FromEuler(pitch, yaw, roll, math::eEulerOrder::ZXY);

    EXPECT_TRUE(test_yxz.IsEqualApprox(expected_yxz));
    EXPECT_TRUE(test_xyz.IsEqualApprox(expected_xyz));
    EXPECT_TRUE(test_zyx.IsEqualApprox(expected_zyx));
    EXPECT_TRUE(test_xzy.IsEqualApprox(expected_xzy));
    EXPECT_TRUE(test_yzx.IsEqualApprox(expected_yzx));
    EXPECT_TRUE(test_zxy.IsEqualApprox(expected_zxy));
}
