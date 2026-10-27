#pragma once

#include <string>

#include "Matrix2x2.h"

namespace ho
{
struct alignas(16) Basis2D
{
    static constexpr Basis2D FromScale(const Vector2& scale);
    static constexpr Basis2D FromAngle(float angleRad);
    static constexpr Basis2D FromScaleAngle(const Vector2& scale, float angleRad);
    static constexpr Basis2D FromMatrix(const Matrix2x2& m);

    constexpr Basis2D();
    constexpr Basis2D(const Basis2D&) = default;
    constexpr Basis2D(const Matrix2x2& m);
    constexpr Basis2D& operator=(const Basis2D& rhs);
    ~Basis2D() = default;

    [[nodiscard]] FORCE_INLINE float GetScaleX() const;
    [[nodiscard]] FORCE_INLINE float GetScaleY() const;
    [[nodiscard]] FORCE_INLINE Vector2 GetRight() const;
    [[nodiscard]] FORCE_INLINE Vector2 GetUp() const;
    [[nodiscard]] constexpr Vector2 GetScaledRight() const;
    [[nodiscard]] constexpr Vector2 GetScaledUp() const;
    [[nodiscard]] FORCE_INLINE float GetRotateAngleRad() const;

    constexpr bool operator==(const Basis2D& rhs) const;
    constexpr bool operator!=(const Basis2D& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Basis2D& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Basis2D& rhs) const;

    [[nodiscard]] constexpr bool IsOrthogonal() const;
    FORCE_INLINE void Orthogonalize();
    [[nodiscard]] FORCE_INLINE Basis2D Orthogonalized() const;

    [[nodiscard]] constexpr bool IsOrthonormal() const;
    FORCE_INLINE void Orthonormalize();
    [[nodiscard]] FORCE_INLINE Basis2D Orthonormalized() const;

    constexpr void Scale(const Vector2& scale);
    [[nodiscard]] constexpr Basis2D Scaled(const Vector2& scale) const;

    constexpr void ScaleLocal(const Vector2& scale);
    [[nodiscard]] constexpr Basis2D ScaledLocal(const Vector2& scale) const;

    constexpr void ScaleUniform(float scale);
    [[nodiscard]] constexpr Basis2D ScaledUniform(float scale) const;

    constexpr void ScaleUniformLocal(float scale);
    [[nodiscard]] constexpr Basis2D ScaledUniformLocal(float scale) const;

    void Rotate(float angleRad);
    [[nodiscard]] Basis2D Rotated(float angleRad) const;

    void RotateLocal(float angleRad);
    [[nodiscard]] Basis2D RotatedLocal(float angleRad) const;

    constexpr void Invert();
    [[nodiscard]] constexpr Basis2D Inverse() const;

    constexpr void InvertFast();
    [[nodiscard]] constexpr Basis2D InverseFast() const;

    void LookAt(const Vector2& at);
    [[nodiscard]] Basis2D LookedAt(const Vector2& at) const;

    constexpr Basis2D& operator*=(const Basis2D& rhs);
    [[nodiscard]] constexpr Basis2D operator*(const Basis2D& rhs) const;

    [[nodiscard]] constexpr Vector2 Transform(const Vector2& v) const;
    [[nodiscard]] constexpr Vector2 InvTransform(const Vector2& v) const;
    [[nodiscard]] constexpr Vector2 InvTransformFast(const Vector2& v) const;

    [[nodiscard]] std::string ToString() const;

    Matrix2x2 Matrix;
};

constexpr Basis2D Basis2D::FromScale(const Vector2& scale)

{
    return Basis2D::FromMatrix(Matrix2x2(scale.X * Vector2::sUnitX, scale.Y * Vector2::sUnitY));
}

constexpr Basis2D Basis2D::FromAngle(float angleRad)

{
    float sin = 0.0f;
    float cos = 0.0f;
    math::SinCos(&sin, &cos, angleRad);
    return Basis2D(Matrix2x2(Vector2(cos, sin), Vector2(-sin, cos)));
}

constexpr Basis2D Basis2D::FromScaleAngle(const Vector2& scale, float angleRad)
{
    float sin = 0.0f;
    float cos = 0.0f;
    math::SinCos(&sin, &cos, angleRad);
    return Basis2D(Matrix2x2(scale.X * Vector2(cos, sin), scale.Y * Vector2(-sin, cos)));
}

constexpr Basis2D Basis2D::FromMatrix(const Matrix2x2& mat)
{
    return Basis2D(mat);
}

constexpr Basis2D::Basis2D()
  : Matrix(Matrix2x2::sIdentity)
{
}

constexpr Basis2D::Basis2D(const Matrix2x2& m)
  : Matrix(m)
{
}

constexpr Basis2D& Basis2D::operator=(const Basis2D& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    Matrix = rhs.Matrix;
    return *this;
}

float Basis2D::GetScaleX() const
{
    return Matrix.GetCol0().Magnitude();
}

float Basis2D::GetScaleY() const
{
    return Matrix.GetCol1().Magnitude();
}

Vector2 Basis2D::GetRight() const
{
    return Matrix.GetCol0().Normalized();
}

Vector2 Basis2D::GetUp() const
{
    return Matrix.GetCol1().Normalized();
}

constexpr Vector2 Basis2D::GetScaledRight() const
{
    return Matrix.GetCol0();
}

constexpr Vector2 Basis2D::GetScaledUp() const
{
    return Matrix.GetCol1();
}

float Basis2D::GetRotateAngleRad() const
{
    const Vector2 right = GetRight();
    return math::Atan2(right.Y, right.X);
}

constexpr bool Basis2D::operator==(const Basis2D& rhs) const
{
    return Matrix == rhs.Matrix;
}

constexpr bool Basis2D::operator!=(const Basis2D& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Basis2D::IsEqualApprox(const Basis2D& rhs) const
{
    return Matrix.IsEqualApprox(rhs.Matrix);
}

constexpr bool Basis2D::IsNotEqualApprox(const Basis2D& rhs) const
{
    return !IsEqualApprox(rhs);
}

constexpr bool Basis2D::IsOrthogonal() const
{
    return Matrix.IsOrthogonal();
}

void Basis2D::Orthogonalize()
{
    Matrix.Orthogonalize();
}

Basis2D Basis2D::Orthogonalized() const
{
    Basis2D copy = *this;
    copy.Orthogonalize();
    return copy;
}

constexpr bool Basis2D::IsOrthonormal() const
{
    return Matrix.IsOrthonormal();
}

void Basis2D::Orthonormalize()
{
    Matrix.Orthonormalize();
}

Basis2D Basis2D::Orthonormalized() const
{
    Basis2D copy = *this;
    copy.Orthonormalize();
    return copy;
}

constexpr void Basis2D::Scale(const Vector2& scale)
{
    Matrix.Row0 *= scale.X;
    Matrix.Row1 *= scale.Y;
}

constexpr Basis2D Basis2D::Scaled(const Vector2& scale) const
{
    Basis2D copy = *this;
    copy.Scale(scale);
    return copy;
}

constexpr void Basis2D::ScaleLocal(const Vector2& scale)
{
    Matrix.Row0 *= scale;
    Matrix.Row1 *= scale;
}

constexpr Basis2D Basis2D::ScaledLocal(const Vector2& scale) const
{
    Basis2D copy = *this;
    copy.ScaleLocal(scale);
    return copy;
}

constexpr void Basis2D::ScaleUniform(float scale)
{
    Matrix *= scale;
}

constexpr Basis2D Basis2D::ScaledUniform(float scale) const
{
    Basis2D copy = *this;
    copy.ScaleUniform(scale);
    return copy;
}

constexpr void Basis2D::ScaleUniformLocal(float scale)
{
    Matrix.Row0 *= scale;
    Matrix.Row1 *= scale;
}

constexpr Basis2D Basis2D::ScaledUniformLocal(float scale) const
{
    Basis2D copy = *this;
    copy.ScaleUniformLocal(scale);
    return copy;
}

constexpr void Basis2D::Invert()
{
    Matrix.Invert();
}

constexpr Basis2D Basis2D::Inverse() const
{
    Basis2D copy = *this;
    copy.Invert();
    return copy;
}

constexpr void Basis2D::InvertFast()
{
    Matrix.Transpose();
}

constexpr Basis2D Basis2D::InverseFast() const
{
    Basis2D copy = *this;
    copy.InvertFast();
    return copy;
}

constexpr Basis2D& Basis2D::operator*=(const Basis2D& rhs)
{
    Matrix *= rhs.Matrix;
    return *this;
}

constexpr Basis2D Basis2D::operator*(const Basis2D& rhs) const
{
    Basis2D result = *this;
    result *= rhs;
    return result;
}

constexpr Vector2 Basis2D::Transform(const Vector2& v) const
{
    return Matrix * v;
}

constexpr Vector2 Basis2D::InvTransform(const Vector2& v) const
{
    return Inverse().Transform(v);
}

constexpr Vector2 Basis2D::InvTransformFast(const Vector2& v) const
{
    return InverseFast().Transform(v);
}
} // namespace ho