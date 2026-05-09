#pragma once

#include <string>

#include "Basis3D.h"
#include "Matrix4x4.h"

namespace ho
{
struct AABB;
struct Sphere;

struct Transform3D final
{
    constexpr Transform3D();
    constexpr Transform3D(float scaleX,
                          float scaleY,
                          float scaleZ,
                          const Vector3& right,
                          const Vector3& up,
                          const Vector3& forward,
                          const Vector3& origin);
    constexpr Transform3D(const Basis3D& basis, const Vector3& origin);
    constexpr Transform3D(const Matrix4x4& m);
    constexpr Transform3D(const Transform3D&) = default;
    constexpr Transform3D& operator=(const Transform3D& rhs);
    ~Transform3D() = default;

    [[nodiscard]] FORCE_INLINE float GetScaleX() const;
    [[nodiscard]] FORCE_INLINE float GetScaleY() const;
    [[nodiscard]] FORCE_INLINE float GetScaleZ() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetRight() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetUp() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetForward() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetScaledRight() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetScaledUp() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetScaledForward() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetOrigin() const;
    constexpr void SetOrigin(const Vector3& origin);

    [[nodiscard]] constexpr Basis3D GetBasis() const;
    constexpr void SetBasis(const Basis3D& basis);

    [[nodiscard]] constexpr bool operator==(const Transform3D& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Transform3D& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Transform3D& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Transform3D& rhs) const;

    [[nodiscard]] constexpr bool IsOrthogonal() const;
    FORCE_INLINE void Orthogonalize();
    [[nodiscard]] FORCE_INLINE Transform3D Orthogonalized() const;

    [[nodiscard]] constexpr bool IsOrthonormal() const;
    FORCE_INLINE void Orthonormalize();
    [[nodiscard]] FORCE_INLINE Transform3D Orthonormalized() const;

    constexpr void Scale(const Vector3& scale);
    constexpr void ScaleUniform(float scale);
    FORCE_INLINE void RotateAxisAngle(const Vector3& axis, float angle);
    FORCE_INLINE void RotateEuler(float angleXRad,
                                  float angleYRad,
                                  float angleZRad,
                                  math::eEulerOrder order = math::eEulerOrder::ZYX);
    constexpr void RotateQuaternion(const Quaternion& q);
    constexpr void Translate(const Vector3& translationVector);

    [[nodiscard]] constexpr Transform3D Scaled(const Vector3& scale) const;
    [[nodiscard]] constexpr Transform3D ScaledUniform(float scale) const;
    [[nodiscard]] FORCE_INLINE Transform3D RotatedAxisAngle(const Vector3& axis, float angle) const;
    [[nodiscard]] FORCE_INLINE Transform3D RotatedEuler(float angleXRad,
                                                        float angleYRad,
                                                        float angleZRad,
                                                        math::eEulerOrder order = math::eEulerOrder::ZYX) const;
    [[nodiscard]] constexpr Transform3D RotatedQuaternion(const Quaternion& q) const;
    [[nodiscard]] constexpr Transform3D Translated(const Vector3& translationVector) const;

    constexpr void ScaleLocal(const Vector3& scale);
    constexpr void ScaleUniformLocal(float scale);
    FORCE_INLINE void RotateAxisAngleLocal(const Vector3& axis, float angle);
    FORCE_INLINE void RotateEulerLocal(float angleXRad,
                                       float angleYRad,
                                       float angleZRad,
                                       math::eEulerOrder order = math::eEulerOrder::ZYX);
    constexpr void RotateQuaternionLocal(const Quaternion& q);
    constexpr void TranslateLocal(const Vector3& translationVector);

    [[nodiscard]] constexpr Transform3D ScaledLocal(const Vector3& scale) const;
    [[nodiscard]] constexpr Transform3D ScaledUniformLocal(float scale) const;
    [[nodiscard]] FORCE_INLINE Transform3D RotatedAxisAngleLocal(const Vector3& axis, float angle) const;
    [[nodiscard]] FORCE_INLINE Transform3D RotatedEulerLocal(float angleXRad,
                                                             float angleYRad,
                                                             float angleZRad,
                                                             math::eEulerOrder order = math::eEulerOrder::ZYX) const;
    [[nodiscard]] constexpr Transform3D RotatedQuaternionLocal(const Quaternion& q) const;
    [[nodiscard]] constexpr Transform3D TranslatedLocal(const Vector3& translationVector) const;

    constexpr void Invert();
    [[nodiscard]] constexpr Transform3D Inverse() const;

    constexpr void InvertFast();
    [[nodiscard]] constexpr Transform3D InverseFast() const;

    FORCE_INLINE void LookAt(const Vector3& at, const Vector3& up, bool bFacingAt = false);
    [[nodiscard]] FORCE_INLINE Transform3D LookedAt(const Vector3& at, const Vector3& up, bool bFacingAt = false) const;

    constexpr Transform3D& operator*=(const Transform3D& rhs);
    [[nodiscard]] constexpr Transform3D operator*(const Transform3D& rhs) const;

    [[nodiscard]] constexpr Vector3 Transform(const Vector3& v) const;
    [[nodiscard]] AABB Transform(const AABB& volume) const;
    [[nodiscard]] Sphere Transform(const Sphere& volume) const;
    [[nodiscard]] constexpr Vector3 InvTransform(const Vector3& v) const;
    [[nodiscard]] constexpr Vector3 InvTransformFast(const Vector3& v) const;

    [[nodiscard]] std::string ToString() const;

    Matrix4x4 Matrix;
};

constexpr Transform3D::Transform3D()
  : Matrix(Matrix4x4::sIdentity)
{
}

constexpr Transform3D::Transform3D(float scaleX,
                                   float scaleY,
                                   float scaleZ,
                                   const Vector3& right,
                                   const Vector3& up,
                                   const Vector3& forward,
                                   const Vector3& origin)
  : Matrix(Matrix4x4(Vector4(scaleX * right),
                     Vector4(scaleY * up),
                     Vector4(scaleZ * forward),
                     Vector4(origin.X, origin.Y, origin.Z, 1.f)))
{
}

constexpr Transform3D::Transform3D(const Basis3D& basis, const Vector3& origin)
  : Matrix(Matrix4x4(Vector4(basis.Matrix.GetCol0()),
                     Vector4(basis.Matrix.GetCol1()),
                     Vector4(basis.Matrix.GetCol2()),
                     origin.ToHomogeneous()))
{
}

constexpr Transform3D::Transform3D(const Matrix4x4& m)
  : Matrix(m)
{
}

constexpr Transform3D& Transform3D::operator=(const Transform3D& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    Matrix = rhs.Matrix;
    return *this;
}

float Transform3D::GetScaleX() const
{
    return Matrix.GetCol0().Magnitude();
}

float Transform3D::GetScaleY() const
{
    return Matrix.GetCol1().Magnitude();
}

float Transform3D::GetScaleZ() const
{
    return Matrix.GetCol2().Magnitude();
}

Vector3 Transform3D::GetRight() const
{
    return Vector3(Matrix.GetCol0()).Normalized();
}

Vector3 Transform3D::GetUp() const
{
    return Vector3(Matrix.GetCol1()).Normalized();
}

Vector3 Transform3D::GetForward() const
{
    return Vector3(Matrix.GetCol2()).Normalized();
}

Vector3 Transform3D::GetScaledRight() const
{
    return Vector3(Matrix.GetCol0());
}

Vector3 Transform3D::GetScaledUp() const
{
    return Vector3(Matrix.GetCol1());
}

Vector3 Transform3D::GetScaledForward() const
{
    return Vector3(Matrix.GetCol2());
}

Vector3 Transform3D::GetOrigin() const
{
    return Vector3(Matrix.GetCol3());
}

constexpr void Transform3D::SetOrigin(const Vector3& origin)
{
    Matrix.SetCol3(origin.ToHomogeneous());
}

constexpr Basis3D Transform3D::GetBasis() const
{
    return Basis3D(Vector3(Matrix.Row0.X, Matrix.Row1.X, Matrix.Row2.X),
                   Vector3(Matrix.Row0.Y, Matrix.Row1.Y, Matrix.Row2.Y),
                   Vector3(Matrix.Row0.Z, Matrix.Row1.Z, Matrix.Row2.Z));
}

constexpr void Transform3D::SetBasis(const Basis3D& basis)
{
    Matrix.SetCol0(Vector4(basis.Matrix.GetCol0()));
    Matrix.SetCol1(Vector4(basis.Matrix.GetCol1()));
    Matrix.SetCol2(Vector4(basis.Matrix.GetCol2()));
}

constexpr bool Transform3D::operator==(const Transform3D& rhs) const
{
    return Matrix == rhs.Matrix;
}

constexpr bool Transform3D::operator!=(const Transform3D& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Transform3D::IsEqualApprox(const Transform3D& rhs) const
{
    return Matrix.IsEqualApprox(rhs.Matrix);
}

constexpr bool Transform3D::IsNotEqualApprox(const Transform3D& rhs) const
{
    return !IsEqualApprox(rhs);
}

constexpr bool Transform3D::IsOrthogonal() const
{
    return GetBasis().IsOrthogonal();
}

void Transform3D::Orthogonalize()
{
    Basis3D b = GetBasis();
    b.Orthogonalize();
    SetBasis(b);
}

Transform3D Transform3D::Orthogonalized() const
{
    Transform3D copy = *this;
    copy.Orthogonalize();
    return copy;
}

constexpr bool Transform3D::IsOrthonormal() const
{
    return GetBasis().IsOrthonormal();
}

void Transform3D::Orthonormalize()
{
    Basis3D b = GetBasis();
    b.Orthonormalize();
    SetBasis(b);
}

Transform3D Transform3D::Orthonormalized() const
{
    Transform3D copy = *this;
    copy.Orthonormalize();
    return copy;
}

constexpr void Transform3D::Scale(const Vector3& scale)
{
    Matrix.Row0 *= scale.X;
    Matrix.Row1 *= scale.Y;
    Matrix.Row2 *= scale.Z;
}

constexpr void Transform3D::ScaleUniform(float scale)
{
    Matrix.Row0 *= scale;
    Matrix.Row1 *= scale;
    Matrix.Row2 *= scale;
}

FORCE_INLINE void Transform3D::RotateAxisAngle(const Vector3& axis, float angle)
{
    const Quaternion q = Quaternion::FromAxisAngle(axis, angle);
    RotateQuaternion(q);
}

FORCE_INLINE void Transform3D::RotateEuler(float angleXRad, float angleYRad, float angleZRad, math::eEulerOrder order)
{
    const Quaternion q = Quaternion::FromEuler(angleXRad, angleYRad, angleZRad, order);
    RotateQuaternion(q);
}

constexpr void Transform3D::RotateQuaternion(const Quaternion& q)
{
    const Matrix3x3 rotationMat = Matrix3x3::FromQuaternion(q);

    for (int32_t j = 0; j < 4; ++j)
    {
        const float m0 = Matrix.Data[0][j];
        const float m1 = Matrix.Data[1][j];
        const float m2 = Matrix.Data[2][j];

        Matrix.Data[0][j] = rotationMat.Data[0][0] * m0 + rotationMat.Data[0][1] * m1 + rotationMat.Data[0][2] * m2;
        Matrix.Data[1][j] = rotationMat.Data[1][0] * m0 + rotationMat.Data[1][1] * m1 + rotationMat.Data[1][2] * m2;
        Matrix.Data[2][j] = rotationMat.Data[2][0] * m0 + rotationMat.Data[2][1] * m1 + rotationMat.Data[2][2] * m2;
    }
}

constexpr void Transform3D::Translate(const Vector3& translationVector)
{
    Matrix.Data[0][3] += translationVector.X;
    Matrix.Data[1][3] += translationVector.Y;
    Matrix.Data[2][3] += translationVector.Z;
}

constexpr Transform3D Transform3D::Scaled(const Vector3& scale) const
{
    Transform3D copy = *this;
    copy.Scale(scale);
    return copy;
}

constexpr Transform3D Transform3D::ScaledUniform(float scale) const
{
    Transform3D copy = *this;
    copy.ScaleUniform(scale);
    return copy;
}

FORCE_INLINE Transform3D Transform3D::RotatedAxisAngle(const Vector3& axis, float angle) const
{
    Transform3D copy = *this;
    copy.RotateAxisAngle(axis, angle);
    return copy;
}

FORCE_INLINE Transform3D Transform3D::RotatedEuler(float angleXRad,
                                                   float angleYRad,
                                                   float angleZRad,
                                                   math::eEulerOrder order) const
{
    Transform3D copy = *this;
    copy.RotateEuler(angleXRad, angleYRad, angleZRad, order);
    return copy;
}

constexpr Transform3D Transform3D::RotatedQuaternion(const Quaternion& q) const
{
    Transform3D copy = *this;
    copy.RotateQuaternion(q);
    return copy;
}

constexpr Transform3D Transform3D::Translated(const Vector3& translationVector) const
{
    Transform3D copy = *this;
    copy.Translate(translationVector);
    return copy;
}

constexpr void Transform3D::ScaleLocal(const Vector3& scale)
{
    Matrix.Data[0][0] *= scale.X;
    Matrix.Data[1][0] *= scale.X;
    Matrix.Data[2][0] *= scale.X;

    Matrix.Data[0][1] *= scale.Y;
    Matrix.Data[1][1] *= scale.Y;
    Matrix.Data[2][1] *= scale.Y;

    Matrix.Data[0][2] *= scale.Z;
    Matrix.Data[1][2] *= scale.Z;
    Matrix.Data[2][2] *= scale.Z;
}

constexpr void Transform3D::ScaleUniformLocal(float scale)
{
    Matrix.Data[0][0] *= scale;
    Matrix.Data[1][0] *= scale;
    Matrix.Data[2][0] *= scale;

    Matrix.Data[0][1] *= scale;
    Matrix.Data[1][1] *= scale;
    Matrix.Data[2][1] *= scale;

    Matrix.Data[0][2] *= scale;
    Matrix.Data[1][2] *= scale;
    Matrix.Data[2][2] *= scale;
}

FORCE_INLINE void Transform3D::RotateAxisAngleLocal(const Vector3& axis, float angle)
{
    const Quaternion q = Quaternion::FromAxisAngle(axis, angle);
    RotateQuaternionLocal(q);
}

FORCE_INLINE void Transform3D::RotateEulerLocal(float angleXRad,
                                                float angleYRad,
                                                float angleZRad,
                                                math::eEulerOrder order)
{
    const Quaternion q = Quaternion::FromEuler(angleXRad, angleYRad, angleZRad, order);
    RotateQuaternionLocal(q);
}

constexpr void Transform3D::RotateQuaternionLocal(const Quaternion& q)
{
    const Matrix3x3 rotationMat = Matrix3x3::FromQuaternion(q);

    for (int32_t i = 0; i < 3; ++i)
    {
        const float m0 = Matrix.Data[i][0];
        const float m1 = Matrix.Data[i][1];
        const float m2 = Matrix.Data[i][2];

        Matrix.Data[i][0] = m0 * rotationMat.Data[0][0] + m1 * rotationMat.Data[1][0] + m2 * rotationMat.Data[2][0];
        Matrix.Data[i][1] = m0 * rotationMat.Data[0][1] + m1 * rotationMat.Data[1][1] + m2 * rotationMat.Data[2][1];
        Matrix.Data[i][2] = m0 * rotationMat.Data[0][2] + m1 * rotationMat.Data[1][2] + m2 * rotationMat.Data[2][2];
    }
}

constexpr void Transform3D::TranslateLocal(const Vector3& translationVector)
{
    Matrix.Data[0][3] += Matrix.Data[0][0] * translationVector.X + Matrix.Data[0][1] * translationVector.Y +
                         Matrix.Data[0][2] * translationVector.Z;
    Matrix.Data[1][3] += Matrix.Data[1][0] * translationVector.X + Matrix.Data[1][1] * translationVector.Y +
                         Matrix.Data[1][2] * translationVector.Z;
    Matrix.Data[2][3] += Matrix.Data[2][0] * translationVector.X + Matrix.Data[2][1] * translationVector.Y +
                         Matrix.Data[2][2] * translationVector.Z;
}

constexpr Transform3D Transform3D::ScaledLocal(const Vector3& scale) const
{
    Transform3D copy = *this;
    copy.ScaleLocal(scale);
    return copy;
}

constexpr Transform3D Transform3D::ScaledUniformLocal(float scale) const
{
    Transform3D copy = *this;
    copy.ScaleUniformLocal(scale);
    return copy;
}

FORCE_INLINE Transform3D Transform3D::RotatedAxisAngleLocal(const Vector3& axis, float angle) const
{
    Transform3D copy = *this;
    copy.RotateAxisAngleLocal(axis, angle);
    return copy;
}

FORCE_INLINE Transform3D Transform3D::RotatedEulerLocal(float angleXRad,
                                                        float angleYRad,
                                                        float angleZRad,
                                                        math::eEulerOrder order) const
{
    Transform3D copy = *this;
    copy.RotateEulerLocal(angleXRad, angleYRad, angleZRad, order);
    return copy;
}

constexpr Transform3D Transform3D::RotatedQuaternionLocal(const Quaternion& q) const
{
    Transform3D copy = *this;
    copy.RotateQuaternionLocal(q);
    return copy;
}

constexpr Transform3D Transform3D::TranslatedLocal(const Vector3& translationVector) const
{
    Transform3D copy = *this;
    copy.TranslateLocal(translationVector);
    return copy;
}

constexpr void Transform3D::Invert()
{
    const float co00 = Matrix.Data[1][1] * Matrix.Data[2][2] - Matrix.Data[1][2] * Matrix.Data[2][1];
    const float co10 = Matrix.Data[1][2] * Matrix.Data[2][0] - Matrix.Data[1][0] * Matrix.Data[2][2];
    const float co20 = Matrix.Data[1][0] * Matrix.Data[2][1] - Matrix.Data[1][1] * Matrix.Data[2][0];

    const float det = Matrix.Data[0][0] * co00 + Matrix.Data[0][1] * co10 + Matrix.Data[0][2] * co20;

    if (math::IsZeroApprox(det))
    {
        return;
    }

    const float invDet = 1.0f / det;

    const float m00 = co00 * invDet;
    const float m10 = co10 * invDet;
    const float m20 = co20 * invDet;

    const float m01 = (Matrix.Data[0][2] * Matrix.Data[2][1] - Matrix.Data[0][1] * Matrix.Data[2][2]) * invDet;
    const float m11 = (Matrix.Data[0][0] * Matrix.Data[2][2] - Matrix.Data[0][2] * Matrix.Data[2][0]) * invDet;
    const float m21 = (Matrix.Data[0][1] * Matrix.Data[2][0] - Matrix.Data[0][0] * Matrix.Data[2][1]) * invDet;

    const float m02 = (Matrix.Data[0][1] * Matrix.Data[1][2] - Matrix.Data[0][2] * Matrix.Data[1][1]) * invDet;
    const float m12 = (Matrix.Data[0][2] * Matrix.Data[1][0] - Matrix.Data[0][0] * Matrix.Data[1][2]) * invDet;
    const float m22 = (Matrix.Data[0][0] * Matrix.Data[1][1] - Matrix.Data[0][1] * Matrix.Data[1][0]) * invDet;

    const float ox = -(m00 * Matrix.Data[0][3] + m01 * Matrix.Data[1][3] + m02 * Matrix.Data[2][3]);
    const float oy = -(m10 * Matrix.Data[0][3] + m11 * Matrix.Data[1][3] + m12 * Matrix.Data[2][3]);
    const float oz = -(m20 * Matrix.Data[0][3] + m21 * Matrix.Data[1][3] + m22 * Matrix.Data[2][3]);

    Matrix.Data[0][0] = m00;
    Matrix.Data[0][1] = m01;
    Matrix.Data[0][2] = m02;
    Matrix.Data[0][3] = ox;
    Matrix.Data[1][0] = m10;
    Matrix.Data[1][1] = m11;
    Matrix.Data[1][2] = m12;
    Matrix.Data[1][3] = oy;
    Matrix.Data[2][0] = m20;
    Matrix.Data[2][1] = m21;
    Matrix.Data[2][2] = m22;
    Matrix.Data[2][3] = oz;
}

constexpr Transform3D Transform3D::Inverse() const
{
    Transform3D copy = *this;
    copy.Invert();
    return copy;
}

constexpr void Transform3D::InvertFast()
{
    float temp = Matrix.Data[0][1];
    Matrix.Data[0][1] = Matrix.Data[1][0];
    Matrix.Data[1][0] = temp;
    temp = Matrix.Data[0][2];
    Matrix.Data[0][2] = Matrix.Data[2][0];
    Matrix.Data[2][0] = temp;
    temp = Matrix.Data[1][2];
    Matrix.Data[1][2] = Matrix.Data[2][1];
    Matrix.Data[2][1] = temp;

    const float tx = -Matrix.Data[0][3];
    const float ty = -Matrix.Data[1][3];
    const float tz = -Matrix.Data[2][3];

    Matrix.Data[0][3] = Matrix.Data[0][0] * tx + Matrix.Data[0][1] * ty + Matrix.Data[0][2] * tz;
    Matrix.Data[1][3] = Matrix.Data[1][0] * tx + Matrix.Data[1][1] * ty + Matrix.Data[1][2] * tz;
    Matrix.Data[2][3] = Matrix.Data[2][0] * tx + Matrix.Data[2][1] * ty + Matrix.Data[2][2] * tz;
}

constexpr Transform3D Transform3D::InverseFast() const
{
    Transform3D copy = *this;
    copy.InvertFast();
    return copy;
}

FORCE_INLINE void Transform3D::LookAt(const Vector3& at, const Vector3& up, bool bFacingAt)
{
    const Vector3 adjAt = at - GetOrigin();
    if (adjAt.IsEqualApprox(Vector3::sZero))
    {
        return;
    }
    SetBasis(GetBasis().LookedAt(adjAt, up, bFacingAt));
}

FORCE_INLINE Transform3D Transform3D::LookedAt(const Vector3& at, const Vector3& up, bool bFacingAt) const
{
    Transform3D copy = *this;
    copy.LookAt(at, up, bFacingAt);
    return copy;
}

constexpr Transform3D& Transform3D::operator*=(const Transform3D& rhs)
{
    Matrix *= rhs.Matrix;
    return *this;
}

constexpr Transform3D Transform3D::operator*(const Transform3D& rhs) const
{
    Transform3D copy = *this;
    return copy *= rhs;
}

constexpr Vector3 Transform3D::Transform(const Vector3& v) const
{
    return GetBasis().Transform(v) + GetOrigin();
}

constexpr Vector3 Transform3D::InvTransform(const Vector3& v) const
{
    const Vector3 w = v - GetOrigin();
    return GetBasis().InvTransform(w);
}

constexpr Vector3 Transform3D::InvTransformFast(const Vector3& v) const
{
    const Vector3 w = v - GetOrigin();
    return GetBasis().InvTransformFast(w);
}

} // namespace ho