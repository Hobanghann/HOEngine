#pragma once

#include <string>

#include "Matrix2x2.h"

namespace ho
{
struct alignas(16) Basis2D
{
    [[nodiscard]] static Basis2D FromAngle(float angle);

    constexpr Basis2D();
    constexpr Basis2D(const Basis2D&) = default;
    constexpr Basis2D(const Vector2& scaledRight, const Vector2& scaledUp);
    constexpr Basis2D(float scaleX, float scaleY, const Vector2& right, const Vector2& up);
    constexpr explicit Basis2D(const Matrix2x2& m);
    constexpr Basis2D& operator=(const Basis2D& rhs);
    ~Basis2D() = default;

    [[nodiscard]] FORCE_INLINE float GetScaleX() const;
    [[nodiscard]] FORCE_INLINE float GetScaleY() const;
    [[nodiscard]] FORCE_INLINE Vector2 GetRight() const;
    [[nodiscard]] FORCE_INLINE Vector2 GetUp() const;
    [[nodiscard]] constexpr Vector2 GetScaledRight() const;
    [[nodiscard]] constexpr Vector2 GetScaledUp() const;

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

    void Rotate(float angle);
    [[nodiscard]] Basis2D Rotated(float angle) const;

    void RotateLocal(float angle);
    [[nodiscard]] Basis2D RotatedLocal(float angle) const;

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

constexpr Basis2D::Basis2D()
  : Matrix(Matrix2x2::sIdentity)
{
}

constexpr Basis2D::Basis2D(const Vector2& scaledRight, const Vector2& scaledUp)
  : Matrix(Matrix2x2(scaledRight, scaledUp))
{
}

constexpr Basis2D::Basis2D(float scaleX, float scaleY, const Vector2& right, const Vector2& up)
  : Matrix(Matrix2x2(scaleX * right, scaleY * up))
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