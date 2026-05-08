#include "Core/Math/Matrix2x2.h"

#include <gtest/gtest.h>

using namespace ho;

HO_DISABLE_COMPILER_WARNING_PUSH
HO_DISABLE_COMPILER_WARNING_PADDED_DUE_TO_ALIGNMENT_SPECIFIER

class Matrix2x2Test : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        zero = Matrix2x2();
        eye = Matrix2x2(Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f));

        mat14 = Matrix2x2(Vector2(1.0f, 3.0f), Vector2(2.0f, 4.0f));
        mat58 = Matrix2x2(Vector2(5.0f, 7.0f), Vector2(6.0f, 8.0f));

        const float angle = math::PI / 4.0f;
        const float s = std::sin(angle);
        const float c = std::cos(angle);
        matOrthonormal = Matrix2x2(Vector2(c, -s), Vector2(s, c));
        matOrthogonal = Matrix2x2(Vector2(2.0f * c, 2.0f * s), Vector2(3.0f * -s, 3.0f * c));
    }

    Matrix2x2 zero, eye;
    Matrix2x2 mat14, mat58;
    Matrix2x2 matOrthogonal, matOrthonormal;
};

HO_DISABLE_COMPILER_WARNING_POP

TEST_F(Matrix2x2Test, Constructor_DefaultAndAllParameters_InitializeCorrectly)
{
    EXPECT_EQ(zero.Row0, Vector2(0.0f, 0.0f));
    EXPECT_EQ(zero.Row1, Vector2(0.0f, 0.0f));

    EXPECT_EQ(mat14.Row0, Vector2(1.0f, 2.0f));
    EXPECT_EQ(mat14.Row1, Vector2(3.0f, 4.0f));
}

TEST_F(Matrix2x2Test, StaticConstant_PredefinedMatrix_MatchesExpectedValues)
{
    EXPECT_EQ(Matrix2x2::sIdentity, eye);
}

TEST_F(Matrix2x2Test, OperatorAssign_AnotherMatrix_CopiesValues)
{
    zero = mat14;
    EXPECT_EQ(zero, mat14);
}

TEST_F(Matrix2x2Test, GetAndSetCol_ValidVectors_UpdatesAndReturnsColumns)
{
    EXPECT_EQ(mat14.GetCol0(), Vector2(1.0f, 3.0f));
    EXPECT_EQ(mat14.GetCol1(), Vector2(2.0f, 4.0f));

    mat14.SetCol0(Vector2(10.0f, 30.0f));
    mat14.SetCol1(Vector2(20.0f, 40.0f));
    EXPECT_EQ(mat14.GetCol0(), Vector2(10.0f, 30.0f));
    EXPECT_EQ(mat14.GetCol1(), Vector2(20.0f, 40.0f));
}

TEST_F(Matrix2x2Test, OperatorAdd_TwoMatrices_ReturnsMatrixSum)
{
    Matrix2x2 sum = mat14 + mat58;
    EXPECT_EQ(sum.Row0, Vector2(6.0f, 8.0f));
    EXPECT_EQ(sum.Row1, Vector2(10.0f, 12.0f));

    mat14 += mat58;
    EXPECT_EQ(mat14, Matrix2x2(Vector2(6.0f, 10.0f), Vector2(8.0f, 12.0f)));
}

TEST_F(Matrix2x2Test, OperatorSub_TwoMatrices_ReturnsMatrixDifference)
{
    Matrix2x2 sub = mat58 - mat14;
    EXPECT_EQ(sub.Row0, Vector2(4.0f, 4.0f));
    EXPECT_EQ(sub.Row1, Vector2(4.0f, 4.0f));

    mat58 -= mat14;
    EXPECT_EQ(mat58, Matrix2x2(Vector2(4.0f, 4.0f), Vector2(4.0f, 4.0f)));
}

TEST_F(Matrix2x2Test, OperatorMul_TwoMatrices_ReturnsMatrixProduct)
{
    // mat14 * mat58 = [[1*5+3*6, 1*7+3*8],
    //          [2*5+4*6, 2*7+4*8]]
    Matrix2x2 expected(Vector2(19.0f, 43.0f), Vector2(22.0f, 50.0f));
    EXPECT_TRUE((mat14 * mat58).IsEqualApprox(expected));

    mat14 *= mat58;
    EXPECT_TRUE(mat14.IsEqualApprox(expected));
}

TEST_F(Matrix2x2Test, OperatorMul_MatrixAndVector_ReturnsTransformedVector)
{
    Vector2 v(5.0f, 6.0f);
    // row0·v = 1*5 + 2*6 = 17
    // row1·v = 3*5 + 4*6 = 39
    EXPECT_TRUE((mat14 * v).IsEqualApprox(Vector2(17.0f, 39.0f)));
}

TEST_F(Matrix2x2Test, OperatorMul_ScalarValue_ScalesAllElements)
{
    Matrix2x2 mul = mat14 * 2.0f;
    EXPECT_EQ(mul.Row0, Vector2(2.0f, 4.0f));
    EXPECT_EQ(mul.Row1, Vector2(6.0f, 8.0f));

    Matrix2x2 leftMul = 2.0f * mat14;
    EXPECT_EQ(leftMul.Row0, Vector2(2.0f, 4.0f));
    EXPECT_EQ(leftMul.Row1, Vector2(6.0f, 8.0f));

    mat14 *= 2.0f;
    EXPECT_EQ(mat14.Row0, Vector2(2.0f, 4.0f));
    EXPECT_EQ(mat14.Row1, Vector2(6.0f, 8.0f));
}

TEST_F(Matrix2x2Test, OperatorDiv_ScalarValue_ReturnsQuotientOrInfinityOnZero)
{
    Matrix2x2 div = mat14 / 2.0f;
    EXPECT_EQ(div.Row0, Vector2(1.0f / 2.0f, 2.0f / 2.0f));
    EXPECT_EQ(div.Row1, Vector2(3.0f / 2.0f, 4.0f / 2.0f));

    mat14 /= 2.0f;
    EXPECT_EQ(mat14.Row0, Vector2(1.0f / 2.0f, 2.0f / 2.0f));
    EXPECT_EQ(mat14.Row1, Vector2(3.0f / 2.0f, 4.0f / 2.0f));

    mat14 /= 0.0f;
    EXPECT_TRUE(std::isinf(mat14.Row0.X));
    EXPECT_TRUE(std::isinf(mat14.Row0.Y));
    EXPECT_TRUE(std::isinf(mat14.Row1.X));
    EXPECT_TRUE(std::isinf(mat14.Row1.Y));
}

TEST_F(Matrix2x2Test, OperatorComparison_DifferentMatrices_ReturnsCorrectBoolean)
{
    Matrix2x2 a(Vector2(1.0f, 3.0f), Vector2(2.0f, 4.0f));

    EXPECT_TRUE(mat14 == a);
    EXPECT_FALSE(mat14 != a);
    EXPECT_TRUE(mat14 != mat58);
}

TEST_F(Matrix2x2Test, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Matrix2x2 a(Vector2(1.000001f, 3.000001f), Vector2(1.999999f, 3.999999f));
    EXPECT_TRUE(mat14.IsEqualApprox(a));
    EXPECT_FALSE(mat14.IsNotEqualApprox(a));
}

TEST_F(Matrix2x2Test, IsOrthogonal_VariousMatrices_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(matOrthogonal.IsOrthogonal());
    EXPECT_FALSE(mat14.IsOrthogonal());
}

TEST_F(Matrix2x2Test, Orthogonalize_NonOrthogonalMatrix_MakesMatrixOrthogonal)
{
    Matrix2x2 o = mat14.Orthogonalized();
    EXPECT_TRUE(o.IsOrthogonal());
    EXPECT_NEAR(o.GetCol0().Magnitude(), mat14.GetCol0().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol1().Magnitude(), mat14.GetCol1().Magnitude(), math::EPSILON_CMP);

    EXPECT_FALSE(mat14.IsOrthogonal());

    mat14.Orthogonalize();
    EXPECT_TRUE(o.IsOrthogonal());
    EXPECT_NEAR(o.GetCol0().Magnitude(), mat14.GetCol0().Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(o.GetCol1().Magnitude(), mat14.GetCol1().Magnitude(), math::EPSILON_CMP);
}

TEST_F(Matrix2x2Test, IsOrthonormal_VariousMatrices_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(matOrthonormal.IsOrthonormal());
    EXPECT_FALSE(mat14.IsOrthonormal());
}

TEST_F(Matrix2x2Test, Orthonormalize_NonOrthonormalMatrix_MakesMatrixOrthonormal)
{
    Matrix2x2 n = mat14.Orthonormalized();
    EXPECT_TRUE(n.IsOrthonormal());
    EXPECT_FALSE(mat14.IsOrthonormal());

    mat14.Orthonormalize();
    EXPECT_TRUE(mat14.IsOrthonormal());
}

TEST_F(Matrix2x2Test, Transposed_ValidMatrix_SwapsRowsAndColumns)
{
    Matrix2x2 transposed(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));

    Matrix2x2 t = mat14.Transposed();
    EXPECT_TRUE(t.IsEqualApprox(transposed));

    EXPECT_TRUE(mat14.IsEqualApprox(Matrix2x2(Vector2(1.0f, 3.0f), Vector2(2.0f, 4.0f))));
}

TEST_F(Matrix2x2Test, Inverse_InvertibleAndSingularMatrices_ReturnsInverseOrInfinity)
{
    Matrix2x2 inversed(Vector2(-2.0f, 1.5f), Vector2(1.0f, -0.5f));

    Matrix2x2 inv = mat14.Inverse();
    EXPECT_TRUE(inv.IsEqualApprox(inversed));
    Matrix2x2 id = mat14 * inv;
    EXPECT_TRUE(id.IsEqualApprox(Matrix2x2::sIdentity));
    EXPECT_TRUE(mat14.IsEqualApprox(Matrix2x2(Vector2(1.0f, 3.0f), Vector2(2.0f, 4.0f))));

    Matrix2x2 a = mat14;
    a.Invert();
    EXPECT_TRUE(a.IsEqualApprox(inversed));
    id = a * mat14;
    EXPECT_TRUE(id.IsEqualApprox(Matrix2x2::sIdentity));

    // determinant = 0 case
    Matrix2x2 singular(Vector2(1.0f, 2.0f), Vector2(2.0f, 4.0f));
    Matrix2x2 invSingular = singular.Inverse();
    EXPECT_TRUE(std::isinf(invSingular.Row0.X));
    EXPECT_TRUE(std::isinf(invSingular.Row0.Y));
    EXPECT_TRUE(std::isinf(invSingular.Row1.X));
    EXPECT_TRUE(std::isinf(invSingular.Row1.Y));
}

TEST_F(Matrix2x2Test, Trace_ValidMatrix_ReturnsSumOfDiagonalElements)
{
    EXPECT_FLOAT_EQ(mat14.Trace(), 1.0f + 4.0f);
}

TEST_F(Matrix2x2Test, Determinant_ValidMatrix_ReturnsCorrectScalarValue)
{
    EXPECT_FLOAT_EQ(mat14.Determinant(), 1.0f * 4.0f - 2.0f * 3.0f);
}
