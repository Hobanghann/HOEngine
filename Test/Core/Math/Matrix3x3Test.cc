#include "Core/Math/Matrix3x3.h"

#include <gtest/gtest.h>
#include <limits>

#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector3.h"

using namespace ho;

class Matrix3x3Test : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        zero = Matrix3x3();
        eye = Matrix3x3(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

        mat19 = Matrix3x3(Vector3(1.0f, 4.0f, 7.0f), Vector3(2.0f, 5.0f, 8.0f), Vector3(3.0f, 6.0f, 9.0f));

        mat91 = Matrix3x3(Vector3(9.0f, 6.0f, 3.0f), Vector3(8.0f, 5.0f, 2.0f), Vector3(7.0f, 4.0f, 1.0f));

        const float angle = math::PI / 4.0f;
        const float s = std::sin(angle);
        const float c = std::cos(angle);

        matPitch = Matrix3x3(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, c, s), Vector3(0.0f, -s, c));

        matYaw = Matrix3x3(Vector3(c, 0.0f, -s), Vector3(0.0f, 1.0f, 0.0f), Vector3(s, 0.0f, c));

        matRoll = Matrix3x3(Vector3(c, s, 0.0f), Vector3(-s, c, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

        matOrthogonal =
            Matrix3x3(Vector3(2.0f * c, 2.0f * s, 0.0f), Vector3(-3.0f * s, 3.0f * c, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

        matNonSingular = Matrix3x3(Vector3(1.0f, 2.0f, 3.0f), Vector3(0.0f, 2.0f, 4.0f), Vector3(5.0f, 6.0f, 0.0f));
    }

    Matrix3x3 zero, eye, mat19, mat91, matPitch, matYaw, matRoll, matOrthogonal, matNonSingular;
};

TEST_F(Matrix3x3Test, Constructor_DefaultAndAllParameters_InitializeCorrectly)
{
    EXPECT_EQ(zero.Row0, Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(zero.Row1, Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(zero.Row2, Vector3(0.0f, 0.0f, 0.0f));

    EXPECT_EQ(mat19.Row0, Vector3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(mat19.Row1, Vector3(4.0f, 5.0f, 6.0f));
    EXPECT_EQ(mat19.Row2, Vector3(7.0f, 8.0f, 9.0f));
}

TEST_F(Matrix3x3Test, StaticConstant_PredefinedMatrix_MatchesExpectedValues)
{
    EXPECT_EQ(Matrix3x3::sIdentity, eye);
}

TEST_F(Matrix3x3Test, OperatorAssign_AnotherMatrix_CopiesValues)
{
    zero = mat19;
    EXPECT_EQ(zero, mat19);
}

TEST_F(Matrix3x3Test, FromAxisAngle_AxisAndAngle_ReturnsRotationMatrix)
{
    Vector3 axis(0.0f, 0.0f, 1.0f);
    const float angle = math::PI / 4.0f;
    Matrix3x3 rot = Matrix3x3::FromAxisAngle(axis, angle);
    EXPECT_TRUE(rot.IsEqualApprox(matRoll));
}

TEST_F(Matrix3x3Test, FromEuler_VariousEulerOrders_MatchesCompositeRotation)
{
    const float angle = math::PI / 4.0f;
    EXPECT_TRUE(
        Matrix3x3::FromEuler(angle, angle, angle, math::eEulerOrder::XYZ).IsEqualApprox(matRoll * matYaw * matPitch));
    EXPECT_TRUE(
        Matrix3x3::FromEuler(angle, angle, angle, math::eEulerOrder::XZY).IsEqualApprox(matYaw * matRoll * matPitch));
    EXPECT_TRUE(
        Matrix3x3::FromEuler(angle, angle, angle, math::eEulerOrder::YZX).IsEqualApprox(matPitch * matRoll * matYaw));
    EXPECT_TRUE(
        Matrix3x3::FromEuler(angle, angle, angle, math::eEulerOrder::YXZ).IsEqualApprox(matRoll * matPitch * matYaw));
    EXPECT_TRUE(
        Matrix3x3::FromEuler(angle, angle, angle, math::eEulerOrder::ZXY).IsEqualApprox(matYaw * matPitch * matRoll));
    EXPECT_TRUE(
        Matrix3x3::FromEuler(angle, angle, angle, math::eEulerOrder::ZYX).IsEqualApprox(matPitch * matYaw * matRoll));
}

TEST_F(Matrix3x3Test, FromQuaternion_ValidQuaternion_ReturnsRotationMatrix)
{
    Quaternion q = Quaternion::FromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), math::PI / 4.0f);
    Matrix3x3 rot = Matrix3x3::FromQuaternion(q);
    EXPECT_TRUE(rot.IsEqualApprox(matRoll));
}

TEST_F(Matrix3x3Test, GetAndSetCol_ValidVectors_UpdatesAndReturnsColumns)
{
    EXPECT_EQ(mat19.GetCol0(), Vector3(1.0f, 4.0f, 7.0f));
    EXPECT_EQ(mat19.GetCol1(), Vector3(2.0f, 5.0f, 8.0f));
    EXPECT_EQ(mat19.GetCol2(), Vector3(3.0f, 6.0f, 9.0f));

    mat19.SetCol0(Vector3(10.0f, 40.0f, 70.0f));
    mat19.SetCol1(Vector3(20.0f, 50.0f, 80.0f));
    mat19.SetCol2(Vector3(30.0f, 60.0f, 90.0f));

    EXPECT_EQ(mat19.GetCol0(), Vector3(10.0f, 40.0f, 70.0f));
    EXPECT_EQ(mat19.GetCol1(), Vector3(20.0f, 50.0f, 80.0f));
    EXPECT_EQ(mat19.GetCol2(), Vector3(30.0f, 60.0f, 90.0f));
}

TEST_F(Matrix3x3Test, OperatorAdd_TwoMatrices_ReturnsMatrixSum)
{
    Matrix3x3 sum = mat19 + mat91;
    EXPECT_EQ(sum.Row0, Vector3(10.0f, 10.0f, 10.0f));
    EXPECT_EQ(sum.Row1, Vector3(10.0f, 10.0f, 10.0f));
    EXPECT_EQ(sum.Row2, Vector3(10.0f, 10.0f, 10.0f));

    mat19 += mat91;
    EXPECT_EQ(mat19, sum);
}

TEST_F(Matrix3x3Test, OperatorSub_TwoMatrices_ReturnsMatrixDifference)
{
    Matrix3x3 sub = mat91 - mat19;
    EXPECT_EQ(sub.Row0, Vector3(8.0f, 6.0f, 4.0f));
    EXPECT_EQ(sub.Row1, Vector3(2.0f, 0.0f, -2.0f));
    EXPECT_EQ(sub.Row2, Vector3(-4.0f, -6.0f, -8.0f));

    mat91 -= mat19;
    EXPECT_EQ(mat91, sub);
}

TEST_F(Matrix3x3Test, OperatorMul_TwoMatrices_ReturnsMatrixProduct)
{
    Matrix3x3 expected(Vector3(30.0f, 84.0f, 138.0f), Vector3(24.0f, 69.0f, 114.0f), Vector3(18.0f, 54.0f, 90.0f));
    EXPECT_EQ(mat19 * mat91, expected);

    mat19 *= mat91;
    EXPECT_EQ(mat19, expected);
}

TEST_F(Matrix3x3Test, OperatorMul_MatrixAndVector_ReturnsTransformedVector)
{
    Vector3 v(1.0f, 1.0f, 1.0f);

    // Row0·v = 1*1 + 2*1 + 3*1 = 6
    // Row1·v = 4*1 + 5*1 + 6*1 = 15
    // Row2·v = 7*1 + 8*1 + 9*1 = 24
    Vector3 result = mat19 * v;

    EXPECT_NEAR(result.X, 6.0f, math::EPSILON_CMP);
    EXPECT_NEAR(result.Y, 15.0f, math::EPSILON_CMP);
    EXPECT_NEAR(result.Z, 24.0f, math::EPSILON_CMP);

    Vector3 v2(1.2f, -3.4f, 5.6f);
    EXPECT_TRUE((Matrix3x3::sIdentity * v2).IsEqualApprox(v2));

    EXPECT_EQ(zero * v2, Vector3::sZero);
}

TEST_F(Matrix3x3Test, OperatorMul_ScalarValue_ScalesAllElements)
{
    Matrix3x3 mul = mat19 * 2.0f;
    EXPECT_EQ(mul.Row0, Vector3(2.0f, 4.0f, 6.0f));
    EXPECT_EQ(mul.Row1, Vector3(8.0f, 10.0f, 12.0f));
    EXPECT_EQ(mul.Row2, Vector3(14.0f, 16.0f, 18.0f));

    Matrix3x3 l_mul = 2.0f * mat19;
    EXPECT_EQ(l_mul, mul);

    mat19 *= 2.0f;
    EXPECT_EQ(mat19, mul);
}

TEST_F(Matrix3x3Test, OperatorDiv_ScalarValue_ReturnsQuotientOrInfinityOnZero)
{
    Matrix3x3 div = mat19 / 2.0f;
    EXPECT_EQ(div.Row0, Vector3(1.0f / 2.0f, 2.0f / 2.0f, 3.0f / 2.0f));
    EXPECT_EQ(div.Row1, Vector3(4.0f / 2.0f, 5.0f / 2.0f, 6.0f / 2.0f));
    EXPECT_EQ(div.Row2, Vector3(7.0f / 2.0f, 8.0f / 2.0f, 9.0f / 2.0f));

    mat19 /= 2.0f;
    EXPECT_EQ(mat19, div);

    mat19 /= 0.0f;
    EXPECT_TRUE(std::isinf(mat19.Row0.X));
    EXPECT_TRUE(std::isinf(mat19.Row0.Y));
    EXPECT_TRUE(std::isinf(mat19.Row0.Z));
    EXPECT_TRUE(std::isinf(mat19.Row1.X));
    EXPECT_TRUE(std::isinf(mat19.Row1.Y));
    EXPECT_TRUE(std::isinf(mat19.Row1.Z));
    EXPECT_TRUE(std::isinf(mat19.Row2.X));
    EXPECT_TRUE(std::isinf(mat19.Row2.Y));
    EXPECT_TRUE(std::isinf(mat19.Row2.Z));
}

TEST_F(Matrix3x3Test, OperatorComparison_DifferentMatrices_ReturnsCorrectBoolean)
{
    Matrix3x3 a(Vector3(1.0f, 4.0f, 7.0f), Vector3(2.0f, 5.0f, 8.0f), Vector3(3.0f, 6.0f, 9.0f));

    EXPECT_TRUE(mat19 == a);
    EXPECT_FALSE(mat19 != a);
    EXPECT_TRUE(mat19 != mat91);
}

TEST_F(Matrix3x3Test, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Matrix3x3 near_19(Vector3(1.000001f, 4.000001f, 7.000001f),
                      Vector3(2.000001f, 5.000001f, 8.000001f),
                      Vector3(3.000001f, 6.000001f, 9.000001f));

    EXPECT_TRUE(mat19.IsEqualApprox(near_19));
    EXPECT_FALSE(mat19.IsNotEqualApprox(near_19));
}

TEST_F(Matrix3x3Test, IsOrthogonal_VariousMatrices_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(matOrthogonal.IsOrthogonal());
    EXPECT_FALSE(mat19.IsOrthogonal());
}

TEST_F(Matrix3x3Test, Orthogonalize_NonOrthogonalMatrix_MakesMatrixOrthogonal)
{
    Matrix3x3 o = matNonSingular.Orthogonalized();
    EXPECT_TRUE(o.IsOrthogonal());
    EXPECT_FALSE(matNonSingular.IsOrthogonal());
    EXPECT_NEAR(o.GetCol0().Magnitude(), matNonSingular.GetCol0().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol1().Magnitude(), matNonSingular.GetCol1().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol2().Magnitude(), matNonSingular.GetCol2().Magnitude(), math::EPSILON_CMP);

    matNonSingular.Orthogonalize();
    EXPECT_TRUE(o.IsOrthogonal());
    EXPECT_NEAR(o.GetCol0().Magnitude(), matNonSingular.GetCol0().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol1().Magnitude(), matNonSingular.GetCol1().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol2().Magnitude(), matNonSingular.GetCol2().Magnitude(), math::EPSILON_CMP);
}

TEST_F(Matrix3x3Test, IsOrthonormal_VariousMatrices_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(matYaw.IsOrthonormal());
    EXPECT_FALSE(mat19.IsOrthonormal());
}

TEST_F(Matrix3x3Test, Orthonormalize_NonOrthonormalMatrix_MakesMatrixOrthonormal)
{
    Matrix3x3 n = matNonSingular.Orthonormalized();
    EXPECT_TRUE(n.IsOrthonormal());
    EXPECT_FALSE(mat19.IsOrthonormal());

    matNonSingular.Orthonormalize();
    EXPECT_TRUE(matNonSingular.IsOrthonormal());
}

TEST_F(Matrix3x3Test, Transposed_ValidMatrix_SwapsRowsAndColumns)
{
    Matrix3x3 t = mat19.Transposed();
    EXPECT_EQ(t.Row0, Vector3(1.0f, 4.0f, 7.0f));
    EXPECT_EQ(t.Row1, Vector3(2.0f, 5.0f, 8.0f));
    EXPECT_EQ(t.Row2, Vector3(3.0f, 6.0f, 9.0f));

    mat19.Transpose();
    EXPECT_EQ(mat19, t);
}

TEST_F(Matrix3x3Test, Inverse_InvertibleAndSingularMatrices_ReturnsInverseOrInfinity)
{
    Matrix3x3 m(Vector3(1.0f, 0.0f, 5.0f), Vector3(2.0f, 1.0f, 6.0f), Vector3(3.0f, 4.0f, 0.0f));

    Matrix3x3 inv = m.Inverse();
    Matrix3x3 id = m * inv;
    EXPECT_TRUE(id.IsEqualApprox(Matrix3x3::sIdentity));

    inv = m;
    inv.Invert();
    id = m * inv;
    EXPECT_TRUE(id.IsEqualApprox(Matrix3x3::sIdentity));

    Matrix3x3 singular(Vector3(1.0f, 2.0f, 3.0f), Vector3(2.0f, 4.0f, 6.0f), Vector3(3.0f, 6.0f, 9.0f));
    Matrix3x3 invSingular = singular.Inverse();
    EXPECT_TRUE(std::isnan(invSingular.Row0.X));
    EXPECT_TRUE(std::isnan(invSingular.Row1.Y));
    EXPECT_TRUE(std::isnan(invSingular.Row2.Z));
}

TEST_F(Matrix3x3Test, Trace_ValidMatrix_ReturnsSumOfDiagonalElements)
{
    EXPECT_FLOAT_EQ(mat19.Trace(), 1.0f + 5.0f + 9.0f);
}

TEST_F(Matrix3x3Test, Determinant_ValidMatrix_ReturnsCorrectScalarValue)
{
    EXPECT_FLOAT_EQ(mat19.Determinant(), 0.0f);
    Matrix3x3 m(Vector3(1.0f, 0.0f, 5.0f), Vector3(2.0f, 1.0f, 6.0f), Vector3(3.0f, 4.0f, 0.0f));
    EXPECT_NEAR(m.Determinant(), 1.0f, math::EPSILON_CMP);
}
