#include "Core/Math/Basis2D.h"

#include <gtest/gtest.h>

#include "Core/Math/MathFuncs.h"
#include "Core/Math/Matrix2x2.h"
#include "Core/Math/Vector2.h"

using namespace ho;

class Basis2DTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        defaultBasis = Basis2D();
        scaleBasis = Basis2D(Vector2(2.0f, 0.0f), Vector2(0.0f, 3.0f));
        float angle = math::PI / 4.0f;
        float sin = std::sin(angle);
        float cos = std::cos(angle);
        rotationBasis = Basis2D(Matrix2x2(Vector2(cos, sin), Vector2(-sin, cos)));
        nonOrthoBasis = Basis2D(Vector2(1.0f, 0.5f), Vector2(0.3f, 1.0f));
    }

    Basis2D defaultBasis, scaleBasis, rotationBasis, nonOrthoBasis;
};

TEST_F(Basis2DTest, Constructor_DefaultAndAllParameters_InitializesMatrixCorrectly)
{
    EXPECT_EQ(defaultBasis.Matrix.Row0, Vector2(1.0f, 0.0f));
    EXPECT_EQ(defaultBasis.Matrix.Row1, Vector2(0.0f, 1.0f));

    EXPECT_EQ(scaleBasis.Matrix.Row0, Vector2(2.0f, 0.0f));
    EXPECT_EQ(scaleBasis.Matrix.Row1, Vector2(0.0f, 3.0f));

    Matrix2x2 m(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
    Basis2D basisFromMat(m);
    EXPECT_EQ(basisFromMat.Matrix.Row0, Vector2(1.0f, 3.0f));
    EXPECT_EQ(basisFromMat.Matrix.Row1, Vector2(2.0f, 4.0f));
}

TEST_F(Basis2DTest, OperatorAssign_AnotherBasis_CopiesValues)
{
    Basis2D b = scaleBasis;
    EXPECT_EQ(b, scaleBasis);
}

TEST_F(Basis2DTest, GetVectorsAndScale_ValidBasis_ReturnsCorrectDirectionsAndMagnitudes)
{
    Basis2D b = scaleBasis;
    EXPECT_EQ(b.GetRight(), Vector2(2.0f, 0.0f).Normalized());
    EXPECT_EQ(b.GetUp(), Vector2(0.0f, 3.0f).Normalized());

    EXPECT_EQ(b.GetScaledRight(), Vector2(2.0f, 0.0f));
    EXPECT_EQ(b.GetScaledUp(), Vector2(0.0f, 3.0f));

    EXPECT_NEAR(b.GetScaleX(), Vector2(2.0f, 0.0f).Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(b.GetScaleY(), Vector2(0.0f, 3.0f).Magnitude(), math::EPSILON_CMP);
}

TEST_F(Basis2DTest, OperatorComparison_DifferentBases_ReturnsCorrectBoolean)
{
    Basis2D b1(Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f));
    Basis2D b2(Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f));
    Basis2D b3(Vector2(2.0f, 0.0f), Vector2(0.0f, 3.0f));

    EXPECT_TRUE(b1 == b2);
    EXPECT_FALSE(b1 != b2);
    EXPECT_TRUE(b1 != b3);
}

TEST_F(Basis2DTest, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Basis2D b1(Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f));
    Basis2D b2(Vector2(1.000001f, 0.0f), Vector2(0.0f, 0.999999f));
    EXPECT_TRUE(b1.IsEqualApprox(b2));
    EXPECT_FALSE(b1.IsNotEqualApprox(b2));
}

TEST_F(Basis2DTest, IsOrthogonal_ValidAndInvalidBases_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(scaleBasis.IsOrthogonal());
    EXPECT_FALSE(nonOrthoBasis.IsOrthogonal());
}

TEST_F(Basis2DTest, Orthogonalize_NonOrthogonalBasis_MakesBasisOrthogonal)
{
    Basis2D o = nonOrthoBasis.Orthogonalized();
    EXPECT_TRUE(o.IsOrthogonal());

    nonOrthoBasis.Orthogonalize();
    EXPECT_TRUE(nonOrthoBasis.IsOrthogonal());
}

TEST_F(Basis2DTest, IsOrthonormal_ValidAndInvalidBases_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(rotationBasis.IsOrthonormal());
    EXPECT_FALSE(scaleBasis.IsOrthonormal());
}

TEST_F(Basis2DTest, Orthonormalize_NonOrthonormalBasis_MakesBasisOrthonormal)
{
    Basis2D n = scaleBasis.Orthonormalized();
    EXPECT_TRUE(n.IsOrthonormal());

    scaleBasis.Orthonormalize();
    EXPECT_TRUE(scaleBasis.IsOrthonormal());
}

TEST_F(Basis2DTest, Scale_GlobalAndLocal_AppliesScalingCorrectly)
{
    Basis2D b(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));

    Basis2D b1 = b;
    b1.Scale(Vector2(2.0f, 3.0f));
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(scaleBasis.Matrix * b.Matrix));

    b1 = b;
    b1.ScaleLocal(Vector2(2.0f, 3.0f));
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * scaleBasis.Matrix));

    b1 = b.Scaled(Vector2(2.0f, 3.0f));
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(scaleBasis.Matrix * b.Matrix));

    b1 = b.ScaledLocal(Vector2(2.0f, 3.0f));
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * scaleBasis.Matrix));

    Matrix2x2 m_scale(Vector2(2.0f, 0.0f), Vector2(0.0f, 2.0f));

    b1 = b;
    b1.ScaleUniform(2.0f);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(m_scale * b.Matrix));

    b1 = b.ScaledUniformLocal(2.0f);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * m_scale));
}

TEST_F(Basis2DTest, Rotate_GlobalAndLocal_AppliesRotationCorrectly)
{
    Basis2D b(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));

    Basis2D b1 = b;
    b1.Rotate(math::PI / 4.0f);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rotationBasis.Matrix * b.Matrix));

    b1 = b;
    b1.RotateLocal(math::PI / 4.0f);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * rotationBasis.Matrix));

    b1 = b.Rotated(math::PI / 4.0f);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rotationBasis.Matrix * b.Matrix));

    b1 = b.RotatedLocal(math::PI / 4.0f);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * rotationBasis.Matrix));
}

TEST_F(Basis2DTest, Inverse_GeneralBasis_ReturnsInvertedMatrix)
{
    Basis2D b1 = nonOrthoBasis;
    b1.Invert();
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(nonOrthoBasis.Matrix.Inverse()));

    b1 = nonOrthoBasis.Inverse();
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(nonOrthoBasis.Matrix.Inverse()));
}

TEST_F(Basis2DTest, InverseFast_OrthonormalBasis_ReturnsInverseUsingTranspose)
{
    Basis2D b1 = nonOrthoBasis;
    b1.InvertFast();
    EXPECT_FALSE(b1.Matrix.IsEqualApprox(nonOrthoBasis.Matrix.Inverse()));

    b1 = rotationBasis;
    b1.InvertFast();
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rotationBasis.Matrix.Inverse()));

    b1 = rotationBasis.InverseFast();
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rotationBasis.Matrix.Inverse()));
}

TEST_F(Basis2DTest, LookAt_TargetDirection_OrientsBasisWhileMaintainingScale)
{
    Basis2D b = scaleBasis;
    b.LookAt(Vector2(1.0f, 0.0f));

    EXPECT_TRUE(b.IsOrthogonal());
    EXPECT_TRUE(b.IsEqualApprox(Basis2D(Vector2(0.0f, -2.0f), Vector2(3.0f, 0.0f))));
}

TEST_F(Basis2DTest, LookAt_ZeroTarget_DoesNotChangeOrientation)
{
    Basis2D b = scaleBasis;
    Basis2D lookedB = b.LookedAt(Vector2::sZero);

    EXPECT_TRUE(b == lookedB);
}

TEST_F(Basis2DTest, LookAt_UnnormalizedTarget_NormalizesAndOrientsCorrectly)
{
    Basis2D b = scaleBasis;
    Vector2 at(10.0f, 10.0f);

    b.LookAt(at);

    EXPECT_TRUE(b.IsOrthogonal());
    EXPECT_NEAR(b.GetScaleX(), 2.0f, math::EPSILON_CMP);
    EXPECT_NEAR(b.GetScaleY(), 3.0f, math::EPSILON_CMP);

    Vector2 expectedUp = at.Normalized() * 3.0f;
    EXPECT_TRUE(b.GetScaledUp().IsEqualApprox(expectedUp));
}

TEST_F(Basis2DTest, LookAt_SameDirection_DoesNotChangeOrientation)
{
    Basis2D b = scaleBasis;

    b.LookAt(Vector2(0.0f, 1.0f));

    EXPECT_TRUE(b.IsOrthogonal());

    EXPECT_TRUE(b.IsEqualApprox(scaleBasis));
}

TEST_F(Basis2DTest, OperatorMul_TwoBases_ReturnsCombinedTransform)
{
    Basis2D b1(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
    Basis2D b2(Vector2(4.0f, 3.0f), Vector2(2.0f, 1.0f));
    Matrix2x2 m1 = Matrix2x2(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
    Matrix2x2 m2 = Matrix2x2(Vector2(4.0f, 3.0f), Vector2(2.0f, 1.0f));

    EXPECT_TRUE((b1 * b2).Matrix.IsEqualApprox(m1 * m2));
    EXPECT_TRUE((b2 * b1).Matrix.IsEqualApprox(m2 * m1));
}

TEST_F(Basis2DTest, Transform_Vector_ReturnsCorrectlyTransformedVector)
{
    Basis2D b1(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
    Matrix2x2 m1 = Matrix2x2(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
    Vector2 v(1.0f, 1.0f);

    EXPECT_TRUE(b1.Transform(v).IsEqualApprox(m1 * v));
    EXPECT_TRUE(b1.InvTransform(v).IsEqualApprox(m1.Inverse() * v));
    EXPECT_FALSE(b1.InvTransformFast(v).IsEqualApprox(m1.Inverse() * v));
    EXPECT_TRUE(rotationBasis.InvTransformFast(v).IsEqualApprox(rotationBasis.Matrix.Inverse() * v));
}
