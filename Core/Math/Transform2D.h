#pragma once

#include <string>

#include "Basis2D.h"
#include "Matrix3x3.h"

namespace ho
{
struct Transform2D final
{
    [[nodiscard]] static Transform2D FromAngle(float angle);

    constexpr Transform2D();
    constexpr Transform2D(float scaleX, float scaleY, const Vector2& right, const Vector2& up, const Vector2& origin);
    constexpr Transform2D(const Basis2D& basis, const Vector2& origin);
    constexpr Transform2D(const Matrix3x3& m);
    constexpr Transform2D(const Transform2D&) = default;
    constexpr Transform2D& operator=(const Transform2D& rhs);
    ~Transform2D() = default;

    [[nodiscard]] FORCE_INLINE float GetScaleX() const;
    [[nodiscard]] FORCE_INLINE float GetScaleY() const;
    [[nodiscard]] FORCE_INLINE Vector2 GetRight() const;
    [[nodiscard]] FORCE_INLINE Vector2 GetUp() const;
    [[nodiscard]] FORCE_INLINE Vector2 GetScaledRight() const;
    [[nodiscard]] FORCE_INLINE Vector2 GetScaledUp() const;
    [[nodiscard]] FORCE_INLINE Vector2 GetOrigin() const;
    constexpr void SetOrigin(const Vector2& origin);

    [[nodiscard]] constexpr Basis2D GetBasis() const;
    constexpr void SetBasis(const Basis2D& basis);

    [[nodiscard]] constexpr bool operator==(const Transform2D& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Transform2D& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Transform2D& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Transform2D& rhs) const;

    [[nodiscard]] constexpr bool IsOrthogonal() const;
    FORCE_INLINE void Orthogonalize();
    [[nodiscard]] FORCE_INLINE Transform2D Orthogonalized() const;

    [[nodiscard]] constexpr bool IsOrthonormal() const;
    FORCE_INLINE void Orthonormalize();
    [[nodiscard]] FORCE_INLINE Transform2D Orthonormalized() const;

    constexpr void Scale(const Vector2& scale);
    constexpr void ScaleUniform(float scale);
    FORCE_INLINE void Rotate(float angle);
    constexpr void Translate(const Vector2& translationVector);

    [[nodiscard]] constexpr Transform2D Scaled(const Vector2& scale) const;
    [[nodiscard]] constexpr Transform2D ScaledUniform(float scale) const;
    [[nodiscard]] FORCE_INLINE Transform2D Rotated(float angle) const;
    [[nodiscard]] constexpr Transform2D Translated(const Vector2& translationVector) const;

    constexpr void ScaleLocal(const Vector2& scale);
    constexpr void ScaleUniformLocal(float scale);
    FORCE_INLINE void RotateLocal(float angle);
    constexpr void TranslateLocal(const Vector2& translationVector);

    [[nodiscard]] constexpr Transform2D ScaledLocal(const Vector2& scale) const;
    [[nodiscard]] constexpr Transform2D ScaledUniformLocal(float scale) const;
    [[nodiscard]] FORCE_INLINE Transform2D RotatedLocal(float angle) const;
    [[nodiscard]] constexpr Transform2D TranslatedLocal(const Vector2& translationVector) const;

    constexpr void Invert();
    [[nodiscard]] constexpr Transform2D Inverse() const;

    constexpr void InvertFast();
    [[nodiscard]] constexpr Transform2D InverseFast() const;

    FORCE_INLINE void LookAt(const Vector2& at);
    [[nodiscard]] FORCE_INLINE Transform2D LookedAt(const Vector2& at) const;

    constexpr Transform2D& operator*=(const Transform2D& rhs);
    [[nodiscard]] constexpr Transform2D operator*(const Transform2D& rhs) const;

    [[nodiscard]] constexpr Vector2 Transform(const Vector2& v) const;
    [[nodiscard]] constexpr Vector2 InvTransform(const Vector2& v) const;
    [[nodiscard]] constexpr Vector2 InvTransformFast(const Vector2& v) const;

    [[nodiscard]] std::string ToString() const;

    Matrix3x3 Matrix;
};

constexpr Transform2D::Transform2D()
  : Matrix(Matrix3x3::sIdentity)
{
}

constexpr Transform2D::Transform2D(
    float scaleX, float scaleY, const Vector2& right, const Vector2& up, const Vector2& origin)
  : Matrix(Matrix3x3(Vector3(scaleX * right), Vector3(scaleY * up), origin.ToHomogeneous()))
{
}

constexpr Transform2D::Transform2D(const Basis2D& basis, const Vector2& origin)
  : Matrix(Matrix3x3(Vector3(basis.GetScaledRight()), Vector3(basis.GetScaledUp()), origin.ToHomogeneous()))
{
}

constexpr Transform2D::Transform2D(const Matrix3x3& m)
  : Matrix(m)
{
}

constexpr Transform2D& Transform2D::operator=(const Transform2D& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    Matrix = rhs.Matrix;
    return *this;
}

float Transform2D::GetScaleX() const
{
    return Matrix.GetCol0().Magnitude();
}

float Transform2D::GetScaleY() const
{
    return Matrix.GetCol1().Magnitude();
}

Vector2 Transform2D::GetRight() const
{
    return Vector2(Matrix.GetCol0()).Normalized();
}

Vector2 Transform2D::GetUp() const
{
    return Vector2(Matrix.GetCol1()).Normalized();
}

Vector2 Transform2D::GetScaledRight() const
{
    return Vector2(Matrix.GetCol0());
}

Vector2 Transform2D::GetScaledUp() const
{
    return Vector2(Matrix.GetCol1());
}

Vector2 Transform2D::GetOrigin() const
{
    return Vector2(Matrix.GetCol2());
}

constexpr void Transform2D::SetOrigin(const Vector2& origin)
{
    Matrix.SetCol2(origin.ToHomogeneous());
}

constexpr Basis2D Transform2D::GetBasis() const
{
    return Basis2D(Vector2(Matrix.GetCol0()), Vector2(Matrix.GetCol1()));
}

constexpr void Transform2D::SetBasis(const Basis2D& basis)
{
    Matrix.SetCol0(Vector3(basis.Matrix.GetCol0()));
    Matrix.SetCol1(Vector3(basis.Matrix.GetCol1()));
}

constexpr bool Transform2D::operator==(const Transform2D& rhs) const
{
    return Matrix == rhs.Matrix;
}

constexpr bool Transform2D::operator!=(const Transform2D& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Transform2D::IsEqualApprox(const Transform2D& rhs) const
{
    return Matrix.IsEqualApprox(rhs.Matrix);
}

constexpr bool Transform2D::IsNotEqualApprox(const Transform2D& rhs) const
{
    return !(IsEqualApprox(rhs));
}

constexpr bool Transform2D::IsOrthogonal() const
{
    return GetBasis().IsOrthogonal();
}

void Transform2D::Orthogonalize()
{
    Basis2D b = GetBasis();
    b.Orthogonalize();
    SetBasis(b);
}

Transform2D Transform2D::Orthogonalized() const
{
    Transform2D copy = *this;
    copy.Orthogonalize();
    return copy;
}

constexpr bool Transform2D::IsOrthonormal() const
{
    return GetBasis().IsOrthonormal();
}

void Transform2D::Orthonormalize()
{
    Basis2D b = GetBasis();
    b.Orthonormalize();
    SetBasis(b);
}

Transform2D Transform2D::Orthonormalized() const
{
    Transform2D copy = *this;
    copy.Orthonormalize();
    return copy;
}

constexpr void Transform2D::Scale(const Vector2& scale)
{
    Matrix.Row0 *= scale.X;
    Matrix.Row1 *= scale.Y;
}

constexpr void Transform2D::ScaleUniform(float scale)
{
    Matrix.Row0 *= scale;
    Matrix.Row1 *= scale;
}

void Transform2D::Rotate(float angle)
{
    float sin = 0.f;
    float cos = 0.f;
    math::SinCos(&sin, &cos, angle);

    for (int32_t i = 0; i < 3; ++i)
    {
        const float m0 = Matrix.Data[0][i];
        const float m1 = Matrix.Data[1][i];
        Matrix.Data[0][i] = cos * m0 - sin * m1;
        Matrix.Data[1][i] = sin * m0 + cos * m1;
    }
}

constexpr void Transform2D::Translate(const Vector2& translationVector)
{
    Matrix.Data[0][2] += translationVector.X;
    Matrix.Data[1][2] += translationVector.Y;
}

constexpr Transform2D Transform2D::Scaled(const Vector2& scale) const
{
    Transform2D copy = *this;
    copy.Scale(scale);
    return copy;
}

constexpr Transform2D Transform2D::ScaledUniform(float scale) const
{
    Transform2D copy = *this;
    copy.ScaleUniform(scale);
    return copy;
}

Transform2D Transform2D::Rotated(float angle) const
{
    Transform2D copy = *this;
    copy.Rotate(angle);
    return copy;
}

constexpr Transform2D Transform2D::Translated(const Vector2& translationVector) const
{
    Transform2D copy = *this;
    copy.Translate(translationVector);
    return copy;
}

constexpr void Transform2D::ScaleLocal(const Vector2& scale)
{
    Matrix.Data[0][0] *= scale.X;
    Matrix.Data[1][0] *= scale.X;

    Matrix.Data[0][1] *= scale.Y;
    Matrix.Data[1][1] *= scale.Y;
}

constexpr void Transform2D::ScaleUniformLocal(float scale)
{
    Matrix.Data[0][0] *= scale;
    Matrix.Data[1][0] *= scale;

    Matrix.Data[0][1] *= scale;
    Matrix.Data[1][1] *= scale;
}

void Transform2D::RotateLocal(float angle)
{
    float sin = 0.f;
    float cos = 0.f;
    math::SinCos(&sin, &cos, angle);

    for (int32_t i = 0; i < 2; ++i)
    {
        const float m0 = Matrix.Data[i][0];
        const float m1 = Matrix.Data[i][1];
        Matrix.Data[i][0] = m0 * cos + m1 * sin;
        Matrix.Data[i][1] = -m0 * sin + m1 * cos;
    }
}

constexpr void Transform2D::TranslateLocal(const Vector2& translationVector)
{
    Matrix.Data[0][2] += Matrix.Data[0][0] * translationVector.X + Matrix.Data[0][1] * translationVector.Y;
    Matrix.Data[1][2] += Matrix.Data[1][0] * translationVector.X + Matrix.Data[1][1] * translationVector.Y;
}

constexpr Transform2D Transform2D::ScaledLocal(const Vector2& scale) const
{
    Transform2D copy = *this;
    copy.ScaleLocal(scale);
    return copy;
}

constexpr Transform2D Transform2D::ScaledUniformLocal(float scale) const
{
    Transform2D copy = *this;
    copy.ScaleUniformLocal(scale);
    return copy;
}

Transform2D Transform2D::RotatedLocal(float angle) const
{
    Transform2D copy = *this;
    copy.RotateLocal(angle);
    return copy;
}

constexpr Transform2D Transform2D::TranslatedLocal(const Vector2& translationVector) const
{
    Transform2D copy = *this;
    copy.TranslateLocal(translationVector);
    return copy;
}

constexpr void Transform2D::Invert()
{
    const float det = Matrix.Data[0][0] * Matrix.Data[1][1] - Matrix.Data[0][1] * Matrix.Data[1][0];

    if (math::IsZeroApprox(det))
    {
        return;
    }

    const float invDet = 1.0f / det;

    const float m00 = Matrix.Data[1][1] * invDet;
    const float m01 = -Matrix.Data[0][1] * invDet;
    const float m10 = -Matrix.Data[1][0] * invDet;
    const float m11 = Matrix.Data[0][0] * invDet;

    const float ox = -(m00 * Matrix.Data[0][2] + m01 * Matrix.Data[1][2]);
    const float oy = -(m10 * Matrix.Data[0][2] + m11 * Matrix.Data[1][2]);

    Matrix.Data[0][0] = m00;
    Matrix.Data[0][1] = m01;
    Matrix.Data[0][2] = ox;
    Matrix.Data[1][0] = m10;
    Matrix.Data[1][1] = m11;
    Matrix.Data[1][2] = oy;
}

constexpr Transform2D Transform2D::Inverse() const
{
    Transform2D copy = *this;
    copy.Invert();
    return copy;
}

constexpr void Transform2D::InvertFast()
{
    const float temp = Matrix.Data[0][1];
    Matrix.Data[0][1] = Matrix.Data[1][0];
    Matrix.Data[1][0] = temp;

    const float tx = -Matrix.Data[0][2];
    const float ty = -Matrix.Data[1][2];

    Matrix.Data[0][2] = Matrix.Data[0][0] * tx + Matrix.Data[0][1] * ty;
    Matrix.Data[1][2] = Matrix.Data[1][0] * tx + Matrix.Data[1][1] * ty;
}

constexpr Transform2D Transform2D::InverseFast() const
{
    Transform2D copy = *this;
    copy.InvertFast();
    return copy;
}

void Transform2D::LookAt(const Vector2& at)
{
    const Vector2 adjAt = at - GetOrigin();
    if (adjAt.IsEqualApprox(Vector2::sZero))
    {
        return;
    }
    SetBasis(GetBasis().LookedAt(adjAt));
}

Transform2D Transform2D::LookedAt(const Vector2& at) const
{
    Transform2D copy = *this;
    copy.LookAt(at);
    return copy;
}

constexpr Transform2D& Transform2D::operator*=(const Transform2D& rhs)
{
    Matrix = Matrix * rhs.Matrix;
    return *this;
}

constexpr Transform2D Transform2D::operator*(const Transform2D& rhs) const
{
    Transform2D copy = *this;
    return copy *= rhs;
}

constexpr Vector2 Transform2D::Transform(const Vector2& v) const
{
    return GetBasis().Transform(v) + GetOrigin();
}

constexpr Vector2 Transform2D::InvTransform(const Vector2& v) const
{
    const Vector2 w = v - GetOrigin();
    return GetBasis().InvTransform(w);
}

constexpr Vector2 Transform2D::InvTransformFast(const Vector2& v) const
{
    const Vector2 w = v - GetOrigin();
    return GetBasis().InvTransformFast(w);
}
} // namespace ho