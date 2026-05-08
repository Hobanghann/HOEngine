#pragma once

#include <string>

#include "Quaternion.h"
#include "Vector3.h"

namespace ho
{

struct alignas(16) Matrix3x3
{
    [[nodiscard]] static Matrix3x3 FromAxisAngle(const Vector3& axis, float angle);
    [[nodiscard]] static Matrix3x3 FromEuler(float x,
                                             float y,
                                             float z,
                                             math::eEulerOrder order = math::eEulerOrder::ZYX);
    [[nodiscard]] constexpr static Matrix3x3 FromQuaternion(const Quaternion& q);

    constexpr Matrix3x3();
    constexpr Matrix3x3(const Vector3& col0, const Vector3& col1, const Vector3& col2);
    constexpr Matrix3x3(const Matrix3x3&) = default;
    constexpr Matrix3x3& operator=(const Matrix3x3& rhs);
    ~Matrix3x3() = default;

    [[nodiscard]] constexpr Vector3 GetCol0() const;
    [[nodiscard]] constexpr Vector3 GetCol1() const;
    [[nodiscard]] constexpr Vector3 GetCol2() const;
    constexpr void SetCol0(const Vector3& col);
    constexpr void SetCol1(const Vector3& col);
    constexpr void SetCol2(const Vector3& col);

    [[nodiscard]] constexpr Matrix3x3 operator+(const Matrix3x3& rhs) const;
    constexpr Matrix3x3& operator+=(const Matrix3x3& rhs);

    [[nodiscard]] constexpr Matrix3x3 operator-(const Matrix3x3& rhs) const;
    constexpr Matrix3x3& operator-=(const Matrix3x3& rhs);

    [[nodiscard]] constexpr Matrix3x3 operator*(const Matrix3x3& rhs) const;
    constexpr Matrix3x3& operator*=(const Matrix3x3& rhs);

    [[nodiscard]] constexpr Matrix3x3 operator*(float scalar) const;
    constexpr Matrix3x3& operator*=(float scalar);

    [[nodiscard]] constexpr Matrix3x3 operator/(float scalar) const;
    constexpr Matrix3x3& operator/=(float scalar);

    [[nodiscard]] constexpr bool operator==(const Matrix3x3& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Matrix3x3& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Matrix3x3& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Matrix3x3& rhs) const;

    [[nodiscard]] constexpr bool IsOrthogonal() const;
    FORCE_INLINE void Orthogonalize();
    [[nodiscard]] FORCE_INLINE Matrix3x3 Orthogonalized() const;

    [[nodiscard]] constexpr bool IsOrthonormal() const;
    FORCE_INLINE void Orthonormalize();
    [[nodiscard]] FORCE_INLINE Matrix3x3 Orthonormalized() const;

    constexpr void Transpose();
    [[nodiscard]] constexpr Matrix3x3 Transposed() const;

    constexpr void Invert();
    [[nodiscard]] constexpr Matrix3x3 Inverse() const;

    [[nodiscard]] constexpr float Trace() const;
    [[nodiscard]] constexpr float Determinant() const;

    [[nodiscard]] std::string ToString() const;

    static const Matrix3x3 sIdentity;

    union
    {
        struct
        {
            Vector3 Row0;
            Vector3 Row1;
            Vector3 Row2;
        };

        float Data[3][4]; // with pad
    };
};

constexpr Matrix3x3::Matrix3x3()
  : Row0(Vector3())
  , Row1(Vector3())
  , Row2(Vector3())
{
}

constexpr Matrix3x3::Matrix3x3(const Vector3& col0, const Vector3& col1, const Vector3& col2)
  : Row0(Vector3(col0.X, col1.X, col2.X))
  , Row1(Vector3(col0.Y, col1.Y, col2.Y))
  , Row2(Vector3(col0.Z, col1.Z, col2.Z))
{
}

constexpr Matrix3x3& Matrix3x3::operator=(const Matrix3x3& rhs)
{
    Row0 = rhs.Row0;
    Row1 = rhs.Row1;
    Row2 = rhs.Row2;
    return *this;
}

constexpr Vector3 Matrix3x3::GetCol0() const
{
    return Vector3(Row0.X, Row1.X, Row2.X);
}

constexpr Vector3 Matrix3x3::GetCol1() const
{
    return Vector3(Row0.Y, Row1.Y, Row2.Y);
}

constexpr Vector3 Matrix3x3::GetCol2() const
{
    return Vector3(Row0.Z, Row1.Z, Row2.Z);
}

constexpr void Matrix3x3::SetCol0(const Vector3& col)
{
    Row0.X = col.X;
    Row1.X = col.Y;
    Row2.X = col.Z;
}

constexpr void Matrix3x3::SetCol1(const Vector3& col)
{
    Row0.Y = col.X;
    Row1.Y = col.Y;
    Row2.Y = col.Z;
}

constexpr void Matrix3x3::SetCol2(const Vector3& col)
{
    Row0.Z = col.X;
    Row1.Z = col.Y;
    Row2.Z = col.Z;
}

constexpr Matrix3x3 Matrix3x3::operator+(const Matrix3x3& rhs) const
{
    Matrix3x3 res;
    res.Row0 = Row0 + rhs.Row0;
    res.Row1 = Row1 + rhs.Row1;
    res.Row2 = Row2 + rhs.Row2;
    return res;
}

constexpr Matrix3x3& Matrix3x3::operator+=(const Matrix3x3& rhs)
{
    Row0 += rhs.Row0;
    Row1 += rhs.Row1;
    Row2 += rhs.Row2;
    return *this;
}

constexpr Matrix3x3 Matrix3x3::operator-(const Matrix3x3& rhs) const
{
    Matrix3x3 res;
    res.Row0 = Row0 - rhs.Row0;
    res.Row1 = Row1 - rhs.Row1;
    res.Row2 = Row2 - rhs.Row2;
    return res;
}

constexpr Matrix3x3& Matrix3x3::operator-=(const Matrix3x3& rhs)
{
    Row0 -= rhs.Row0;
    Row1 -= rhs.Row1;
    Row2 -= rhs.Row2;
    return *this;
}

constexpr Matrix3x3 Matrix3x3::operator*(const Matrix3x3& rhs) const
{
    Matrix3x3 res;

    res.Row0.X = Row0.X * rhs.Row0.X + Row0.Y * rhs.Row1.X + Row0.Z * rhs.Row2.X;
    res.Row0.Y = Row0.X * rhs.Row0.Y + Row0.Y * rhs.Row1.Y + Row0.Z * rhs.Row2.Y;
    res.Row0.Z = Row0.X * rhs.Row0.Z + Row0.Y * rhs.Row1.Z + Row0.Z * rhs.Row2.Z;

    res.Row1.X = Row1.X * rhs.Row0.X + Row1.Y * rhs.Row1.X + Row1.Z * rhs.Row2.X;
    res.Row1.Y = Row1.X * rhs.Row0.Y + Row1.Y * rhs.Row1.Y + Row1.Z * rhs.Row2.Y;
    res.Row1.Z = Row1.X * rhs.Row0.Z + Row1.Y * rhs.Row1.Z + Row1.Z * rhs.Row2.Z;

    res.Row2.X = Row2.X * rhs.Row0.X + Row2.Y * rhs.Row1.X + Row2.Z * rhs.Row2.X;
    res.Row2.Y = Row2.X * rhs.Row0.Y + Row2.Y * rhs.Row1.Y + Row2.Z * rhs.Row2.Y;
    res.Row2.Z = Row2.X * rhs.Row0.Z + Row2.Y * rhs.Row1.Z + Row2.Z * rhs.Row2.Z;

    return res;
}

constexpr Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& rhs)
{
    *this = *this * rhs;
    return *this;
}

constexpr Matrix3x3 Matrix3x3::operator*(float scalar) const
{
    Matrix3x3 res;
    res.Row0 = Row0 * scalar;
    res.Row1 = Row1 * scalar;
    res.Row2 = Row2 * scalar;
    return res;
}

constexpr Matrix3x3& Matrix3x3::operator*=(float scalar)
{
    Row0 *= scalar;
    Row1 *= scalar;
    Row2 *= scalar;
    return *this;
}

constexpr Matrix3x3 Matrix3x3::operator/(float scalar) const
{
    Matrix3x3 res;
    res.Row0 = Row0 / scalar;
    res.Row1 = Row1 / scalar;
    res.Row2 = Row2 / scalar;
    return res;
}

constexpr Matrix3x3& Matrix3x3::operator/=(float scalar)
{
    Row0 /= scalar;
    Row1 /= scalar;
    Row2 /= scalar;
    return *this;
}

constexpr bool Matrix3x3::operator==(const Matrix3x3& rhs) const
{
    return Row0 == rhs.Row0 && Row1 == rhs.Row1 && Row2 == rhs.Row2;
}

constexpr bool Matrix3x3::operator!=(const Matrix3x3& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Matrix3x3::IsEqualApprox(const Matrix3x3& rhs) const
{
    return Row0.IsEqualApprox(rhs.Row0) && Row1.IsEqualApprox(rhs.Row1) && Row2.IsEqualApprox(rhs.Row2);
}

constexpr bool Matrix3x3::IsNotEqualApprox(const Matrix3x3& rhs) const
{
    return !IsEqualApprox(rhs);
}

constexpr bool Matrix3x3::IsOrthogonal() const
{
    const Vector3 c0 = GetCol0();
    const Vector3 c1 = GetCol1();
    const Vector3 c2 = GetCol2();
    return math::IsEqualApprox(c0.Dot(c1), 0.0f, math::EPSILON_ORTHO) &&
           math::IsEqualApprox(c0.Dot(c2), 0.0f, math::EPSILON_ORTHO) &&
           math::IsEqualApprox(c1.Dot(c2), 0.0f, math::EPSILON_ORTHO);
}

void Matrix3x3::Orthogonalize()
{
    const float mag0 = GetCol0().Magnitude();
    const float mag1 = GetCol1().Magnitude();
    const float mag2 = GetCol2().Magnitude();
    Orthonormalize();
    SetCol0(GetCol0() * mag0);
    SetCol1(GetCol1() * mag1);
    SetCol2(GetCol2() * mag2);
}

Matrix3x3 Matrix3x3::Orthogonalized() const
{
    Matrix3x3 copy = *this;
    copy.Orthogonalize();
    return copy;
}

constexpr bool Matrix3x3::IsOrthonormal() const
{
    return IsOrthogonal() && GetCol0().IsUnitApprox() && GetCol1().IsUnitApprox() && GetCol2().IsUnitApprox();
}

void Matrix3x3::Orthonormalize()
{
    Vector3 x = GetCol0();
    Vector3 y = GetCol1();
    Vector3 z = GetCol2();

    x.Normalize();
    y -= x * (x.Dot(y));
    y.Normalize();
    z -= (x * (x.Dot(z)) + y * (y.Dot(z)));
    z.Normalize();

    SetCol0(x);
    SetCol1(y);
    SetCol2(z);
}

Matrix3x3 Matrix3x3::Orthonormalized() const
{
    Matrix3x3 copy = *this;
    copy.Orthonormalize();
    return copy;
}

constexpr void Matrix3x3::Transpose()
{
    float tmp = 0.0f;
    tmp = Data[0][1];
    Data[0][1] = Data[1][0];
    Data[1][0] = tmp;
    tmp = Data[0][2];
    Data[0][2] = Data[2][0];
    Data[2][0] = tmp;
    tmp = Data[1][2];
    Data[1][2] = Data[2][1];
    Data[2][1] = tmp;
}

constexpr Matrix3x3 Matrix3x3::Transposed() const
{
    return Matrix3x3(Row0, Row1, Row2);
}

constexpr void Matrix3x3::Invert()
{
    const float det = Determinant();

    const float invDet = 1.0f / det;

    const Matrix3x3& m = *this;
    Matrix3x3 adj = Matrix3x3();

    adj.Row0.X = m.Row1.Y * m.Row2.Z - m.Row1.Z * m.Row2.Y;
    adj.Row0.Y = -(m.Row0.Y * m.Row2.Z - m.Row0.Z * m.Row2.Y);
    adj.Row0.Z = m.Row0.Y * m.Row1.Z - m.Row0.Z * m.Row1.Y;

    adj.Row1.X = -(m.Row1.X * m.Row2.Z - m.Row1.Z * m.Row2.X);
    adj.Row1.Y = m.Row0.X * m.Row2.Z - m.Row0.Z * m.Row2.X;
    adj.Row1.Z = -(m.Row0.X * m.Row1.Z - m.Row0.Z * m.Row1.X);

    adj.Row2.X = m.Row1.X * m.Row2.Y - m.Row1.Y * m.Row2.X;
    adj.Row2.Y = -(m.Row0.X * m.Row2.Y - m.Row0.Y * m.Row2.X);
    adj.Row2.Z = m.Row0.X * m.Row1.Y - m.Row0.Y * m.Row1.X;

    *this = adj * invDet;
}

constexpr Matrix3x3 Matrix3x3::Inverse() const
{
    Matrix3x3 copy = *this;
    copy.Invert();
    return copy;
}

constexpr float Matrix3x3::Trace() const
{
    return Row0.X + Row1.Y + Row2.Z;
}

constexpr float Matrix3x3::Determinant() const
{
    return Row0.X * (Row1.Y * Row2.Z - Row1.Z * Row2.Y) - Row0.Y * (Row1.X * Row2.Z - Row1.Z * Row2.X) +
           Row0.Z * (Row1.X * Row2.Y - Row1.Y * Row2.X);
}

// This function for converting a quaternion to a 3x3 matrix is adapted from
// the set_quaternion() method in the Godot Engine's Basis class.
// Source: https://github.com/godotengine/godot/blob/master/core/math/basis.cpp
constexpr Matrix3x3 Matrix3x3::FromQuaternion(const Quaternion& q)
{
    Matrix3x3 m;
    const float d = q.SqrdMagnitude();
    const float s = 2.0f / d;
    const float xs = q.X * s, ys = q.Y * s, zs = q.Z * s;
    const float wx = q.W * xs, wy = q.W * ys, wz = q.W * zs;
    const float xx = q.X * xs, xy = q.X * ys, xz = q.X * zs;
    const float yy = q.Y * ys, yz = q.Y * zs, zz = q.Z * zs;
    m.SetCol0(Vector3(1.0f - (yy + zz), xy + wz, xz - wy));
    m.SetCol1(Vector3(xy - wz, 1.0f - (xx + zz), yz + wx));
    m.SetCol2(Vector3(xz + wy, yz - wx, 1.0f - (xx + yy)));
    return m;
}

INLINE constexpr Matrix3x3 Matrix3x3::sIdentity =
    Matrix3x3(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

[[nodiscard]] FORCE_INLINE constexpr Matrix3x3 operator*(float scalar, const Matrix3x3& matrix)
{
    return matrix * scalar;
}

[[nodiscard]] FORCE_INLINE constexpr Vector3 operator*(const Matrix3x3& m, const Vector3& v)
{
    return Vector3(m.Row0.Dot(v), m.Row1.Dot(v), m.Row2.Dot(v));
}

} // namespace ho
