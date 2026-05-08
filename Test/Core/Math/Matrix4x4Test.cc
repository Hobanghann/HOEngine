#include "Core/Math/Matrix4x4.h"

#include <gtest/gtest.h>
#include <limits>

#include "Core/Math/Matrix3x3.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector4.h"

using namespace ho;

class Matrix4x4Test : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        zero = Matrix4x4();
        eye = Matrix4x4(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                        Vector4(0.0f, 1.0f, 0.0f, 0.0f),
                        Vector4(0.0f, 0.0f, 1.0f, 0.0f),
                        Vector4(0.0f, 0.0f, 0.0f, 1.0f));

        mat116 = Matrix4x4(Vector4(1.0f, 5.0f, 9.0f, 13.0f),
                           Vector4(2.0f, 6.0f, 10.0f, 14.0f),
                           Vector4(3.0f, 7.0f, 11.0f, 15.0f),
                           Vector4(4.0f, 8.0f, 12.0f, 16.0f));

        mat161 = Matrix4x4(Vector4(16.0f, 12.0f, 8.0f, 4.0f),
                           Vector4(15.0f, 11.0f, 7.0f, 3.0f),
                           Vector4(14.0f, 10.0f, 6.0f, 2.0f),
                           Vector4(13.0f, 9.0f, 5.0f, 1.0f));

        const float angle = math::PI / 4.0f;
        const float s = std::sin(angle);
        const float c = std::cos(angle);
        matOrthonormal = Matrix4x4(Vector4(c, -s, 0.0f, 0.0f),
                                   Vector4(s, c, 0.0f, 0.0f),
                                   Vector4(0.0f, 0.0f, 1.0f, 0.0f),
                                   Vector4(0.0f, 0.0f, 0.0f, 1.0f));
        matOrthogonal = Matrix4x4(Vector4(2.0f * c, -2.0f * s, 0.0f, 0.0f),
                                  Vector4(3.0f * s, 3.0f * c, 0.0f, 0.0f),
                                  Vector4(0.0f, 0.0f, 1.0f, 0.0f),
                                  Vector4(0.0f, 0.0f, 0.0f, 1.0f));
        matNonSingular = Matrix4x4(Vector4(1.0f, 2.0f, 3.0f, 4.0f),
                                   Vector4(5.0f, 6.0f, 7.0f, 8.0f),
                                   Vector4(2.0f, 6.0f, 4.0f, 8.0f),
                                   Vector4(3.0f, 1.0f, 1.0f, 2.0f));
    }

    Matrix4x4 zero, eye, mat116, mat161, matOrthogonal, matOrthonormal, matNonSingular;
};

TEST_F(Matrix4x4Test, Constructor_DefaultAndAllParameters_InitializeCorrectly)
{
    EXPECT_EQ(zero.Row0, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    EXPECT_EQ(zero.Row1, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    EXPECT_EQ(zero.Row2, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    EXPECT_EQ(zero.Row3, Vector4(0.0f, 0.0f, 0.0f, 0.0f));

    EXPECT_EQ(mat116.Row0, Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(mat116.Row1, Vector4(5.0f, 6.0f, 7.0f, 8.0f));
    EXPECT_EQ(mat116.Row2, Vector4(9.0f, 10.0f, 11.0f, 12.0f));
    EXPECT_EQ(mat116.Row3, Vector4(13.0f, 14.0f, 15.0f, 16.0f));
}

TEST_F(Matrix4x4Test, StaticConstant_PredefinedMatrix_MatchesExpectedValues)
{
    EXPECT_EQ(Matrix4x4::sIdentity, eye);
}

TEST_F(Matrix4x4Test, OperatorAssign_AnotherMatrix_CopiesValues)
{
    zero = mat116;
    EXPECT_EQ(zero, mat116);
}

TEST_F(Matrix4x4Test, GetAndSetCol_ValidVectors_UpdatesAndReturnsColumns)
{
    EXPECT_EQ(mat116.GetCol0(), Vector4(1.0f, 5.0f, 9.0f, 13.0f));
    EXPECT_EQ(mat116.GetCol1(), Vector4(2.0f, 6.0f, 10.0f, 14.0f));
    EXPECT_EQ(mat116.GetCol2(), Vector4(3.0f, 7.0f, 11.0f, 15.0f));
    EXPECT_EQ(mat116.GetCol3(), Vector4(4.0f, 8.0f, 12.0f, 16.0f));

    mat116.SetCol0(Vector4(10.0f, 50.0f, 90.0f, 130.0f));
    mat116.SetCol1(Vector4(20.0f, 60.0f, 100.0f, 140.0f));
    mat116.SetCol2(Vector4(30.0f, 70.0f, 110.0f, 150.0f));
    mat116.SetCol3(Vector4(40.0f, 80.0f, 120.0f, 160.0f));
    EXPECT_EQ(mat116.GetCol0(), Vector4(10.0f, 50.0f, 90.0f, 130.0f));
    EXPECT_EQ(mat116.GetCol1(), Vector4(20.0f, 60.0f, 100.0f, 140.0f));
    EXPECT_EQ(mat116.GetCol2(), Vector4(30.0f, 70.0f, 110.0f, 150.0f));
    EXPECT_EQ(mat116.GetCol3(), Vector4(40.0f, 80.0f, 120.0f, 160.0f));
}

TEST_F(Matrix4x4Test, OperatorAdd_TwoMatrices_ReturnsMatrixSum)
{
    Matrix4x4 sum = mat116 + mat161;
    EXPECT_EQ(sum.Row0, Vector4(17.0f, 17.0f, 17.0f, 17.0f));
    EXPECT_EQ(sum.Row1, Vector4(17.0f, 17.0f, 17.0f, 17.0f));
    EXPECT_EQ(sum.Row2, Vector4(17.0f, 17.0f, 17.0f, 17.0f));
    EXPECT_EQ(sum.Row3, Vector4(17.0f, 17.0f, 17.0f, 17.0f));

    mat116 += mat161;
    EXPECT_EQ(mat116.Row0, Vector4(17.0f, 17.0f, 17.0f, 17.0f));
    EXPECT_EQ(mat116.Row1, Vector4(17.0f, 17.0f, 17.0f, 17.0f));
    EXPECT_EQ(mat116.Row2, Vector4(17.0f, 17.0f, 17.0f, 17.0f));
    EXPECT_EQ(mat116.Row3, Vector4(17.0f, 17.0f, 17.0f, 17.0f));
}

TEST_F(Matrix4x4Test, OperatorSub_TwoMatrices_ReturnsMatrixDifference)
{
    Matrix4x4 sub = mat161 - mat116;
    EXPECT_EQ(sub.Row0, Vector4(15.0f, 13.0f, 11.0f, 9.0f));
    EXPECT_EQ(sub.Row1, Vector4(7.0f, 5.0f, 3.0f, 1.0f));
    EXPECT_EQ(sub.Row2, Vector4(-1.0f, -3.0f, -5.0f, -7.0f));
    EXPECT_EQ(sub.Row3, Vector4(-9.0f, -11.0f, -13.0f, -15.0f));

    mat161 -= mat116;
    EXPECT_EQ(mat161.Row0, Vector4(15.0f, 13.0f, 11.0f, 9.0f));
    EXPECT_EQ(mat161.Row1, Vector4(7.0f, 5.0f, 3.0f, 1.0f));
    EXPECT_EQ(mat161.Row2, Vector4(-1.0f, -3.0f, -5.0f, -7.0f));
    EXPECT_EQ(mat161.Row3, Vector4(-9.0f, -11.0f, -13.0f, -15.0f));
}

TEST_F(Matrix4x4Test, OperatorMul_TwoMatrices_ReturnsMatrixProduct)
{
    Matrix4x4 expected(Vector4(80.0f, 240.0f, 400.0f, 560.0f),
                       Vector4(70.0f, 214.0f, 358.0f, 502.0f),
                       Vector4(60.0f, 188.0f, 316.0f, 444.0f),
                       Vector4(50.0f, 162.0f, 274.0f, 386.0f));
    EXPECT_EQ(mat116 * mat161, expected);

    mat116 *= mat161;
    EXPECT_EQ(mat116, expected);
}

TEST_F(Matrix4x4Test, OperatorMul_MatrixAndVector_ReturnsTransformedVector)
{
    Vector4 v(1.0f, 1.0f, 1.0f, 1.0f);

    Vector4 result = mat116 * v;

    EXPECT_NEAR(result.X, 10.0f, math::EPSILON_CMP);
    EXPECT_NEAR(result.Y, 26.0f, math::EPSILON_CMP);
    EXPECT_NEAR(result.Z, 42.0f, math::EPSILON_CMP);
    EXPECT_NEAR(result.W, 58.0f, math::EPSILON_CMP);

    Vector4 v2(1.2f, -3.4f, 5.6f, 7.8f);
    EXPECT_TRUE((Matrix4x4::sIdentity * v2).IsEqualApprox(v2));

    EXPECT_EQ(zero * v2, Vector4::sZero);
}

TEST_F(Matrix4x4Test, OperatorMul_ScalarValue_ScalesAllElements)
{
    Matrix4x4 m = mat116 * 2.0f;
    EXPECT_EQ(m.Row0, Vector4(2.0f, 4.0f, 6.0f, 8.0f));
    EXPECT_EQ(m.Row1, Vector4(10.0f, 12.0f, 14.0f, 16.0f));
    EXPECT_EQ(m.Row2, Vector4(18.0f, 20.0f, 22.0f, 24.0f));
    EXPECT_EQ(m.Row3, Vector4(26.0f, 28.0f, 30.0f, 32.0f));

    mat116 *= 2.0f;
    EXPECT_EQ(mat116.Row0, Vector4(2.0f, 4.0f, 6.0f, 8.0f));
    EXPECT_EQ(mat116.Row1, Vector4(10.0f, 12.0f, 14.0f, 16.0f));
    EXPECT_EQ(mat116.Row2, Vector4(18.0f, 20.0f, 22.0f, 24.0f));
    EXPECT_EQ(mat116.Row3, Vector4(26.0f, 28.0f, 30.0f, 32.0f));
}

TEST_F(Matrix4x4Test, OperatorDiv_ScalarValue_ReturnsQuotientOrInfinityOnZero)
{
    Matrix4x4 div = mat116 / 2.0f;
    EXPECT_EQ(div.Row0, Vector4(1.0f / 2.0f, 2.0f / 2.0f, 3.0f / 2.0f, 4.0f / 2.0f));
    EXPECT_EQ(div.Row1, Vector4(5.0f / 2.0f, 6.0f / 2.0f, 7.0f / 2.0f, 8.0f / 2.0f));
    EXPECT_EQ(div.Row2, Vector4(9.0f / 2.0f, 10.0f / 2.0f, 11.0f / 2.0f, 12.0f / 2.0f));
    EXPECT_EQ(div.Row3, Vector4(13.0f / 2.0f, 14.0f / 2.0f, 15.0f / 2.0f, 16.0f / 2.0f));

    mat116 /= 2.0f;
    EXPECT_EQ(mat116.Row0, Vector4(1.0f / 2.0f, 2.0f / 2.0f, 3.0f / 2.0f, 4.0f / 2.0f));
    EXPECT_EQ(mat116.Row1, Vector4(5.0f / 2.0f, 6.0f / 2.0f, 7.0f / 2.0f, 8.0f / 2.0f));
    EXPECT_EQ(mat116.Row2, Vector4(9.0f / 2.0f, 10.0f / 2.0f, 11.0f / 2.0f, 12.0f / 2.0f));
    EXPECT_EQ(mat116.Row3, Vector4(13.0f / 2.0f, 14.0f / 2.0f, 15.0f / 2.0f, 16.0f / 2.0f));

    mat116 /= 0.0f;
    EXPECT_TRUE(std::isinf(mat116.Row0.X));
    EXPECT_TRUE(std::isinf(mat116.Row0.Y));
    EXPECT_TRUE(std::isinf(mat116.Row0.Z));
    EXPECT_TRUE(std::isinf(mat116.Row0.W));
    EXPECT_TRUE(std::isinf(mat116.Row1.X));
    EXPECT_TRUE(std::isinf(mat116.Row1.Y));
    EXPECT_TRUE(std::isinf(mat116.Row1.Z));
    EXPECT_TRUE(std::isinf(mat116.Row1.W));
    EXPECT_TRUE(std::isinf(mat116.Row2.X));
    EXPECT_TRUE(std::isinf(mat116.Row2.Y));
    EXPECT_TRUE(std::isinf(mat116.Row2.Z));
    EXPECT_TRUE(std::isinf(mat116.Row2.W));
    EXPECT_TRUE(std::isinf(mat116.Row3.X));
    EXPECT_TRUE(std::isinf(mat116.Row3.Y));
    EXPECT_TRUE(std::isinf(mat116.Row3.Z));
    EXPECT_TRUE(std::isinf(mat116.Row3.W));
}

TEST_F(Matrix4x4Test, OperatorComparison_DifferentMatrices_ReturnsCorrectBoolean)
{
    Matrix4x4 a(Vector4(1.0f, 5.0f, 9.0f, 13.0f),
                Vector4(2.0f, 6.0f, 10.0f, 14.0f),
                Vector4(3.0f, 7.0f, 11.0f, 15.0f),
                Vector4(4.0f, 8.0f, 12.0f, 16.0f));
    EXPECT_TRUE(mat116 == a);
    EXPECT_FALSE(mat116 != a);
    EXPECT_TRUE(mat116 != mat161);
}

TEST_F(Matrix4x4Test, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Matrix4x4 near_116(Vector4(1.000001f, 5.000001f, 9.000001f, 13.000001f),
                       Vector4(2.000001f, 6.000001f, 10.000001f, 14.000001f),
                       Vector4(3.000001f, 7.000001f, 11.000001f, 15.000001f),
                       Vector4(4.000001f, 8.000001f, 12.000001f, 16.000001f));

    EXPECT_TRUE(mat116.IsEqualApprox(near_116));
    EXPECT_FALSE(mat116.IsNotEqualApprox(near_116));
}

TEST_F(Matrix4x4Test, IsOrthogonal_VariousMatrices_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(matOrthogonal.IsOrthogonal());
    EXPECT_FALSE(mat116.IsOrthogonal());
}

TEST_F(Matrix4x4Test, Orthogonalize_NonOrthogonalMatrix_MakesMatrixOrthogonal)
{
    Matrix4x4 o = matNonSingular.Orthogonalized();
    EXPECT_TRUE(o.IsOrthogonal());
    EXPECT_FALSE(matNonSingular.IsOrthogonal());
    EXPECT_NEAR(o.GetCol0().Magnitude(), matNonSingular.GetCol0().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol1().Magnitude(), matNonSingular.GetCol1().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol2().Magnitude(), matNonSingular.GetCol2().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol3().Magnitude(), matNonSingular.GetCol3().Magnitude(), math::EPSILON_CMP);

    matNonSingular.Orthogonalize();
    EXPECT_TRUE(o.IsOrthogonal());
    EXPECT_NEAR(o.GetCol0().Magnitude(), matNonSingular.GetCol0().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol1().Magnitude(), matNonSingular.GetCol1().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol2().Magnitude(), matNonSingular.GetCol2().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol3().Magnitude(), matNonSingular.GetCol3().Magnitude(), math::EPSILON_CMP);
}

TEST_F(Matrix4x4Test, IsOrthonormal_VariousMatrices_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(matOrthonormal.IsOrthonormal());
    EXPECT_FALSE(mat116.IsOrthonormal());
}

TEST_F(Matrix4x4Test, Orthonormalize_NonOrthonormalMatrix_MakesMatrixOrthonormal)
{
    Matrix4x4 n = matNonSingular.Orthonormalized();
    EXPECT_TRUE(n.IsOrthonormal());
    EXPECT_FALSE(matNonSingular.IsOrthonormal());

    matNonSingular.Orthonormalize();
    EXPECT_TRUE(matNonSingular.IsOrthonormal());
}

TEST_F(Matrix4x4Test, Transposed_ValidMatrix_SwapsRowsAndColumns)
{
    Matrix4x4 t = mat116.Transposed();
    EXPECT_EQ(t.Row0, Vector4(1.0f, 5.0f, 9.0f, 13.0f));
    EXPECT_EQ(t.Row1, Vector4(2.0f, 6.0f, 10.0f, 14.0f));
    EXPECT_EQ(t.Row2, Vector4(3.0f, 7.0f, 11.0f, 15.0f));
    EXPECT_EQ(t.Row3, Vector4(4.0f, 8.0f, 12.0f, 16.0f));

    mat116.Transpose();
    EXPECT_EQ(mat116, t);
}

TEST_F(Matrix4x4Test, Inverse_InvertibleAndSingularMatrices_ReturnsInverseOrInfinity)
{
    Matrix4x4 m(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                Vector4(0.0f, 2.0f, 0.0f, 0.0f),
                Vector4(0.0f, 0.0f, 3.0f, 0.0f),
                Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    Matrix4x4 expected(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                       Vector4(0.0f, 0.5f, 0.0f, 0.0f),
                       Vector4(0.0f, 0.0f, 1.0f / 3.0f, 0.0f),
                       Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    Matrix4x4 inv = m.Inverse();
    EXPECT_TRUE(inv.IsEqualApprox(expected));
    Matrix4x4 id = m * inv;
    EXPECT_TRUE(id.IsEqualApprox(Matrix4x4::sIdentity));

    inv = m;
    inv.Invert();
    id = m * inv;
    EXPECT_TRUE(inv.IsEqualApprox(expected));
    EXPECT_TRUE(id.IsEqualApprox(Matrix4x4::sIdentity));

    Matrix4x4 singular(Vector4(1.0f, 2.0f, 3.0f, 4.0f),
                       Vector4(2.0f, 4.0f, 6.0f, 8.0f),
                       Vector4(3.0f, 6.0f, 9.0f, 12.0f),
                       Vector4(4.0f, 8.0f, 12.0f, 16.0f));
    Matrix4x4 invSingular = singular.Inverse();
    EXPECT_TRUE(std::isnan(invSingular.Row0.X));
    EXPECT_TRUE(std::isnan(invSingular.Row3.W));
}

TEST_F(Matrix4x4Test, Trace_ValidMatrix_ReturnsSumOfDiagonalElements)
{
    EXPECT_FLOAT_EQ(mat116.Trace(), 1.0f + 6.0f + 11.0f + 16.0f);
}

TEST_F(Matrix4x4Test, Determinant_ValidMatrix_ReturnsCorrectScalarValue)
{
    Matrix4x4 m(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                Vector4(0.0f, 2.0f, 0.0f, 0.0f),
                Vector4(0.0f, 0.0f, 3.0f, 0.0f),
                Vector4(0.0f, 0.0f, 0.0f, 4.0f));
    EXPECT_NEAR(m.Determinant(), 24.0f, math::EPSILON_CMP);
}

TEST_F(Matrix4x4Test, ToMatrix3x3Conversion)
{
    Matrix3x3 sub = mat116.ToMatrix3x3();
    EXPECT_EQ(sub.Row0, Vector3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(sub.Row1, Vector3(5.0f, 6.0f, 7.0f));
    EXPECT_EQ(sub.Row2, Vector3(9.0f, 10.0f, 11.0f));
}
