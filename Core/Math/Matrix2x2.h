#pragma once

#include <string>

#include "Vector2.h"

namespace ho
{
struct alignas(16) Matrix2x2
{
    constexpr Matrix2x2();
    constexpr Matrix2x2(const Vector2& col0, const Vector2& col1);
    constexpr Matrix2x2(const Matrix2x2&) = default;
    constexpr Matrix2x2& operator=(const Matrix2x2& rhs);
    ~Matrix2x2() = default;

    [[nodiscard]] constexpr Vector2 GetCol0() const;
    [[nodiscard]] constexpr Vector2 GetCol1() const;
    constexpr void SetCol0(const Vector2& col);
    constexpr void SetCol1(const Vector2& col);

    [[nodiscard]] constexpr Matrix2x2 operator+(const Matrix2x2& rhs) const;
    constexpr Matrix2x2& operator+=(const Matrix2x2& rhs);

    [[nodiscard]] constexpr Matrix2x2 operator-(const Matrix2x2& rhs) const;
    constexpr Matrix2x2& operator-=(const Matrix2x2& rhs);

    [[nodiscard]] constexpr Matrix2x2 operator*(const Matrix2x2& rhs) const;
    constexpr Matrix2x2& operator*=(const Matrix2x2& rhs);

    [[nodiscard]] constexpr Matrix2x2 operator*(float scalar) const;
    constexpr Matrix2x2& operator*=(float scalar);

    [[nodiscard]] constexpr Matrix2x2 operator/(float scalar) const;
    constexpr Matrix2x2& operator/=(float scalar);

    [[nodiscard]] constexpr bool operator==(const Matrix2x2& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Matrix2x2& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Matrix2x2& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Matrix2x2& rhs) const;

    [[nodiscard]] constexpr bool IsOrthogonal() const;
    FORCE_INLINE void Orthogonalize();
    [[nodiscard]] FORCE_INLINE Matrix2x2 Orthogonalized() const;

    [[nodiscard]] constexpr bool IsOrthonormal() const;
    FORCE_INLINE void Orthonormalize();
    [[nodiscard]] FORCE_INLINE Matrix2x2 Orthonormalized() const;

    constexpr void Transpose();
    [[nodiscard]] constexpr Matrix2x2 Transposed() const;

    constexpr void Invert();
    [[nodiscard]] constexpr Matrix2x2 Inverse() const;

    [[nodiscard]] constexpr float Trace() const;

    [[nodiscard]] constexpr float Determinant() const;

    [[nodiscard]] std::string ToString() const;

    static const Matrix2x2 sIdentity;

    union
    {
        struct
        {
            Vector2 Row0;
            Vector2 Row1;
        };

        float Data[2][2];
    };
};

constexpr Matrix2x2::Matrix2x2()
  : Row0(Vector2())
  , Row1(Vector2())
{
}

constexpr Matrix2x2::Matrix2x2(const Vector2& col0, const Vector2& col1)
  : Row0(Vector2(col0.X, col1.X))
  , Row1(Vector2(col0.Y, col1.Y))
{
}

INLINE constexpr Matrix2x2 Matrix2x2::sIdentity = Matrix2x2(Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f));

constexpr Matrix2x2& Matrix2x2::operator=(const Matrix2x2& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    Row0 = rhs.Row0;
    Row1 = rhs.Row1;
    return *this;
}

constexpr Vector2 Matrix2x2::GetCol0() const
{
    return Vector2(Row0.X, Row1.X);
}

constexpr Vector2 Matrix2x2::GetCol1() const
{
    return Vector2(Row0.Y, Row1.Y);
}

constexpr void Matrix2x2::SetCol0(const Vector2& col)
{
    Row0.X = col.X;
    Row1.X = col.Y;
}

constexpr void Matrix2x2::SetCol1(const Vector2& col)
{
    Row0.Y = col.X;
    Row1.Y = col.Y;
}

constexpr Matrix2x2 Matrix2x2::operator+(const Matrix2x2& rhs) const
{
    Matrix2x2 res;
    res.Row0 = Row0 + rhs.Row0;
    res.Row1 = Row1 + rhs.Row1;
    return res;
}

constexpr Matrix2x2& Matrix2x2::operator+=(const Matrix2x2& rhs)
{
    Row0 += rhs.Row0;
    Row1 += rhs.Row1;
    return *this;
}

constexpr Matrix2x2 Matrix2x2::operator-(const Matrix2x2& rhs) const
{
    Matrix2x2 res;
    res.Row0 = Row0 - rhs.Row0;
    res.Row1 = Row1 - rhs.Row1;
    return res;
}

constexpr Matrix2x2& Matrix2x2::operator-=(const Matrix2x2& rhs)
{
    Row0 -= rhs.Row0;
    Row1 -= rhs.Row1;
    return *this;
}

constexpr Matrix2x2 Matrix2x2::operator*(const Matrix2x2& rhs) const
{
    const Vector2 rhsCol0 = rhs.GetCol0();
    const Vector2 rhsCol1 = rhs.GetCol1();

    return Matrix2x2(Vector2(Row0.Dot(rhsCol0), Row1.Dot(rhsCol0)), Vector2(Row0.Dot(rhsCol1), Row1.Dot(rhsCol1)));
}

constexpr Matrix2x2& Matrix2x2::operator*=(const Matrix2x2& rhs)
{
    const Vector2 rhsCol0 = rhs.GetCol0();
    const Vector2 rhsCol1 = rhs.GetCol1();

    const Vector2 newCol0(Vector2(Row0.Dot(rhsCol0), Row1.Dot(rhsCol0)));
    const Vector2 newCol1(Vector2(Row0.Dot(rhsCol1), Row1.Dot(rhsCol1)));

    SetCol0(newCol0);
    SetCol1(newCol1);
    return *this;
}

constexpr Matrix2x2 Matrix2x2::operator*(float scalar) const
{
    Matrix2x2 res;
    res.Row0 = Row0 * scalar;
    res.Row1 = Row1 * scalar;
    return res;
}

constexpr Matrix2x2& Matrix2x2::operator*=(float scalar)
{
    Row0 *= scalar;
    Row1 *= scalar;
    return *this;
}

constexpr Matrix2x2 Matrix2x2::operator/(float scalar) const
{
    Matrix2x2 res;
    res.Row0 = Row0 / scalar;
    res.Row1 = Row1 / scalar;
    return res;
}

constexpr Matrix2x2& Matrix2x2::operator/=(float scalar)
{
    Row0 /= scalar;
    Row1 /= scalar;
    return *this;
}

constexpr bool Matrix2x2::operator==(const Matrix2x2& rhs) const
{
    return Row0 == rhs.Row0 && Row1 == rhs.Row1;
}

constexpr bool Matrix2x2::operator!=(const Matrix2x2& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Matrix2x2::IsEqualApprox(const Matrix2x2& rhs) const
{
    return Row0.IsEqualApprox(rhs.Row0) && Row1.IsEqualApprox(rhs.Row1);
}

constexpr bool Matrix2x2::IsNotEqualApprox(const Matrix2x2& rhs) const
{
    return !IsEqualApprox(rhs);
}

constexpr bool Matrix2x2::IsOrthogonal() const
{
    return math::IsEqualApprox(GetCol0().Dot(GetCol1()), 0.0f, math::EPSILON_ORTHO);
}

void Matrix2x2::Orthogonalize()
{
    const float mag0 = GetCol0().Magnitude();
    const float mag1 = GetCol1().Magnitude();
    Orthonormalize();
    SetCol0(GetCol0() * mag0);
    SetCol1(GetCol1() * mag1);
}

Matrix2x2 Matrix2x2::Orthogonalized() const
{
    Matrix2x2 copy = *this;
    copy.Orthogonalize();
    return copy;
}

constexpr bool Matrix2x2::IsOrthonormal() const
{
    return IsOrthogonal() && GetCol0().IsUnitApprox() && GetCol1().IsUnitApprox();
}

void Matrix2x2::Orthonormalize()
{
    Vector2 x = GetCol0();
    Vector2 y = GetCol1();

    x.Normalize();
    y -= x * (x.Dot(y));
    y.Normalize();

    SetCol0(x);
    SetCol1(y);
}

Matrix2x2 Matrix2x2::Orthonormalized() const
{
    Matrix2x2 copy = *this;
    copy.Orthonormalize();
    return copy;
}

constexpr void Matrix2x2::Transpose()
{
    const float tmp = Data[0][1];
    Data[0][1] = Data[1][0];
    Data[1][0] = tmp;
}

constexpr Matrix2x2 Matrix2x2::Transposed() const
{
    return Matrix2x2(Row0, Row1);
}

constexpr void Matrix2x2::Invert()
{
    const float det = Determinant();

    const float intDet = 1.0f / det;

    const float tempX = Row0.X;
    Row0.X = Row1.Y * intDet;
    Row1.X = -Row1.X * intDet;
    Row0.Y = -Row0.Y * intDet;
    Row1.Y = tempX * intDet;
}

constexpr Matrix2x2 Matrix2x2::Inverse() const
{
    Matrix2x2 copy = *this;
    copy.Invert();
    return copy;
}

constexpr float Matrix2x2::Trace() const
{
    return Row0.X + Row1.Y;
}

constexpr float Matrix2x2::Determinant() const
{
    return Row0.X * Row1.Y - Row0.Y * Row1.X;
}

[[nodiscard]] FORCE_INLINE constexpr Matrix2x2 operator*(float scalar, const Matrix2x2& matrix)
{
    return matrix * scalar;
}

[[nodiscard]] FORCE_INLINE constexpr Vector2 operator*(const Matrix2x2& m, const Vector2& v)
{
    return Vector2(m.Row0.Dot(v), m.Row1.Dot(v));
}
} // namespace ho
