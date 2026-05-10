#pragma once

#include <string>

#include "Matrix3x3.h"
#include "Quaternion.h"

namespace ho
{
struct Basis3D
{
    [[nodiscard]] static Basis3D FromAxisAngle(const Vector3& axis, float angle);
    [[nodiscard]] static Basis3D FromEuler(float angleXRad,
                                           float angleYRad,
                                           float angleZRad,
                                           math::eEulerOrder order = math::eEulerOrder::YXZ);

    constexpr Basis3D();
    constexpr Basis3D(const Basis3D& transform) = default;
    constexpr Basis3D(const Vector3& scaledRight, const Vector3& scaledUp, const Vector3& scaledForward);
    constexpr Basis3D(
        float scaleX, float scaleY, float scaleZ, const Vector3& right, const Vector3& up, const Vector3& forward);
    constexpr explicit Basis3D(const Matrix3x3& m);
    constexpr explicit Basis3D(const Quaternion& q);
    constexpr Basis3D& operator=(const Basis3D& rhs);
    ~Basis3D() = default;

    [[nodiscard]] FORCE_INLINE float GetScaleX() const;
    [[nodiscard]] FORCE_INLINE float GetScaleY() const;
    [[nodiscard]] FORCE_INLINE float GetScaleZ() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetRight() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetUp() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetForward() const;
    [[nodiscard]] constexpr Vector3 GetScaledRight() const;
    [[nodiscard]] constexpr Vector3 GetScaledUp() const;
    [[nodiscard]] constexpr Vector3 GetScaledForward() const;

    [[nodiscard]] constexpr bool operator==(const Basis3D& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Basis3D& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Basis3D& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Basis3D& rhs) const;

    [[nodiscard]] constexpr bool IsOrthogonal() const;
    FORCE_INLINE void Orthogonalize();
    [[nodiscard]] FORCE_INLINE Basis3D Orthogonalized() const;

    [[nodiscard]] constexpr bool IsOrthonormal() const;
    FORCE_INLINE void Orthonormalize();
    [[nodiscard]] FORCE_INLINE Basis3D Orthonormalized() const;

    constexpr void Scale(const Vector3& scale);
    [[nodiscard]] constexpr Basis3D Scaled(const Vector3& scale) const;

    constexpr void ScaleLocal(const Vector3& scale);
    [[nodiscard]] constexpr Basis3D ScaledLocal(const Vector3& scale) const;

    constexpr void ScaleUniform(float scale);
    [[nodiscard]] constexpr Basis3D ScaledUniform(float scale) const;

    constexpr void ScaleUniformLocal(float scale);
    [[nodiscard]] constexpr Basis3D ScaledUniformLocal(float scale) const;

    void RotateEuler(float angleXRad,
                     float angleYRad,
                     float angleZRad,
                     math::eEulerOrder order = math::eEulerOrder::YXZ);
    void RotateAxisAngle(const Vector3& axis, float angle);
    constexpr void RotateQuaternion(const Quaternion& q);
    [[nodiscard]] Basis3D RotatedEuler(float angleXRad,
                                       float angleYRad,
                                       float angleZRad,
                                       math::eEulerOrder order = math::eEulerOrder::YXZ) const;
    [[nodiscard]] Basis3D RotatedAxisAngle(const Vector3& axis, float angle) const;
    [[nodiscard]] constexpr Basis3D RotatedQuaternion(const Quaternion& q) const;

    void RotateEulerLocal(float angleXRad,
                          float angleYRad,
                          float angleZRad,
                          math::eEulerOrder order = math::eEulerOrder::YXZ);
    void RotateAxisAngleLocal(const Vector3& axis, float angle);
    constexpr void RotateQuaternionLocal(const Quaternion& q);
    [[nodiscard]] Basis3D RotatedEulerLocal(float angleXRad,
                                            float angleYRad,
                                            float angleZRad,
                                            math::eEulerOrder order = math::eEulerOrder::YXZ) const;
    [[nodiscard]] Basis3D RotatedAxisAngleLocal(const Vector3& axis, float angle) const;
    [[nodiscard]] constexpr Basis3D RotatedQuaternionLocal(const Quaternion& q) const;

    constexpr void Invert();
    [[nodiscard]] constexpr Basis3D Inverse() const;

    constexpr void InvertFast();
    [[nodiscard]] constexpr Basis3D InverseFast() const;

    void LookAt(const Vector3& at, const Vector3& up, bool bFacingAt = false);
    [[nodiscard]] Basis3D LookedAt(const Vector3& at, const Vector3& up, bool bFacingAt = false) const;

    constexpr Basis3D& operator*=(const Basis3D& rhs);
    [[nodiscard]] constexpr Basis3D operator*(const Basis3D& rhs) const;

    [[nodiscard]] FORCE_INLINE constexpr Vector3 Transform(const Vector3& v) const;
    [[nodiscard]] FORCE_INLINE constexpr Vector3 InvTransform(const Vector3& v) const;
    [[nodiscard]] FORCE_INLINE constexpr Vector3 InvTransformFast(const Vector3& v) const;

    [[nodiscard]] std::string ToString() const;

    Matrix3x3 Matrix;
};

constexpr Basis3D::Basis3D()
  : Matrix(Matrix3x3::sIdentity)
{
}

constexpr Basis3D::Basis3D(const Vector3& scaledRight, const Vector3& scaledUp, const Vector3& scaledForward)
  : Matrix(Matrix3x3(scaledRight, scaledUp, scaledForward))
{
}

constexpr Basis3D::Basis3D(
    float scaleX, float scaleY, float scaleZ, const Vector3& right, const Vector3& up, const Vector3& forward)
  : Matrix(Matrix3x3(scaleX * right, scaleY * up, scaleZ * forward))
{
}

constexpr Basis3D::Basis3D(const Matrix3x3& m)
  : Matrix(m)
{
}

constexpr Basis3D::Basis3D(const Quaternion& q)
  : Matrix(Matrix3x3::FromQuaternion(q))
{
}

constexpr Basis3D& Basis3D::operator=(const Basis3D& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    Matrix = rhs.Matrix;
    return *this;
}

float Basis3D::GetScaleX() const
{
    return Matrix.GetCol0().Magnitude();
}

float Basis3D::GetScaleY() const
{
    return Matrix.GetCol1().Magnitude();
}

float Basis3D::GetScaleZ() const
{
    return Matrix.GetCol2().Magnitude();
}

Vector3 Basis3D::GetRight() const
{
    return Matrix.GetCol0().Normalized();
}

Vector3 Basis3D::GetUp() const
{
    return Matrix.GetCol1().Normalized();
}

Vector3 Basis3D::GetForward() const
{
    return Matrix.GetCol2().Normalized();
}

constexpr Vector3 Basis3D::GetScaledRight() const
{
    return Matrix.GetCol0();
}

constexpr Vector3 Basis3D::GetScaledUp() const
{
    return Matrix.GetCol1();
}

constexpr Vector3 Basis3D::GetScaledForward() const
{
    return Matrix.GetCol2();
}

constexpr bool Basis3D::operator==(const Basis3D& rhs) const
{
    return Matrix == rhs.Matrix;
}

constexpr bool Basis3D::operator!=(const Basis3D& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Basis3D::IsEqualApprox(const Basis3D& rhs) const
{
    return Matrix.IsEqualApprox(rhs.Matrix);
}

constexpr bool Basis3D::IsNotEqualApprox(const Basis3D& rhs) const
{
    return !IsEqualApprox(rhs);
}

constexpr bool Basis3D::IsOrthogonal() const
{
    return Matrix.IsOrthogonal();
}

void Basis3D::Orthogonalize()
{
    Matrix.Orthogonalize();
}

Basis3D Basis3D::Orthogonalized() const
{
    Basis3D copy = *this;
    copy.Orthogonalize();
    return copy;
}

constexpr bool Basis3D::IsOrthonormal() const
{
    return Matrix.IsOrthonormal();
}

void Basis3D::Orthonormalize()
{
    Matrix.Orthonormalize();
}

Basis3D Basis3D::Orthonormalized() const
{
    Basis3D copy = *this;
    copy.Orthonormalize();
    return copy;
}

constexpr void Basis3D::Scale(const Vector3& scale)
{
    Matrix.Row0 *= scale.X;
    Matrix.Row1 *= scale.Y;
    Matrix.Row2 *= scale.Z;
}

constexpr Basis3D Basis3D::Scaled(const Vector3& scale) const
{
    Basis3D copy = *this;
    copy.Scale(scale);
    return copy;
}

constexpr void Basis3D::ScaleLocal(const Vector3& scale)
{
    Matrix.Row0 *= scale;
    Matrix.Row1 *= scale;
    Matrix.Row2 *= scale;
}

constexpr Basis3D Basis3D::ScaledLocal(const Vector3& scale) const
{
    Basis3D copy = *this;
    copy.ScaleLocal(scale);
    return copy;
}

constexpr void Basis3D::ScaleUniform(float scale)
{
    Matrix *= scale;
}

constexpr Basis3D Basis3D::ScaledUniform(float scale) const
{
    Basis3D copy = *this;
    copy.ScaleUniform(scale);
    return copy;
}

constexpr void Basis3D::ScaleUniformLocal(float scale)
{
    Matrix.Row0 *= scale;
    Matrix.Row1 *= scale;
    Matrix.Row2 *= scale;
}

constexpr Basis3D Basis3D::ScaledUniformLocal(float scale) const
{
    Basis3D copy = *this;
    copy.ScaleUniformLocal(scale);
    return copy;
}

constexpr void Basis3D::RotateQuaternion(const Quaternion& q)
{
    HO_ASSERT(q.IsUnitApprox(), "Quaternion must be unit.");

    const Vector3 scaledRight = GetScaledRight();
    const Vector3 scaledUp = GetScaledUp();
    const Vector3 scaledForward = GetScaledForward();

    Matrix.SetCol0(q.Transform(scaledRight));
    Matrix.SetCol1(q.Transform(scaledUp));
    Matrix.SetCol2(q.Transform(scaledForward));
}

constexpr Basis3D Basis3D::RotatedQuaternion(const Quaternion& q) const
{
    Basis3D copy = *this;
    copy.RotateQuaternion(q);
    return copy;
}

constexpr void Basis3D::RotateQuaternionLocal(const Quaternion& q)
{
    HO_ASSERT(q.IsUnitApprox(), "Quaternion must be unit.");

    const Matrix3x3 rm = Matrix3x3::FromQuaternion(q);
    Matrix = Matrix * rm;
}

constexpr Basis3D Basis3D::RotatedQuaternionLocal(const Quaternion& q) const
{
    Basis3D copy = *this;
    copy.RotateQuaternionLocal(q);
    return copy;
}

constexpr void Basis3D::Invert()
{
    Matrix.Invert();
}

constexpr Basis3D Basis3D::Inverse() const
{
    Basis3D copy = *this;
    copy.Invert();
    return copy;
}

constexpr void Basis3D::InvertFast()
{
    Matrix.Transpose();
}

constexpr Basis3D Basis3D::InverseFast() const
{
    Basis3D copy = *this;
    copy.InvertFast();
    return copy;
}

constexpr Basis3D& Basis3D::operator*=(const Basis3D& rhs)
{
    Matrix *= rhs.Matrix;
    return *this;
}

constexpr Basis3D Basis3D::operator*(const Basis3D& rhs) const
{
    Basis3D copy = *this;
    return copy *= rhs;
}

constexpr Vector3 Basis3D::Transform(const Vector3& v) const
{
    return Matrix * v;
}

constexpr Vector3 Basis3D::InvTransform(const Vector3& v) const
{
    return Inverse().Transform(v);
}

constexpr Vector3 Basis3D::InvTransformFast(const Vector3& v) const
{
    return InverseFast().Transform(v);
}
} // namespace ho
