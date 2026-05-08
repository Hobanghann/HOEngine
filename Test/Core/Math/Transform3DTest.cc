#include "Core/Math/Transform3D.h"

#include <gtest/gtest.h>

#include "Core/Math/Basis3D.h"
#include "Core/Math/MathFuncs.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector3.h"

using namespace ho;

class Transform3DTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        defaultBasis = Basis3D();
        scaleBasis = Basis3D(Vector3(2.0f, 0.0f, 0.0f), Vector3(0.0f, 3.0f, 0.0f), Vector3(0.0f, 0.0f, 4.0f));

        float angle = math::PI / 4.0f;
        float sin = std::sin(angle);
        float cos = std::cos(angle);

        pitchRotBasis =
            Basis3D(Matrix3x3(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, cos, sin), Vector3(0.0f, -sin, cos)));
        yawRotBasis = Basis3D(Matrix3x3(Vector3(cos, 0.0f, -sin), Vector3(0.0f, 1.0f, 0.0f), Vector3(sin, 0.0f, cos)));
        rollRotBasis = Basis3D(Matrix3x3(Vector3(cos, sin, 0.0f), Vector3(-sin, cos, 0.0f), Vector3(0.0f, 0.0f, 1.0f)));

        nonOrthoBasis = Basis3D(Vector3(1.0f, 0.2f, 0.3f), Vector3(0.5f, 1.0f, 0.0f), Vector3(0.0f, 0.5f, 1.0f));

        defaultTransform = Transform3D();
        scaleTransform = Transform3D(scaleBasis, Vector3(1.0f, 2.0f, 3.0f));
        rotationTransform = Transform3D(yawRotBasis, Vector3(4.0f, 5.0f, 6.0f));
        nonOrthoTransform = Transform3D(nonOrthoBasis, Vector3(2.0f, 3.0f, 4.0f));
    }

    Basis3D defaultBasis, scaleBasis, pitchRotBasis, yawRotBasis, rollRotBasis, nonOrthoBasis;
    Transform3D defaultTransform, scaleTransform, rotationTransform, nonOrthoTransform;
};

TEST_F(Transform3DTest, Constructor_DefaultAndAllParameters_InitializesMatrixCorrectly)
{
    EXPECT_EQ(defaultTransform.GetBasis(), defaultBasis);
    EXPECT_EQ(defaultTransform.GetOrigin(), Vector3(0.0f, 0.0f, 0.0f));

    EXPECT_EQ(scaleTransform.GetBasis(), scaleBasis);
    EXPECT_EQ(scaleTransform.GetOrigin(), Vector3(1.0f, 2.0f, 3.0f));

    Matrix4x4 m = scaleTransform.Matrix;
    Transform3D transformFromMat(m);
    EXPECT_TRUE(transformFromMat.Matrix.IsEqualApprox(m));
}

TEST_F(Transform3DTest, OperatorAssign_AnotherTransform_CopiesValues)
{
    Transform3D t = scaleTransform;
    EXPECT_EQ(t, scaleTransform);
}

TEST_F(Transform3DTest, GetAndSet_ValidTransform_UpdatesAndReturnsProperties)
{
    Transform3D t = scaleTransform;
    EXPECT_EQ(t.GetRight(), Vector3(2.0f, 0.0f, 0.0f).Normalized());
    EXPECT_EQ(t.GetUp(), Vector3(0.0f, 3.0f, 0.0f).Normalized());
    EXPECT_EQ(t.GetForward(), Vector3(0.0f, 0.0f, 4.0f).Normalized());

    EXPECT_EQ(t.GetScaledRight(), Vector3(2.0f, 0.0f, 0.0f));
    EXPECT_EQ(t.GetScaledUp(), Vector3(0.0f, 3.0f, 0.0f));
    EXPECT_EQ(t.GetScaledForward(), Vector3(0.0f, 0.0f, 4.0f));

    EXPECT_NEAR(t.GetScaleX(), Vector3(2.0f, 0.0f, 0.0f).Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(t.GetScaleY(), Vector3(0.0f, 3.0f, 0.0f).Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(t.GetScaleZ(), Vector3(0.0f, 0.0f, 4.0f).Magnitude(), math::EPSILON_CMP);

    EXPECT_EQ(scaleTransform.GetOrigin(), Vector3(1.0f, 2.0f, 3.0f));
    scaleTransform.SetOrigin(Vector3(3.0, 2.0f, 1.0f));
    EXPECT_EQ(scaleTransform.GetOrigin(), Vector3(3.0, 2.0f, 1.0f));

    EXPECT_EQ(scaleTransform.GetBasis(), scaleBasis);
    scaleTransform.SetBasis(rollRotBasis);
    EXPECT_EQ(scaleTransform.GetBasis(), rollRotBasis);
}

TEST_F(Transform3DTest, OperatorComparison_DifferentTransforms_ReturnsCorrectBoolean)
{
    Transform3D t1(scaleBasis, Vector3(1.0f, 2.0f, 3.0f));
    Transform3D t2(scaleBasis, Vector3(1.0f, 2.0f, 3.0f));
    Transform3D t3(yawRotBasis, Vector3(1.0f, 2.0f, 3.0f));

    EXPECT_TRUE(t1 == t2);
    EXPECT_FALSE(t1 != t2);
    EXPECT_TRUE(t1 != t3);
}

TEST_F(Transform3DTest, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Transform3D t1(scaleBasis, Vector3(1.0f, 2.0f, 3.0f));
    Transform3D t2(
        Basis3D(Vector3(2.000001f, 0.0f, 0.0f), Vector3(0.0f, 3.000001f, 0.0f), Vector3(0.0f, 0.0f, 4.000001f)),
        Vector3(1.000001f, 2.000001f, 3.000001f));
    EXPECT_TRUE(t1.IsEqualApprox(t2));
    EXPECT_FALSE(t1.IsNotEqualApprox(t2));
}

TEST_F(Transform3DTest, IsOrthogonal_ValidAndInvalidTransforms_ReturnsCorrectBoolean)
{
    EXPECT_FALSE(nonOrthoTransform.IsOrthogonal());
    EXPECT_TRUE(scaleTransform.IsOrthogonal());
}

TEST_F(Transform3DTest, Orthogonalize_NonOrthogonalTransform_MakesTransformOrthogonal)
{
    Transform3D o = nonOrthoTransform.Orthogonalized();
    EXPECT_TRUE(o.IsOrthogonal());

    nonOrthoTransform.Orthogonalize();
    EXPECT_TRUE(nonOrthoTransform.IsOrthogonal());
}

TEST_F(Transform3DTest, IsOrthonormal_ValidAndInvalidTransforms_ReturnsCorrectBoolean)
{
    EXPECT_FALSE(scaleTransform.IsOrthonormal());
    EXPECT_TRUE(rotationTransform.IsOrthonormal());
}

TEST_F(Transform3DTest, Orthonormalize_NonOrthonormalTransform_MakesTransformOrthonormal)
{
    Transform3D n = nonOrthoTransform.Orthonormalized();
    EXPECT_TRUE(n.IsOrthonormal());

    nonOrthoTransform.Orthonormalize();
    EXPECT_TRUE(nonOrthoTransform.IsOrthonormal());
}

TEST_F(Transform3DTest, Scale_GlobalSpace_AppliesScalingCorrectly)
{
    Transform3D t(yawRotBasis, Vector3(1.0f, 2.0f, 3.0f));
    Matrix4x4 baseMat = t.Matrix;

    Matrix4x4 scaleMat(Vector4(2.0f, 0.0f, 0.0f, 0.0f),
                       Vector4(0.0f, 3.0f, 0.0f, 0.0f),
                       Vector4(0.0f, 0.0f, 4.0f, 0.0f),
                       Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    Transform3D t1 = t;
    t1.Scale(Vector3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(scaleMat * baseMat));

    t1 = t.Scaled(Vector3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(scaleMat * baseMat));

    Matrix4x4 uniformScaleMat(Vector4(2.0f, 0.0f, 0.0f, 0.0f),
                              Vector4(0.0f, 2.0f, 0.0f, 0.0f),
                              Vector4(0.0f, 0.0f, 2.0f, 0.0f),
                              Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    t1 = t;
    t1.ScaleUniform(2.0f);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(uniformScaleMat * baseMat));

    t1 = t.ScaledUniform(2.0f);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(uniformScaleMat * baseMat));
}

TEST_F(Transform3DTest, Scale_LocalSpace_AppliesScalingCorrectly)
{
    Transform3D t(yawRotBasis, Vector3(1.0f, 2.0f, 3.0f));
    Matrix4x4 baseMat = t.Matrix;

    Matrix4x4 scaleMat(Vector4(2.0f, 0.0f, 0.0f, 0.0f),
                       Vector4(0.0f, 3.0f, 0.0f, 0.0f),
                       Vector4(0.0f, 0.0f, 4.0f, 0.0f),
                       Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    Transform3D t1 = t;
    t1.ScaleLocal(Vector3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * scaleMat));

    t1 = t.ScaledLocal(Vector3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * scaleMat));

    Matrix4x4 uniformScaleMat(Vector4(2.0f, 0.0f, 0.0f, 0.0f),
                              Vector4(0.0f, 2.0f, 0.0f, 0.0f),
                              Vector4(0.0f, 0.0f, 2.0f, 0.0f),
                              Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    t1 = t;
    t1.ScaleUniformLocal(2.0f);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * uniformScaleMat));

    t1 = t.ScaledUniformLocal(2.0f);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(baseMat * uniformScaleMat));
}

TEST_F(Transform3DTest, Rotate_GlobalSpace_AppliesRotationCorrectly)
{
    Transform3D t(pitchRotBasis, Vector3(1.0f, 2.0f, 3.0f));
    Vector3 axis = Vector3(1.0f, 2.0f, 3.0f);
    float angle = math::PI / 4.0f;
    Matrix3x3 m = Matrix3x3::FromAxisAngle(axis, angle);
    Matrix4x4 rotationMat =
        Matrix4x4(Vector4(m.GetCol0()), Vector4(m.GetCol1()), Vector4(m.GetCol2()), Vector4::sUnitW);

    Transform3D t1 = t;
    t1.RotateAxisAngle(axis, angle);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat * t.Matrix));

    t1 = t.RotatedAxisAngle(axis, angle);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat * t.Matrix));
}

TEST_F(Transform3DTest, Rotate_LocalSpace_AppliesRotationCorrectly)
{
    Transform3D t(pitchRotBasis, Vector3(1.0f, 2.0f, 3.0f));
    Vector3 axis = Vector3(1.0f, 2.0f, 3.0f);
    float angle = math::PI / 4.0f;
    Matrix3x3 m = Matrix3x3::FromAxisAngle(axis, angle);
    Matrix4x4 rotationMat =
        Matrix4x4(Vector4(m.GetCol0()), Vector4(m.GetCol1()), Vector4(m.GetCol2()), Vector4::sUnitW);

    Transform3D t1 = t;
    t1.RotateAxisAngleLocal(axis, angle);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat));

    t1 = t.RotatedAxisAngleLocal(axis, angle);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat));
}

TEST_F(Transform3DTest, RotateEuler_GlobalSpaceVaryingOrders_AppliesRotationsCorrectly)
{
    Transform3D t(pitchRotBasis, Vector3(1.0f, 2.0f, 3.0f));
    float angle = math::PI / 4.0f;

    Transform3D t1;

    Matrix3x3 rotationMat = pitchRotBasis.Matrix * yawRotBasis.Matrix * rollRotBasis.Matrix;
    Matrix4x4 rotationMat4(Vector4(rotationMat.GetCol0()),
                           Vector4(rotationMat.GetCol1()),
                           Vector4(rotationMat.GetCol2()),
                           Vector4::sUnitW);
    t1 = t;
    t1.RotateEuler(angle, angle, angle, math::eEulerOrder::XYZ);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    t1 = t.RotatedEuler(angle, angle, angle, math::eEulerOrder::XYZ);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    rotationMat = pitchRotBasis.Matrix * rollRotBasis.Matrix * yawRotBasis.Matrix;
    rotationMat4 = Matrix4x4(Vector4(rotationMat.GetCol0()),
                             Vector4(rotationMat.GetCol1()),
                             Vector4(rotationMat.GetCol2()),
                             Vector4::sUnitW);
    t1 = t;
    t1.RotateEuler(angle, angle, angle, math::eEulerOrder::XZY);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    t1 = t.RotatedEuler(angle, angle, angle, math::eEulerOrder::XZY);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    rotationMat = yawRotBasis.Matrix * rollRotBasis.Matrix * pitchRotBasis.Matrix;
    rotationMat4 = Matrix4x4(Vector4(rotationMat.GetCol0()),
                             Vector4(rotationMat.GetCol1()),
                             Vector4(rotationMat.GetCol2()),
                             Vector4::sUnitW);
    t1 = t;
    t1.RotateEuler(angle, angle, angle, math::eEulerOrder::YZX);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    t1 = t.RotatedEuler(angle, angle, angle, math::eEulerOrder::YZX);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    rotationMat = yawRotBasis.Matrix * pitchRotBasis.Matrix * rollRotBasis.Matrix;
    rotationMat4 = Matrix4x4(Vector4(rotationMat.GetCol0()),
                             Vector4(rotationMat.GetCol1()),
                             Vector4(rotationMat.GetCol2()),
                             Vector4::sUnitW);
    t1 = t;
    t1.RotateEuler(angle, angle, angle, math::eEulerOrder::YXZ);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    t1 = t.RotatedEuler(angle, angle, angle, math::eEulerOrder::YXZ);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    rotationMat = rollRotBasis.Matrix * pitchRotBasis.Matrix * yawRotBasis.Matrix;
    rotationMat4 = Matrix4x4(Vector4(rotationMat.GetCol0()),
                             Vector4(rotationMat.GetCol1()),
                             Vector4(rotationMat.GetCol2()),
                             Vector4::sUnitW);
    t1 = t;
    t1.RotateEuler(angle, angle, angle, math::eEulerOrder::ZXY);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    t1 = t.RotatedEuler(angle, angle, angle, math::eEulerOrder::ZXY);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    rotationMat = rollRotBasis.Matrix * yawRotBasis.Matrix * pitchRotBasis.Matrix;
    rotationMat4 = Matrix4x4(Vector4(rotationMat.GetCol0()),
                             Vector4(rotationMat.GetCol1()),
                             Vector4(rotationMat.GetCol2()),
                             Vector4::sUnitW);
    t1 = t;
    t1.RotateEuler(angle, angle, angle, math::eEulerOrder::ZYX);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));

    t1 = t.RotatedEuler(angle, angle, angle, math::eEulerOrder::ZYX);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat4 * t.Matrix));
}

TEST_F(Transform3DTest, RotateEuler_LocalSpaceVaryingOrders_AppliesRotationsCorrectly)
{
    Transform3D t(pitchRotBasis, Vector3(1.0f, 2.0f, 3.0f));
    float angle = math::PI / 4.0f;
    Transform3D t1;

    Matrix3x3 rotationMat = pitchRotBasis.Matrix * yawRotBasis.Matrix * rollRotBasis.Matrix;
    Matrix4x4 rotationMat4(Vector4(rotationMat.GetCol0()),
                           Vector4(rotationMat.GetCol1()),
                           Vector4(rotationMat.GetCol2()),
                           Vector4::sUnitW);
    t1 = t;
    t1.RotateEulerLocal(angle, angle, angle, math::eEulerOrder::XYZ);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    t1 = t.RotatedEulerLocal(angle, angle, angle, math::eEulerOrder::XYZ);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    rotationMat = pitchRotBasis.Matrix * rollRotBasis.Matrix * yawRotBasis.Matrix;
    rotationMat4 = Matrix4x4(Vector4(rotationMat.GetCol0()),
                             Vector4(rotationMat.GetCol1()),
                             Vector4(rotationMat.GetCol2()),
                             Vector4::sUnitW);
    t1 = t;
    t1.RotateEulerLocal(angle, angle, angle, math::eEulerOrder::XZY);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    t1 = t.RotatedEulerLocal(angle, angle, angle, math::eEulerOrder::XZY);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    rotationMat = yawRotBasis.Matrix * rollRotBasis.Matrix * pitchRotBasis.Matrix;
    rotationMat4 = Matrix4x4(Vector4(rotationMat.GetCol0()),
                             Vector4(rotationMat.GetCol1()),
                             Vector4(rotationMat.GetCol2()),
                             Vector4::sUnitW);
    t1 = t;
    t1.RotateEulerLocal(angle, angle, angle, math::eEulerOrder::YZX);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    t1 = t.RotatedEulerLocal(angle, angle, angle, math::eEulerOrder::YZX);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    rotationMat = yawRotBasis.Matrix * pitchRotBasis.Matrix * rollRotBasis.Matrix;
    rotationMat4 = Matrix4x4(Vector4(rotationMat.GetCol0()),
                             Vector4(rotationMat.GetCol1()),
                             Vector4(rotationMat.GetCol2()),
                             Vector4::sUnitW);
    t1 = t;
    t1.RotateEulerLocal(angle, angle, angle, math::eEulerOrder::YXZ);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    t1 = t.RotatedEulerLocal(angle, angle, angle, math::eEulerOrder::YXZ);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    rotationMat = rollRotBasis.Matrix * pitchRotBasis.Matrix * yawRotBasis.Matrix;
    rotationMat4 = Matrix4x4(Vector4(rotationMat.GetCol0()),
                             Vector4(rotationMat.GetCol1()),
                             Vector4(rotationMat.GetCol2()),
                             Vector4::sUnitW);
    t1 = t;
    t1.RotateEulerLocal(angle, angle, angle, math::eEulerOrder::ZXY);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    t1 = t.RotatedEulerLocal(angle, angle, angle, math::eEulerOrder::ZXY);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    rotationMat = rollRotBasis.Matrix * yawRotBasis.Matrix * pitchRotBasis.Matrix;
    rotationMat4 = Matrix4x4(Vector4(rotationMat.GetCol0()),
                             Vector4(rotationMat.GetCol1()),
                             Vector4(rotationMat.GetCol2()),
                             Vector4::sUnitW);
    t1 = t;
    t1.RotateEulerLocal(angle, angle, angle, math::eEulerOrder::ZYX);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));

    t1 = t.RotatedEulerLocal(angle, angle, angle, math::eEulerOrder::ZYX);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat4));
}

TEST_F(Transform3DTest, RotateQuaternion_GlobalSpace_AppliesRotationCorrectly)
{
    Transform3D t(pitchRotBasis, Vector3(1.0f, 2.0f, 3.0f));
    Vector3 axis(1.0f, 2.0f, 3.0f);
    float angle = math::PI / 4.0f;
    Quaternion q = Quaternion::FromAxisAngle(axis, angle);
    Matrix3x3 m = Matrix3x3::FromQuaternion(q);
    Matrix4x4 rotationMat =
        Matrix4x4(Vector4(m.GetCol0()), Vector4(m.GetCol1()), Vector4(m.GetCol2()), Vector4::sUnitW);

    Transform3D t1 = t;
    t1.RotateQuaternion(q);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat * t.Matrix));

    t1 = t.RotatedQuaternion(q);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationMat * t.Matrix));
}

TEST_F(Transform3DTest, RotateQuaternion_LocalSpace_AppliesRotationCorrectly)
{
    Transform3D t(pitchRotBasis, Vector3(1.0f, 2.0f, 3.0f));
    Vector3 axis(1.0f, 2.0f, 3.0f);
    float angle = math::PI / 4.0f;
    Quaternion q = Quaternion::FromAxisAngle(axis, angle);
    Matrix3x3 m = Matrix3x3::FromQuaternion(q);
    Matrix4x4 rotationMat =
        Matrix4x4(Vector4(m.GetCol0()), Vector4(m.GetCol1()), Vector4(m.GetCol2()), Vector4::sUnitW);

    Transform3D t1 = t;
    t1.RotateQuaternionLocal(q);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat));

    t1 = t.RotatedQuaternionLocal(q);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * rotationMat));
}

TEST_F(Transform3DTest, Translate_GlobalSpace_AppliesTranslationCorrectly)
{
    Transform3D t(scaleBasis, Vector3(1.0f, 2.0f, 3.0f));
    Vector3 translationVector(3.0f, 4.0f, 5.0f);
    Matrix4x4 translationMat(Vector4(1.0f, 0.0f, 0.0f, 0.f),
                             Vector4(0.0f, 1.0f, 0.0f, 0.f),
                             Vector4(0.0f, 0.0f, 1.0f, 0.f),
                             translationVector.ToHomogeneous());

    Transform3D t1 = t;
    t1.Translate(translationVector);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(translationMat * t.Matrix));

    t1 = t.Translated(translationVector);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(translationMat * t.Matrix));
}

TEST_F(Transform3DTest, Translate_LocalSpace_AppliesTranslationCorrectly)
{
    Transform3D t(scaleBasis, Vector3(1.0f, 2.0f, 3.0f));
    Vector3 translationVector(3.0f, 4.0f, 5.0f);
    Matrix4x4 translationMat(Vector4(1.0f, 0.0f, 0.0f, 0.f),
                             Vector4(0.0f, 1.0f, 0.0f, 0.f),
                             Vector4(0.0f, 0.0f, 1.0f, 0.f),
                             translationVector.ToHomogeneous());

    Transform3D t1 = t;
    t1.TranslateLocal(translationVector);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * translationMat));

    t1 = t.TranslatedLocal(translationVector);
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(t.Matrix * translationMat));
}

TEST_F(Transform3DTest, Inverse_GeneralTransform_ReturnsInvertedMatrix)
{
    Transform3D t1 = nonOrthoTransform;
    t1.Invert();
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(nonOrthoTransform.Matrix.Inverse()));

    t1 = nonOrthoTransform.Inverse();
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(nonOrthoTransform.Matrix.Inverse()));
}

TEST_F(Transform3DTest, InverseFast_OrthonormalTransform_ReturnsInverseUsingTranspose)
{
    Transform3D t1 = nonOrthoTransform;
    t1.InvertFast();
    EXPECT_FALSE(t1.Matrix.IsEqualApprox(nonOrthoTransform.Matrix.Inverse()));

    t1 = rotationTransform;
    t1.InvertFast();
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationTransform.Matrix.Inverse()));

    t1 = rotationTransform.InverseFast();
    EXPECT_TRUE(t1.Matrix.IsEqualApprox(rotationTransform.Matrix.Inverse()));
}

TEST_F(Transform3DTest, LookAt_StandardTargetAndUp_OrientsTransformCorrectly)
{
    Vector3 at(1.0f, 2.0f, -3.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Transform3D t = scaleTransform;
    t.LookAt(at, up, false);

    EXPECT_TRUE(t.GetBasis().IsOrthogonal());

    Vector3 dir = (at - scaleTransform.GetOrigin()).Normalized();
    EXPECT_TRUE(t.GetForward().IsEqualApprox(-dir));

    Vector3 right = t.GetRight();
    Vector3 crossRightUp = right.Cross(t.GetUp());
    EXPECT_TRUE(crossRightUp.IsEqualApprox(t.GetForward()));

    EXPECT_EQ(t.GetOrigin(), scaleTransform.GetOrigin());
}

TEST_F(Transform3DTest, LookAt_FacingTargetFlagTrue_OrientsForwardToTarget)
{
    Vector3 at(1.0f, 2.0f, -3.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Transform3D t = scaleTransform;
    t.LookAt(at, up, true);

    EXPECT_TRUE(t.GetBasis().IsOrthogonal());

    Vector3 dir = (at - scaleTransform.GetOrigin()).Normalized();
    EXPECT_TRUE(t.GetForward().Normalized().IsEqualApprox(dir));

    Vector3 right = t.GetRight();
    Vector3 crossRightUp = right.Cross(t.GetUp());
    EXPECT_TRUE(crossRightUp.IsEqualApprox(t.GetForward()));

    EXPECT_EQ(t.GetOrigin(), scaleTransform.GetOrigin());
}

TEST_F(Transform3DTest, LookAt_TargetIsOrigin_DoesNotChangeOrientation)
{
    Transform3D t = scaleTransform;
    Vector3 at = t.GetOrigin();
    t.LookAt(at, Vector3::sUnitY, false);

    EXPECT_TRUE(t == scaleTransform);
}

TEST_F(Transform3DTest, LookAt_TargetCollinearWithUp_HandlesGracefully)
{
    Transform3D t = scaleTransform;

    Vector3 at(0.0f, 5.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    t.LookAt(at, up, true);

    EXPECT_TRUE(t.GetBasis().IsOrthogonal());

    Vector3 dir = (at - scaleTransform.GetOrigin()).Normalized();
    EXPECT_TRUE(t.GetForward().Normalized().IsEqualApprox(dir));

    Vector3 right = t.GetRight();
    Vector3 crossRightUp = right.Cross(t.GetUp());
    EXPECT_TRUE(crossRightUp.IsEqualApprox(t.GetForward()));

    EXPECT_EQ(t.GetOrigin(), scaleTransform.GetOrigin());
}

TEST_F(Transform3DTest, LookAt_SameDirection_DoesNotChangeOrientation)
{
    Vector3 at = (scaleTransform.GetScaledForward() * 2.0f) + scaleTransform.GetOrigin();
    Transform3D t = scaleTransform;
    t.LookAt(at, Vector3::sUnitY, true);

    EXPECT_TRUE(scaleTransform.IsEqualApprox(t));
}

TEST_F(Transform3DTest, OperatorMul_TwoTransforms_ReturnsCombinedTransform)
{
    Transform3D t1(scaleBasis, Vector3(1.0f, 2.0f, 3.0f));
    Transform3D t2(yawRotBasis, Vector3(3.0f, 4.0f, 5.0f));

    EXPECT_TRUE((t1 * t2).Matrix.IsEqualApprox(t1.Matrix * t2.Matrix));
    EXPECT_TRUE((t2 * t1).Matrix.IsEqualApprox(t2.Matrix * t1.Matrix));

    Transform3D t3 = t1;
    t3 *= t2;
    EXPECT_TRUE(t3.Matrix.IsEqualApprox(t1.Matrix * t2.Matrix));
}

TEST_F(Transform3DTest, Transform_Vector_ReturnsCorrectlyTransformedCartesianVector)
{
    Transform3D t(yawRotBasis, Vector3(1.0f, 2.0f, 3.0f));
    Matrix4x4 m = t.Matrix;
    Vector3 v(1.0f, 1.0f, 1.0f);

    Vector4 vh(v.X, v.Y, v.Z, 1.0f);
    Vector3 expected = (m * vh).ToCartesian();

    EXPECT_TRUE(t.Transform(v).IsEqualApprox(expected));

    EXPECT_TRUE(t.InvTransform(v).IsEqualApprox((m.Inverse() * vh).ToCartesian()));

    EXPECT_FALSE(
        nonOrthoTransform.InvTransformFast(v).IsEqualApprox((nonOrthoTransform.Matrix.Inverse() * vh).ToCartesian()));

    EXPECT_TRUE(
        rotationTransform.InvTransformFast(v).IsEqualApprox((rotationTransform.Matrix.Inverse() * vh).ToCartesian()));
}

TEST_F(Transform3DTest, MatrixProperty_ValidTransform_MaintainsHomogeneousLayout)
{
    Transform3D t(scaleBasis, Vector3(1.0f, 2.0f, 3.0f));
    Matrix4x4 m = t.Matrix;

    EXPECT_TRUE(m.Row0.IsEqualApprox(Vector4(2.0f, 0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(m.Row1.IsEqualApprox(Vector4(0.0f, 3.0f, 0.0f, 2.0f)));
    EXPECT_TRUE(m.Row2.IsEqualApprox(Vector4(0.0f, 0.0f, 4.0f, 3.0f)));
    EXPECT_TRUE(m.Row3.IsEqualApprox(Vector4(0.0f, 0.0f, 0.0f, 1.0f)));
}