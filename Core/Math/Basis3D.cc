#include "Basis3D.h"

namespace ho
{
Basis3D Basis3D::FromAxisAngle(const Vector3& axis, float angle)
{
    return Basis3D(Quaternion::FromAxisAngle(axis, angle));
}

Basis3D Basis3D::FromEuler(float angleXRad, float angleYRad, float angleZRad, math::eEulerOrder order)
{
    return Basis3D(Matrix3x3::FromEuler(angleXRad, angleYRad, angleZRad, order));
}

void Basis3D::RotateEuler(float angleXRad, float angleYRad, float angleZRad, math::eEulerOrder order)
{
    const Matrix3x3 rotationMat = Matrix3x3::FromEuler(angleXRad, angleYRad, angleZRad, order);
    Matrix = rotationMat * Matrix;
}

void Basis3D::RotateAxisAngle(const Vector3& axis, float angle)
{
    const Quaternion rotationQuat = Quaternion::FromAxisAngle(axis, angle);
    RotateQuaternion(rotationQuat);
}

Basis3D Basis3D::RotatedEuler(float angleXRad, float angleYRad, float angleZRad, math::eEulerOrder order) const
{
    Basis3D copy = *this;
    copy.RotateEuler(angleXRad, angleYRad, angleZRad, order);
    return copy;
}

Basis3D Basis3D::RotatedAxisAngle(const Vector3& axis, float angle) const
{
    Basis3D copy = *this;
    copy.RotateAxisAngle(axis, angle);
    return copy;
}

void Basis3D::RotateEulerLocal(float angleXRad, float angleYRad, float angleZRad, math::eEulerOrder order)
{
    const Matrix3x3 rotationMat = Matrix3x3::FromEuler(angleXRad, angleYRad, angleZRad, order);
    Matrix = Matrix * rotationMat;
}

void Basis3D::RotateAxisAngleLocal(const Vector3& axis, float angle)
{
    const Quaternion rotationQuat = Quaternion::FromAxisAngle(axis, angle);
    RotateQuaternionLocal(rotationQuat);
}

Basis3D Basis3D::RotatedEulerLocal(float angleXRad, float angleYRad, float angleZRad, math::eEulerOrder order) const
{
    Basis3D copy = *this;
    copy.RotateEulerLocal(angleXRad, angleYRad, angleZRad, order);
    return copy;
}

Basis3D Basis3D::RotatedAxisAngleLocal(const Vector3& axis, float angle) const
{
    Basis3D copy = *this;
    copy.RotateAxisAngleLocal(axis, angle);
    return copy;
}

void Basis3D::LookAt(const Vector3& at, const Vector3& up, bool bFacingAt)
{
    if (at.IsEqualApprox(Vector3::sZero))
    {
        return;
    }

    const float magX = GetScaleX();
    const float magY = GetScaleY();
    const float magZ = GetScaleZ();

    Vector3 forward = at.Normalized();
    if (!bFacingAt)
    {
        forward = -forward;
    }

    Vector3 right = up.Cross(forward);
    // Check up and foward are parallel
    if (right.IsEqualApprox(Vector3::sZero))
    {
        if (math::Abs(up.X) <= math::Abs(up.Y) && math::Abs(up.X) <= math::Abs(up.Z))
        {
            right = up.Cross(Vector3(1.0f, 0.0f, 0.0f));
        }
        else
        {
            right = up.Cross(Vector3(0.0f, 1.0f, 0.0f));
        }
    }
    right.Normalize();

    const Vector3 newUp = forward.Cross(right);

    Matrix.SetCol0(right * magX);
    Matrix.SetCol1(newUp * magY);
    Matrix.SetCol2(forward * magZ);
}

Basis3D Basis3D::LookedAt(const Vector3& at, const Vector3& up, bool bFacingAt) const
{
    Basis3D copy = *this;
    copy.LookAt(at, up, bFacingAt);
    return copy;
}

std::string Basis3D::ToString() const
{
    char buf[200];
    snprintf(buf,
             sizeof(buf),
             "Basis3D Right: %s, Up: %s, Forward: %s",
             GetRight().ToString().c_str(),
             GetUp().ToString().c_str(),
             GetForward().ToString().c_str());
    return buf;
}
} // namespace ho