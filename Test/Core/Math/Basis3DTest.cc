#include "Core/Math/Basis3D.h"

#include <gtest/gtest.h>

#include "Core/Math/MathFuncs.h"
#include "Core/Math/Matrix3x3.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector3.h"

using namespace ho;

class Basis3DTest : public ::testing::Test
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
    }

    Basis3D defaultBasis, scaleBasis, pitchRotBasis, yawRotBasis, rollRotBasis, nonOrthoBasis;
};

TEST_F(Basis3DTest, Constructor_DefaultAndAllParameters_InitializesMatrixCorrectly)
{
    EXPECT_EQ(defaultBasis.Matrix.Row0, Vector3(1.0f, 0.0f, 0.0f));
    EXPECT_EQ(defaultBasis.Matrix.Row1, Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_EQ(defaultBasis.Matrix.Row2, Vector3(0.0f, 0.0f, 1.0f));

    EXPECT_EQ(scaleBasis.Matrix.Row0, Vector3(2.0f, 0.0f, 0.0f));
    EXPECT_EQ(scaleBasis.Matrix.Row1, Vector3(0.0f, 3.0f, 0.0f));
    EXPECT_EQ(scaleBasis.Matrix.Row2, Vector3(0.0f, 0.0f, 4.0f));

    Matrix3x3 mat(Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 5.0f, 6.0f), Vector3(7.0f, 8.0f, 9.0f));
    Basis3D basisFromMat(mat);
    EXPECT_EQ(basisFromMat.Matrix, mat);

    Quaternion quat = Quaternion::FromAxisAngle(Vector3::sUnitY, math::PI / 4.0f);
    Basis3D basisFromQuat(quat);
    EXPECT_TRUE(basisFromQuat.IsEqualApprox(yawRotBasis));
}

TEST_F(Basis3DTest, OperatorAssign_AnotherBasis_CopiesValues)
{
    Basis3D b = scaleBasis;
    EXPECT_EQ(b, scaleBasis);
}

TEST_F(Basis3DTest, GetVectorsAndScale_ValidBasis_ReturnsCorrectDirectionsAndMagnitudes)
{
    Basis3D b = scaleBasis;
    EXPECT_EQ(b.GetRight(), Vector3(2.0f, 0.0f, 0.0f).Normalized());
    EXPECT_EQ(b.GetUp(), Vector3(0.0f, 3.0f, 0.0f).Normalized());
    EXPECT_EQ(b.GetForward(), Vector3(0.0f, 0.0f, 4.0f).Normalized());

    EXPECT_EQ(b.GetScaledRight(), Vector3(2.0f, 0.0f, 0.0f));
    EXPECT_EQ(b.GetScaledUp(), Vector3(0.0f, 3.0f, 0.0f));
    EXPECT_EQ(b.GetScaledForward(), Vector3(0.0f, 0.0f, 4.0f));

    EXPECT_NEAR(b.GetScaleX(), Vector3(2.0f, 0.0f, 0.0f).Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(b.GetScaleY(), Vector3(0.0f, 3.0f, 0.0f).Magnitude(), math::EPSILON_CMP);
    EXPECT_NEAR(b.GetScaleZ(), Vector3(0.0f, 0.0f, 4.0f).Magnitude(), math::EPSILON_CMP);
}

TEST_F(Basis3DTest, OperatorComparison_DifferentBases_ReturnsCorrectBoolean)
{
    Basis3D b1(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));
    Basis3D b2 = b1;
    Basis3D b3 = scaleBasis;

    EXPECT_TRUE(b1 == b2);
    EXPECT_FALSE(b1 != b2);
    EXPECT_TRUE(b1 != b3);
}

TEST_F(Basis3DTest, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    Basis3D b1(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));
    Basis3D b2(Vector3(1.000001f, 0.0f, 0.0f), Vector3(0.0f, 1.000001f, 0.0f), Vector3(0.0f, 0.0f, 0.999999f));
    EXPECT_TRUE(b1.IsEqualApprox(b2));
    EXPECT_FALSE(b1.IsNotEqualApprox(b2));
}

TEST_F(Basis3DTest, IsOrthogonal_ValidAndInvalidBases_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(scaleBasis.IsOrthogonal());
    EXPECT_FALSE(nonOrthoBasis.IsOrthogonal());
}

TEST_F(Basis3DTest, Orthogonalize_NonOrthogonalBasis_MakesBasisOrthogonal)
{
    Basis3D o = nonOrthoBasis.Orthogonalized();
    EXPECT_TRUE(o.IsOrthogonal());

    nonOrthoBasis.Orthogonalize();
    EXPECT_TRUE(nonOrthoBasis.IsOrthogonal());
}

TEST_F(Basis3DTest, IsOrthonormal_ValidAndInvalidBases_ReturnsCorrectBoolean)
{
    EXPECT_TRUE(yawRotBasis.IsOrthonormal());
    EXPECT_FALSE(scaleBasis.IsOrthonormal());
}

TEST_F(Basis3DTest, Orthonormalize_NonOrthonormalBasis_MakesBasisOrthonormal)
{
    Basis3D n = nonOrthoBasis.Orthonormalized();
    EXPECT_TRUE(n.IsOrthonormal());

    nonOrthoBasis.Orthonormalize();
    EXPECT_TRUE(nonOrthoBasis.IsOrthonormal());
}

TEST_F(Basis3DTest, Scale_GlobalSpace_AppliesScalingCorrectly)
{
    Basis3D b(Vector3(1.0f, 2.0f, 3.0f), Vector3(0.0f, 1.0f, 1.0f), Vector3(1.0f, 0.0f, 1.0f));

    Basis3D b1 = b;
    b1.Scale(Vector3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(scaleBasis.Matrix * b.Matrix));

    b1 = b.Scaled(Vector3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(scaleBasis.Matrix * b.Matrix));

    Matrix3x3 m_scale(Vector3(2.0f, 0.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, 0.0f, 2.0f));

    b1 = b;
    b1.ScaleUniform(2.0f);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(m_scale * b.Matrix));

    b1 = b.ScaledUniform(2.0f);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(m_scale * b.Matrix));
}

TEST_F(Basis3DTest, Scale_LocalSpace_AppliesScalingCorrectly)
{
    Basis3D b(Vector3(1.0f, 2.0f, 3.0f), Vector3(0.0f, 1.0f, 1.0f), Vector3(1.0f, 0.0f, 1.0f));
    Basis3D b1;

    b1 = b;
    b1.ScaleLocal(Vector3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * scaleBasis.Matrix));

    b1 = b.ScaledLocal(Vector3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * scaleBasis.Matrix));

    Matrix3x3 m_scale(Vector3(2.0f, 0.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, 0.0f, 2.0f));

    b1 = b;
    b1.ScaleUniformLocal(2.0f);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * m_scale));

    b1 = b.ScaledUniformLocal(2.0f);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * m_scale));
}

TEST_F(Basis3DTest, RotateEuler_GlobalSpaceVaryingOrders_AppliesRotationsCorrectly)
{
    Basis3D b(Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 5.0f, 6.0f), Vector3(7.0f, 8.0f, 9.0f));
    Basis3D b1;

    b1 = b;
    b1.RotateEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::XYZ);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(pitchRotBasis.Matrix * yawRotBasis.Matrix * rollRotBasis.Matrix * b.Matrix));

    b1 = b;
    b1.RotateEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::XZY);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(pitchRotBasis.Matrix * rollRotBasis.Matrix * yawRotBasis.Matrix * b.Matrix));

    b1 = b;
    b1.RotateEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::YZX);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(yawRotBasis.Matrix * rollRotBasis.Matrix * pitchRotBasis.Matrix * b.Matrix));

    b1 = b;
    b1.RotateEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::YXZ);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(yawRotBasis.Matrix * pitchRotBasis.Matrix * rollRotBasis.Matrix * b.Matrix));

    b1 = b;
    b1.RotateEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::ZXY);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rollRotBasis.Matrix * pitchRotBasis.Matrix * yawRotBasis.Matrix * b.Matrix));

    b1 = b;
    b1.RotateEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::ZYX);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rollRotBasis.Matrix * yawRotBasis.Matrix * pitchRotBasis.Matrix * b.Matrix));

    b1 = b.RotatedEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::XYZ);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(pitchRotBasis.Matrix * yawRotBasis.Matrix * rollRotBasis.Matrix * b.Matrix));

    b1 = b.RotatedEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::XZY);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(pitchRotBasis.Matrix * rollRotBasis.Matrix * yawRotBasis.Matrix * b.Matrix));

    b1 = b.RotatedEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::YZX);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(yawRotBasis.Matrix * rollRotBasis.Matrix * pitchRotBasis.Matrix * b.Matrix));

    b1 = b.RotatedEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::YXZ);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(yawRotBasis.Matrix * pitchRotBasis.Matrix * rollRotBasis.Matrix * b.Matrix));

    b1 = b.RotatedEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::ZXY);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rollRotBasis.Matrix * pitchRotBasis.Matrix * yawRotBasis.Matrix * b.Matrix));

    b1 = b.RotatedEuler(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::ZYX);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rollRotBasis.Matrix * yawRotBasis.Matrix * pitchRotBasis.Matrix * b.Matrix));
}

TEST_F(Basis3DTest, RotateAxisAngle_GlobalSpace_AppliesRotationCorrectly)
{
    Basis3D b(Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 3.0f, 6.0f), Vector3(7.0f, 6.0f, 9.0f));

    Vector3 axis = Vector3(1.0f, 2.0f, 3.0f);
    float angle = math::PI / 4.0f;

    Matrix3x3 rotationMat = Matrix3x3::FromAxisAngle(axis, angle);

    Basis3D b1;
    b1 = b;
    b1.RotateAxisAngle(axis, angle);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rotationMat * b.Matrix));

    b1 = b.RotatedAxisAngle(axis, angle);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rotationMat * b.Matrix));
}

TEST_F(Basis3DTest, RotateQuaternion_GlobalSpace_AppliesRotationCorrectly)
{
    Basis3D b(Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 5.0f, 6.0f), Vector3(7.0f, 8.0f, 9.0f));

    Vector3 axis = Vector3(1.0f, 2.0f, 3.0f);
    float angle = math::PI / 4.0f;
    Quaternion quat = Quaternion::FromAxisAngle(axis, angle);
    Matrix3x3 rotationMat = Matrix3x3::FromQuaternion(quat);

    Basis3D b1;
    b1 = b;
    b1.RotateQuaternion(quat);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rotationMat * b.Matrix));

    b1 = b.RotatedQuaternion(quat);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(rotationMat * b.Matrix));
}

TEST_F(Basis3DTest, RotateEuler_LocalSpaceVaryingOrders_AppliesRotationsCorrectly)
{
    Basis3D b(Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 5.0f, 6.0f), Vector3(7.0f, 8.0f, 9.0f));
    Basis3D b1;

    b1 = b;
    b1.RotateEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::XYZ);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * pitchRotBasis.Matrix * yawRotBasis.Matrix * rollRotBasis.Matrix));

    b1 = b;
    b1.RotateEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::XZY);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * pitchRotBasis.Matrix * rollRotBasis.Matrix * yawRotBasis.Matrix));

    b1 = b;
    b1.RotateEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::YZX);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * yawRotBasis.Matrix * rollRotBasis.Matrix * pitchRotBasis.Matrix));

    b1 = b;
    b1.RotateEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::YXZ);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * yawRotBasis.Matrix * pitchRotBasis.Matrix * rollRotBasis.Matrix));

    b1 = b;
    b1.RotateEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::ZXY);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * rollRotBasis.Matrix * pitchRotBasis.Matrix * yawRotBasis.Matrix));

    b1 = b;
    b1.RotateEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::ZYX);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * rollRotBasis.Matrix * yawRotBasis.Matrix * pitchRotBasis.Matrix));

    b1 = b.RotatedEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::XYZ);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * pitchRotBasis.Matrix * yawRotBasis.Matrix * rollRotBasis.Matrix));

    b1 = b.RotatedEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::XZY);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * pitchRotBasis.Matrix * rollRotBasis.Matrix * yawRotBasis.Matrix));

    b1 = b.RotatedEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::YZX);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * yawRotBasis.Matrix * rollRotBasis.Matrix * pitchRotBasis.Matrix));

    b1 = b.RotatedEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::YXZ);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * yawRotBasis.Matrix * pitchRotBasis.Matrix * rollRotBasis.Matrix));

    b1 = b.RotatedEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::ZXY);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * rollRotBasis.Matrix * pitchRotBasis.Matrix * yawRotBasis.Matrix));

    b1 = b.RotatedEulerLocal(math::PI / 4.0f, math::PI / 4.0f, math::PI / 4.0f, math::eEulerOrder::ZYX);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * rollRotBasis.Matrix * yawRotBasis.Matrix * pitchRotBasis.Matrix));
}

TEST_F(Basis3DTest, RotateAxisAngle_LocalSpace_AppliesRotationCorrectly)
{
    Basis3D b(Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 5.0f, 6.0f), Vector3(7.0f, 8.0f, 9.0f));

    Vector3 axis = Vector3(1.0f, 2.0f, 3.0f);
    float angle = math::PI / 4.0f;

    Matrix3x3 rotationMat = Matrix3x3::FromAxisAngle(axis, angle);

    Basis3D b1;
    b1 = b;
    b1.RotateAxisAngleLocal(axis, angle);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * rotationMat));

    b1 = b.RotatedAxisAngleLocal(axis, angle);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * rotationMat));
}

TEST_F(Basis3DTest, RotateQuaternion_LocalSpace_AppliesRotationCorrectly)
{
    Basis3D b(Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 5.0f, 6.0f), Vector3(7.0f, 8.0f, 9.0f));

    Vector3 axis = Vector3(1.0f, 2.0f, 3.0f);
    float angle = math::PI / 4.0f;
    Quaternion quat = Quaternion::FromAxisAngle(axis, angle);
    Matrix3x3 rotationMat = Matrix3x3::FromQuaternion(quat);

    Basis3D b1;
    b1 = b;
    b1.RotateQuaternionLocal(quat);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * rotationMat));

    b1 = b.RotatedQuaternionLocal(quat);
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(b.Matrix * rotationMat));
}

TEST_F(Basis3DTest, Inverse_GeneralBasis_ReturnsInvertedMatrix)
{
    Basis3D b1 = nonOrthoBasis;
    b1.Invert();
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(nonOrthoBasis.Matrix.Inverse()));

    b1 = nonOrthoBasis.Inverse();
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(nonOrthoBasis.Matrix.Inverse()));
}

TEST_F(Basis3DTest, InverseFast_OrthonormalBasis_ReturnsInverseUsingTranspose)
{
    Basis3D b1 = nonOrthoBasis;
    b1.InvertFast();
    EXPECT_FALSE(b1.Matrix.IsEqualApprox(nonOrthoBasis.Matrix.Inverse()));

    b1 = pitchRotBasis;
    b1.InvertFast();
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(pitchRotBasis.Matrix.Inverse()));

    b1 = pitchRotBasis.InverseFast();
    EXPECT_TRUE(b1.Matrix.IsEqualApprox(pitchRotBasis.Matrix.Inverse()));
}

TEST_F(Basis3DTest, LookAt_StandardTargetAndUp_OrientsBasisCorrectly)
{
    Vector3 at(1.0f, 2.0f, -3.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Basis3D b;
    b.LookAt(at, up);

    EXPECT_TRUE(b.IsOrthogonal());

    Vector3 normalizedAt = at.Normalized();
    EXPECT_TRUE(b.GetForward().Normalized().IsEqualApprox(-normalizedAt));

    float dotWithOriginUp = up.Normalized().Dot(b.GetUp().Normalized());
    EXPECT_GT(dotWithOriginUp, 0.0f);

    Vector3 right = b.GetRight();
    Vector3 orthoForward = right.Cross(b.GetUp()).Normalized();
    EXPECT_FLOAT_EQ(orthoForward.Dot(normalizedAt), -1.0f);
}

TEST_F(Basis3DTest, LookAt_FacingTargetFlagTrue_OrientsForwardToTarget)
{
    Vector3 at(1.0f, 2.0f, -3.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Basis3D b;
    b.LookAt(at, up, true);

    EXPECT_TRUE(b.IsOrthogonal());

    Vector3 normalizedAt = at.Normalized();
    EXPECT_TRUE(b.GetForward().Normalized().IsEqualApprox(normalizedAt));

    float dotWithOriginUp = up.Normalized().Dot(b.GetUp().Normalized());
    EXPECT_GT(dotWithOriginUp, 0.0f);

    Vector3 right = b.GetRight();
    Vector3 orthoForward = right.Cross(b.GetUp()).Normalized();
    EXPECT_TRUE(orthoForward.IsEqualApprox(b.GetForward().Normalized()));
}

TEST_F(Basis3DTest, LookAt_ZeroTarget_DoesNotChangeOrientation)
{
    Basis3D b = scaleBasis;
    Vector3 up(0.0f, 1.0f, 0.0f);

    Basis3D lookedB = b.LookedAt(Vector3::sZero, up);

    EXPECT_TRUE(b == lookedB);
}

TEST_F(Basis3DTest, LookAt_TargetCollinearWithUp_HandlesGracefully)
{
    Basis3D b = scaleBasis;

    Vector3 at(0.0f, 5.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    b.LookAt(at, up, true);

    EXPECT_TRUE(b.IsOrthogonal());

    Vector3 expectedForward = at.Normalized();
    EXPECT_TRUE(b.GetForward().Normalized().IsEqualApprox(expectedForward));
}

TEST_F(Basis3DTest, LookAt_UnnormalizedInputs_NormalizesAndOrientsCorrectly)
{
    Basis3D b;
    Vector3 at(10.0f, 20.0f, -30.0f);
    Vector3 up(0.0f, 5.0f, 0.0f);

    b.LookAt(at, up, true);

    EXPECT_TRUE(b.IsOrthogonal());

    Vector3 expectedForward = at.Normalized();
    EXPECT_TRUE(b.GetForward().IsEqualApprox(expectedForward));

    EXPECT_GT(b.GetUp().Normalized().Dot(up.Normalized()), 0.0f);
}

TEST_F(Basis3DTest, LookAt_SameDirection_DoesNotChangeOrientation)
{
    Basis3D b = scaleBasis;

    Basis3D lookedB = b.LookedAt(b.GetForward(), b.GetUp(), true);

    EXPECT_TRUE(b == lookedB);
}

TEST_F(Basis3DTest, OperatorMul_TwoBases_ReturnsCombinedTransform)
{
    Basis3D b1(Vector3(1.0f, 2.0f, 3.0f), Vector3(0.0f, 1.0f, 2.0f), Vector3(3.0f, 0.0f, 1.0f));

    Basis3D b2(Vector3(2.0f, 0.0f, 1.0f), Vector3(1.0f, 2.0f, 0.0f), Vector3(0.0f, 1.0f, 3.0f));

    EXPECT_TRUE((b1 * b2).Matrix.IsEqualApprox(b1.Matrix * b2.Matrix));
    EXPECT_TRUE((b2 * b1).Matrix.IsEqualApprox(b2.Matrix * b1.Matrix));
}

TEST_F(Basis3DTest, Transform_Vector_ReturnsCorrectlyTransformedVector)
{
    Basis3D b1(Vector3(1.0f, 2.0f, 3.0f), Vector3(0.0f, 1.0f, 2.0f), Vector3(3.0f, 0.0f, 1.0f));

    Matrix3x3 m1 = b1.Matrix;
    Vector3 v(1.0f, 1.0f, 1.0f);

    EXPECT_TRUE(b1.Transform(v).IsEqualApprox(m1 * v));
    EXPECT_TRUE(b1.InvTransform(v).IsEqualApprox(m1.Inverse() * v));
    EXPECT_FALSE(b1.InvTransformFast(v).IsEqualApprox(m1.Inverse() * v));
    EXPECT_TRUE(pitchRotBasis.InvTransformFast(v).IsEqualApprox(pitchRotBasis.Matrix.Inverse() * v));
}
