#include "Core/Math/Transform2D.h"

#include <gtest/gtest.h>

#include "Core/Math/Basis2D.h"
#include "Core/Math/MathFuncs.h"
#include "Core/Math/Matrix3x3.h"
#include "Core/Math/Vector2.h"

using namespace ho;

class Transform2DTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        defaultBasis = Basis2D();
        scaleBasis = Basis2D(Vector2(2.0f, 0.0f), Vector2(0.0f, 3.0f));

        float angle = math::PI / 4.0f;
        float sin = std::sin(angle);
        float cos = std::cos(angle);
        rotationBasis = Basis2D(Matrix2x2(Vector2(cos, -sin), Vector2(sin, cos)));

        defaultTransform = Transform2D();
        scaleTransform = Transform2D(scaleBasis, Vector2(1.0f, 2.0f));
        transformFromBasis = Transform2D(rotationBasis, Vector2(3.0f, 4.0f));
        nonOrthoTransform = Transform2D(Basis2D(Vector2(1.0f, 0.2f), Vector2(0.3f, 1.0f)), Vector2(2.0f, 1.0f));
    }

    Basis2D defaultBasis, scaleBasis, rotationBasis;
    Transform2D defaultTransform, scaleTransform, transformFromBasis, nonOrthoTransform;
};

TEST_F(Transform2DTest, Constructor_DefaultAndAllParameters_InitializesMatrixCorrectly)
{
    EXPECT_EQ(defaultTransform.GetBasis(), defaultBasis);
    EXPECT_EQ(defaultTransform.GetOrigin(), Vector2(0.0f, 0.0f));

    EXPECT_EQ(scaleTransform.GetBasis(), scaleBasis);
    EXPECT_EQ(scaleTransform.GetOrigin(), Vector2(1.0f, 2.0f));

    Matrix3x3 m = scaleTransform.Matrix;
    Transform2D transformFromMatrix(m);
    EXPECT_TRUE(transformFromMatrix.Matrix.IsEqualApprox(m));
}

TEST_F(Transform2DTest, OperatorAssign_AnotherTransform_CopiesValues)
{
    Transform2D t = scaleTransform;
    EXPECT_EQ(t, scaleTransform);
}

TEST_F(Transform2DTest, GetAndSet_ValidTransform_UpdatesAndReturnsProperties)
{
    Transform2D t = scaleTransform;
    EXPECT_EQ(t.GetRight(), Vector2(2.0f, 0.0f).Normalized());
    EXPECT_EQ(t.GetUp(), Vector2(0.0f, 3.0f).Normalized());

    EXPECT_EQ(t.GetScaledRight(), Vector2(2.0f, 0.0f));
    EXPECT_EQ(t.GetScaledUp(), Vector2(0.0f, 3.0f));

    EXPECT_NEAR(t.GetScaleX(), Vector2(2.0f, 0.0f).Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(t.GetScaleY(), Vector2(0.0f, 3.0f).Magnitude(), math::EPSILON_CMP);

    EXPECT_EQ(transformFromBasis.GetOrigin(), Vector2(3.0f, 4.0f));
    transformFromBasis.SetOrigin(Vector2(4.0f, 3.0f));
    EXPECT_EQ(transformFromBasis.GetOrigin(), Vector2(4.0f, 3.0f));

    EXPECT_EQ(transformFromBasis.GetBasis(), rotationBasis);
    transformFromBasis.SetBasis(scaleBasis);
    EXPECT_EQ(transformFromBasis.GetBasis(), scaleBasis);
}

TEST_F(Transform2DTest, OperatorComparison_DifferentTransforms_ReturnsCorrectBoolean)
{
    Transform2D t1(scaleBasis, Vector2(1.0f, 2.0f));
    Transform2D t2(scaleBasis, Vector2(1.0f, 2.0f));
    Transform2D t3(rotationBasis, Vector2(3.0f, 4.0f));

    EXPECT_TRUE(t1 == t2);
    EXPECT_FALSE(t1 != t2);
    EXPECT_TRUE(t1 != t3);
}

TEST_F(Transform2DTest, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Transform2D t1(scaleBasis, Vector2(1.0f, 2.0f));
    Transform2D t2(Basis2D(Vector2(2.000001f, 0.0f), Vector2(0.0f, 3.000001f)), Vector2(1.000001f, 2.000001f));
    EXPECT_TRUE(t1.IsEqualApprox(t2));
    EXPECT_FALSE(t1.IsNotEqualApprox(t2));
}

TEST_F(Transform2DTest, IsOrthogonal_ValidAndInvalidTransforms_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(transformFromBasis.IsOrthogonal());
    EXPECT_FALSE(nonOrthoTransform.IsOrthogonal());
}

TEST_F(Transform2DTest, Orthogonalize_NonOrthogonalTransform_MakesTransformOrthogonal)
{
    Transform2D ortho = nonOrthoTransform.Orthogonalized();
    EXPECT_TRUE(ortho.IsOrthogonal());

    nonOrthoTransform.Orthogonalize();
    EXPECT_TRUE(nonOrthoTransform.IsOrthogonal());
}

TEST_F(Transform2DTest, IsOrthonormal_ValidAndInvalidTransforms_ReturnsCorrectBoolean)
{
    EXPECT_FALSE(scaleTransform.IsOrthonormal());
    EXPECT_TRUE(transformFromBasis.IsOrthonormal());
}

TEST_F(Transform2DTest, Orthonormalize_NonOrthonormalTransform_MakesTransformOrthonormal)
{
    EXPECT_FALSE(scaleTransform.IsOrthonormal());
    EXPECT_TRUE(transformFromBasis.IsOrthonormal());

    Transform2D normed = nonOrthoTransform.Orthonormalized();
    EXPECT_TRUE(normed.IsOrthonormal());

    nonOrthoTransform.Orthonormalize();
    EXPECT_TRUE(nonOrthoTransform.IsOrthonormal());
}

TEST_F(Transform2DTest, Scale_GlobalSpace_AppliesScalingCorrectly)
{
    Transform2D t(rotationBasis, Vector2(1.0f, 2.0f));
    Matrix3x3 baseMat = t.Matrix;
    Matrix3x3 scaleMat(Vector3(2.0f, 0.0f, 0.0f), Vector3(0.0f, 3.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    Transform2D t1 = t;
    t1.Scale(Vector2(2.0f, 3.0f));
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(scaleMat * baseMat));

    t1 = t.Scaled(Vector2(2.0f, 3.0f));
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(scaleMat * baseMat));

    t1 = t;
    t1.ScaleUniform(2.0f);
    Matrix3x3 uniformScaleMat(Vector3(2.0f, 0.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(uniformScaleMat * baseMat));

    t1 = t.ScaledUniform(2.0f);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(uniformScaleMat * baseMat));
}

TEST_F(Transform2DTest, Scale_LocalSpace_AppliesScalingCorrectly)
{
    Transform2D t(rotationBasis, Vector2(1.0f, 2.0f));
    Matrix3x3 baseMat = t.Matrix;
    Matrix3x3 scaleMat(Vector3(2.0f, 0.0f, 0.0f), Vector3(0.0f, 3.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    Transform2D t1 = t;
    t1.ScaleLocal(Vector2(2.0f, 3.0f));
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * scaleMat));

    t1 = t.ScaledLocal(Vector2(2.0f, 3.0f));
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * scaleMat));

    Matrix3x3 uniformScaleMat(Vector3(2.0f, 0.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));
    t1 = t;
    t1.ScaleUniformLocal(2.0f);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * uniformScaleMat));

    t1 = t.ScaledUniformLocal(2.0f);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * uniformScaleMat));
}

TEST_F(Transform2DTest, Rotate_GlobalSpace_AppliesRotationCorrectly)
{
    Transform2D t(scaleBasis, Vector2(1.0f, 2.0f));
    Matrix3x3 baseMat = t.Matrix;
    float angle = math::PI / 4.0f;
    float sin = std::sin(angle);
    float cos = std::cos(angle);
    Matrix3x3 rotationMat(Vector3(cos, sin, 0.0f), Vector3(-sin, cos, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    Transform2D t1 = t;
    t1.Rotate(angle);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat * baseMat));

    t1 = t.Rotated(angle);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat * baseMat));
}

TEST_F(Transform2DTest, Rotate_LocalSpace_AppliesRotationCorrectly)
{
    Transform2D t(scaleBasis, Vector2(1.0f, 2.0f));
    Matrix3x3 baseMat = t.Matrix;
    float angle = math::PI / 4.0f;
    float sin = std::sin(angle);
    float cos = std::cos(angle);
    Matrix3x3 rotationMat(Vector3(cos, sin, 0.0f), Vector3(-sin, cos, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    Transform2D t1 = t;
    t1.RotateLocal(angle);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * rotationMat));

    t1 = t.RotatedLocal(angle);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * rotationMat));
}

TEST_F(Transform2DTest, Translate_GlobalSpace_AppliesTranslationCorrectly)
{
    Transform2D t(scaleBasis, Vector2(1.0f, 2.0f));
    Matrix3x3 baseMat = t.Matrix;

    Vector2 translationVector(3.0f, 4.0f);
    Matrix3x3 translationMat(
        Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(translationVector.X, translationVector.Y, 1.0f));

    Transform2D t1 = t;
    t1.Translate(translationVector);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(translationMat * baseMat));

    t1 = t.Translated(translationVector);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(translationMat * baseMat));
}

TEST_F(Transform2DTest, Translate_LocalSpace_AppliesTranslationCorrectly)
{
    Transform2D t(scaleBasis, Vector2(1.0f, 2.0f));
    Matrix3x3 baseMat = t.Matrix;

    Vector2 translationVector(3.0f, 4.0f);
    Matrix3x3 translationMat(
        Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(translationVector.X, translationVector.Y, 1.0f));

    Transform2D t1 = t;
    t1.TranslateLocal(translationVector);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * translationMat));

    t1 = t.TranslatedLocal(translationVector);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * translationMat));
}

TEST_F(Transform2DTest, Inverse_GeneralTransform_ReturnsInvertedMatrix)
{
    Transform2D t = nonOrthoTransform;
    Transform2D t1 = t;
    t1.Invert();
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix.Inverse()));

    t1 = t.Inverse();
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix.Inverse()));
}

TEST_F(Transform2DTest, InverseFast_OrthonormalTransform_ReturnsInverseUsingTranspose)
{
    Transform2D t = nonOrthoTransform;
    Transform2D t1 = t;
    t1.InvertFast();
    EXPECT_FALSE(t1.Matrix.IsEqualApprox(t.Matrix.Inverse()));

    t1 = transformFromBasis;
    t1.InvertFast();
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(transformFromBasis.Matrix.Inverse()));

    t1 = transformFromBasis.InverseFast();
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(transformFromBasis.Matrix.Inverse()));
}

TEST_F(Transform2DTest, LookAt_StandardTarget_OrientsBasisWhileMaintainingScale)
{
    Vector2 at(1.0f, 2.0f);
    Transform2D t = transformFromBasis;
    t.LookAt(at);
    Vector2 dir = (at - transformFromBasis.GetOrigin()).Normalized();

    EXPECT_TRUE(t.GetBasis().IsOrthogonal());
    EXPECT_TRUE(t.GetUp().IsEqualApprox(dir));
    EXPECT_NEAR(t.GetScaleY(), transformFromBasis.GetScaleY(), math::EPSILON_CMP);
    EXPECT_NEAR(t.GetScaleX(), transformFromBasis.GetScaleX(), math::EPSILON_CMP);
    EXPECT_NEAR(t.GetRight().Dot(dir), 0.0f, math::EPSILON_CMP);

    EXPECT_TRUE(t.GetOrigin().IsEqualApprox(transformFromBasis.GetOrigin()));
}

TEST_F(Transform2DTest, LookAt_TargetIsOrigin_DoesNotChangeOrientation)
{
    Transform2D t = transformFromBasis;
    Vector2 at = t.GetOrigin();
    t.LookAt(at);

    EXPECT_TRUE(t == transformFromBasis);
}

TEST_F(Transform2DTest, LookAt_SameDirection_DoesNotChangeOrientation)
{
    Vector2 at = (transformFromBasis.GetUp() * 2.0f) + transformFromBasis.GetOrigin();
    Transform2D t = transformFromBasis;
    t.LookAt(at);

    EXPECT_TRUE(transformFromBasis.IsEqualApprox(t));
}

TEST_F(Transform2DTest, OperatorMul_TwoTransforms_ReturnsCombinedTransform)
{
    Transform2D t1(scaleBasis, Vector2(1.0f, 2.0f));
    Transform2D t2(rotationBasis, Vector2(3.0f, 4.0f));

    EXPECT_TRUE((t1 * t2).Matrix.IsEqualApprox(t1.Matrix * t2.Matrix));
    EXPECT_TRUE((t2 * t1).Matrix.IsEqualApprox(t2.Matrix * t1.Matrix));
}

TEST_F(Transform2DTest, Transform_Vector_ReturnsCorrectlyTransformedVector)
{
    Transform2D t(scaleBasis, Vector2(1.0f, 2.0f));
    Matrix3x3 m = t.Matrix;
    Vector2 v(2.0f, 3.0f);

    EXPECT_TRUE(t.Transform(v).IsEqualApprox(Vector2(m * Vector3(v.X, v.Y, 1.0f))));
    EXPECT_TRUE(t.InvTransform(v).IsEqualApprox(Vector2(m.Inverse() * Vector3(v.X, v.Y, 1.0f))));
    EXPECT_FALSE(t.InvTransformFast(v).IsEqualApprox(Vector2(m.Inverse() * Vector3(v.X, v.Y, 1.0f))));

    Transform2D orthoTransform(rotationBasis, Vector2(1.0f, 2.0f));
    EXPECT_TRUE(orthoTransform.InvTransformFast(v).IsEqualApprox(
        Vector2(orthoTransform.Matrix.Inverse() * Vector3(v.X, v.Y, 1.0f))));
}
