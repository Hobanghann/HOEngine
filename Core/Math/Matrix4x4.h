#pragma once

#include <string>

#include "Vector4.h"

namespace ho
{
struct Matrix3x3;
struct Vector3;

struct alignas(16) Matrix4x4
{
    constexpr Matrix4x4();
    constexpr Matrix4x4(const Vector4& col0, const Vector4& col1, const Vector4& col2, const Vector4& col3);
    constexpr Matrix4x4(const Matrix4x4&) = default;
    constexpr Matrix4x4& operator=(const Matrix4x4& rhs);
    ~Matrix4x4() = default;

    [[nodiscard]] constexpr Vector4 GetCol0() const;
    [[nodiscard]] constexpr Vector4 GetCol1() const;
    [[nodiscard]] constexpr Vector4 GetCol2() const;
    [[nodiscard]] constexpr Vector4 GetCol3() const;
    constexpr void SetCol0(const Vector4& col);
    constexpr void SetCol1(const Vector4& col);
    constexpr void SetCol2(const Vector4& col);
    constexpr void SetCol3(const Vector4& col);

    [[nodiscard]] constexpr Matrix4x4 operator+(const Matrix4x4& rhs) const;
    constexpr Matrix4x4& operator+=(const Matrix4x4& rhs);

    [[nodiscard]] constexpr Matrix4x4 operator-(const Matrix4x4& rhs) const;
    constexpr Matrix4x4& operator-=(const Matrix4x4& rhs);

    [[nodiscard]] constexpr Matrix4x4 operator*(const Matrix4x4& rhs) const;
    constexpr Matrix4x4& operator*=(const Matrix4x4& rhs);

    [[nodiscard]] constexpr Matrix4x4 operator*(float scalar) const;
    constexpr Matrix4x4& operator*=(float scalar);

    [[nodiscard]] constexpr Matrix4x4 operator/(float scalar) const;
    constexpr Matrix4x4& operator/=(float scalar);

    [[nodiscard]] constexpr bool operator==(const Matrix4x4& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Matrix4x4& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Matrix4x4& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Matrix4x4& rhs) const;

    [[nodiscard]] constexpr bool IsOrthogonal() const;
    FORCE_INLINE void Orthogonalize();
    [[nodiscard]] FORCE_INLINE Matrix4x4 Orthogonalized() const;

    [[nodiscard]] constexpr bool IsOrthonormal() const;
    FORCE_INLINE void Orthonormalize();
    [[nodiscard]] FORCE_INLINE Matrix4x4 Orthonormalized() const;

    constexpr void Transpose();
    [[nodiscard]] constexpr Matrix4x4 Transposed() const;

    constexpr void Invert();
    [[nodiscard]] constexpr Matrix4x4 Inverse() const;

    [[nodiscard]] constexpr float Trace() const;
    [[nodiscard]] constexpr float Determinant() const;

    [[nodiscard]] Matrix3x3 ToMatrix3x3() const;
    [[nodiscard]] std::string ToString() const;

    static const Matrix4x4 sIdentity;

    union
    {
        struct
        {
            Vector4 Row0;
            Vector4 Row1;
            Vector4 Row2;
            Vector4 Row3;
        };

        float Data[4][4] = {};
    };
};

constexpr Matrix4x4::Matrix4x4()
  : Row0(Vector4())
  , Row1(Vector4())
  , Row2(Vector4())
  , Row3(Vector4())
{
}

constexpr Matrix4x4::Matrix4x4(const Vector4& col0, const Vector4& col1, const Vector4& col2, const Vector4& col3)
  : Row0(Vector4(col0.X, col1.X, col2.X, col3.X))
  , Row1(Vector4(col0.Y, col1.Y, col2.Y, col3.Y))
  , Row2(Vector4(col0.Z, col1.Z, col2.Z, col3.Z))
  , Row3(Vector4(col0.W, col1.W, col2.W, col3.W))
{
}

constexpr Matrix4x4& Matrix4x4::operator=(const Matrix4x4& rhs)
{
    Row0 = rhs.Row0;
    Row1 = rhs.Row1;
    Row2 = rhs.Row2;
    Row3 = rhs.Row3;
    return *this;
}

constexpr Vector4 Matrix4x4::GetCol0() const
{
    return Vector4(Row0.X, Row1.X, Row2.X, Row3.X);
}

constexpr Vector4 Matrix4x4::GetCol1() const
{
    return Vector4(Row0.Y, Row1.Y, Row2.Y, Row3.Y);
}

constexpr Vector4 Matrix4x4::GetCol2() const
{
    return Vector4(Row0.Z, Row1.Z, Row2.Z, Row3.Z);
}

constexpr Vector4 Matrix4x4::GetCol3() const
{
    return Vector4(Row0.W, Row1.W, Row2.W, Row3.W);
}

constexpr void Matrix4x4::SetCol0(const Vector4& c)
{
    Row0.X = c.X;
    Row1.X = c.Y;
    Row2.X = c.Z;
    Row3.X = c.W;
}

constexpr void Matrix4x4::SetCol1(const Vector4& c)
{
    Row0.Y = c.X;
    Row1.Y = c.Y;
    Row2.Y = c.Z;
    Row3.Y = c.W;
}

constexpr void Matrix4x4::SetCol2(const Vector4& c)
{
    Row0.Z = c.X;
    Row1.Z = c.Y;
    Row2.Z = c.Z;
    Row3.Z = c.W;
}

constexpr void Matrix4x4::SetCol3(const Vector4& c)
{
    Row0.W = c.X;
    Row1.W = c.Y;
    Row2.W = c.Z;
    Row3.W = c.W;
}

constexpr Matrix4x4 Matrix4x4::operator+(const Matrix4x4& rhs) const
{
    Matrix4x4 res;
    res.Row0 = Row0 + rhs.Row0;
    res.Row1 = Row1 + rhs.Row1;
    res.Row2 = Row2 + rhs.Row2;
    res.Row3 = Row3 + rhs.Row3;
    return res;
}

constexpr Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& rhs)
{
    Row0 += rhs.Row0;
    Row1 += rhs.Row1;
    Row2 += rhs.Row2;
    Row3 += rhs.Row3;
    return *this;
}

constexpr Matrix4x4 Matrix4x4::operator-(const Matrix4x4& rhs) const
{
    Matrix4x4 res;
    res.Row0 = Row0 - rhs.Row0;
    res.Row1 = Row1 - rhs.Row1;
    res.Row2 = Row2 - rhs.Row2;
    res.Row3 = Row3 - rhs.Row3;
    return res;
}

constexpr Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& rhs)
{
    Row0 -= rhs.Row0;
    Row1 -= rhs.Row1;
    Row2 -= rhs.Row2;
    Row3 -= rhs.Row3;
    return *this;
}

constexpr Matrix4x4 Matrix4x4::operator*(const Matrix4x4& rhs) const
{
    Matrix4x4 res;

    res.Row0.X = Row0.X * rhs.Row0.X + Row0.Y * rhs.Row1.X + Row0.Z * rhs.Row2.X + Row0.W * rhs.Row3.X;
    res.Row0.Y = Row0.X * rhs.Row0.Y + Row0.Y * rhs.Row1.Y + Row0.Z * rhs.Row2.Y + Row0.W * rhs.Row3.Y;
    res.Row0.Z = Row0.X * rhs.Row0.Z + Row0.Y * rhs.Row1.Z + Row0.Z * rhs.Row2.Z + Row0.W * rhs.Row3.Z;
    res.Row0.W = Row0.X * rhs.Row0.W + Row0.Y * rhs.Row1.W + Row0.Z * rhs.Row2.W + Row0.W * rhs.Row3.W;

    res.Row1.X = Row1.X * rhs.Row0.X + Row1.Y * rhs.Row1.X + Row1.Z * rhs.Row2.X + Row1.W * rhs.Row3.X;
    res.Row1.Y = Row1.X * rhs.Row0.Y + Row1.Y * rhs.Row1.Y + Row1.Z * rhs.Row2.Y + Row1.W * rhs.Row3.Y;
    res.Row1.Z = Row1.X * rhs.Row0.Z + Row1.Y * rhs.Row1.Z + Row1.Z * rhs.Row2.Z + Row1.W * rhs.Row3.Z;
    res.Row1.W = Row1.X * rhs.Row0.W + Row1.Y * rhs.Row1.W + Row1.Z * rhs.Row2.W + Row1.W * rhs.Row3.W;

    res.Row2.X = Row2.X * rhs.Row0.X + Row2.Y * rhs.Row1.X + Row2.Z * rhs.Row2.X + Row2.W * rhs.Row3.X;
    res.Row2.Y = Row2.X * rhs.Row0.Y + Row2.Y * rhs.Row1.Y + Row2.Z * rhs.Row2.Y + Row2.W * rhs.Row3.Y;
    res.Row2.Z = Row2.X * rhs.Row0.Z + Row2.Y * rhs.Row1.Z + Row2.Z * rhs.Row2.Z + Row2.W * rhs.Row3.Z;
    res.Row2.W = Row2.X * rhs.Row0.W + Row2.Y * rhs.Row1.W + Row2.Z * rhs.Row2.W + Row2.W * rhs.Row3.W;

    res.Row3.X = Row3.X * rhs.Row0.X + Row3.Y * rhs.Row1.X + Row3.Z * rhs.Row2.X + Row3.W * rhs.Row3.X;
    res.Row3.Y = Row3.X * rhs.Row0.Y + Row3.Y * rhs.Row1.Y + Row3.Z * rhs.Row2.Y + Row3.W * rhs.Row3.Y;
    res.Row3.Z = Row3.X * rhs.Row0.Z + Row3.Y * rhs.Row1.Z + Row3.Z * rhs.Row2.Z + Row3.W * rhs.Row3.Z;
    res.Row3.W = Row3.X * rhs.Row0.W + Row3.Y * rhs.Row1.W + Row3.Z * rhs.Row2.W + Row3.W * rhs.Row3.W;

    return res;
}

constexpr Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& rhs)
{
    *this = *this * rhs;
    return *this;
}

constexpr Matrix4x4 Matrix4x4::operator*(float scalar) const
{
    Matrix4x4 res;
    res.Row0 = Row0 * scalar;
    res.Row1 = Row1 * scalar;
    res.Row2 = Row2 * scalar;
    res.Row3 = Row3 * scalar;
    return res;
}

constexpr Matrix4x4& Matrix4x4::operator*=(float scalar)
{
    Row0 *= scalar;
    Row1 *= scalar;
    Row2 *= scalar;
    Row3 *= scalar;
    return *this;
}

constexpr Matrix4x4 Matrix4x4::operator/(float scalar) const
{
    Matrix4x4 res;
    res.Row0 = Row0 / scalar;
    res.Row1 = Row1 / scalar;
    res.Row2 = Row2 / scalar;
    res.Row3 = Row3 / scalar;
    return res;
}

constexpr Matrix4x4& Matrix4x4::operator/=(float scalar)
{
    Row0 /= scalar;
    Row1 /= scalar;
    Row2 /= scalar;
    Row3 /= scalar;
    return *this;
}

constexpr bool Matrix4x4::operator==(const Matrix4x4& r) const
{
    return Row0 == r.Row0 && Row1 == r.Row1 && Row2 == r.Row2 && Row3 == r.Row3;
}

constexpr bool Matrix4x4::operator!=(const Matrix4x4& r) const
{
    return !(*this == r);
}

constexpr bool Matrix4x4::IsEqualApprox(const Matrix4x4& r) const
{
    return Row0.IsEqualApprox(r.Row0) && Row1.IsEqualApprox(r.Row1) && Row2.IsEqualApprox(r.Row2) &&
           Row3.IsEqualApprox(r.Row3);
}

constexpr bool Matrix4x4::IsNotEqualApprox(const Matrix4x4& r) const
{
    return !IsEqualApprox(r);
}

constexpr bool Matrix4x4::IsOrthogonal() const
{
    const Vector4 c0 = GetCol0();
    const Vector4 c1 = GetCol1();
    const Vector4 c2 = GetCol2();
    const Vector4 c3 = GetCol3();
    return math::IsEqualApprox(c0.Dot(c1), 0.0f, math::EPSILON_ORTHO) &&
           math::IsEqualApprox(c0.Dot(c2), 0.0f, math::EPSILON_ORTHO) &&
           math::IsEqualApprox(c0.Dot(c3), 0.0f, math::EPSILON_ORTHO) &&
           math::IsEqualApprox(c1.Dot(c2), 0.0f, math::EPSILON_ORTHO) &&
           math::IsEqualApprox(c1.Dot(c3), 0.0f, math::EPSILON_ORTHO) &&
           math::IsEqualApprox(c2.Dot(c3), 0.0f, math::EPSILON_ORTHO);
}

void Matrix4x4::Orthogonalize()
{
    const float mag0 = GetCol0().Magnitude();
    const float mag1 = GetCol1().Magnitude();
    const float mag2 = GetCol2().Magnitude();
    const float mag3 = GetCol3().Magnitude();
    Orthonormalize();
    SetCol0(GetCol0() * mag0);
    SetCol1(GetCol1() * mag1);
    SetCol2(GetCol2() * mag2);
    SetCol3(GetCol3() * mag3);
}

Matrix4x4 Matrix4x4::Orthogonalized() const
{
    Matrix4x4 copy = *this;
    copy.Orthogonalize();
    return copy;
}

constexpr bool Matrix4x4::IsOrthonormal() const
{
    return IsOrthogonal() && GetCol0().IsUnitApprox() && GetCol1().IsUnitApprox() && GetCol2().IsUnitApprox() &&
           GetCol3().IsUnitApprox();
}

void Matrix4x4::Orthonormalize()
{
    Vector4 x = GetCol0();
    Vector4 y = GetCol1();
    Vector4 z = GetCol2();
    Vector4 w = GetCol3();

    x.Normalize();
    y -= x * (x.Dot(y));
    y.Normalize();
    z -= x * (x.Dot(z)) + y * (y.Dot(z));
    z.Normalize();
    w -= x * (x.Dot(w)) + y * (y.Dot(w)) + z * (z.Dot(w));
    w.Normalize();

    SetCol0(x);
    SetCol1(y);
    SetCol2(z);
    SetCol3(w);
}

Matrix4x4 Matrix4x4::Orthonormalized() const
{
    Matrix4x4 copy = *this;
    copy.Orthonormalize();
    return copy;
}

constexpr void Matrix4x4::Transpose()
{
    *this = Matrix4x4(Row0, Row1, Row2, Row3);
}

constexpr Matrix4x4 Matrix4x4::Transposed() const
{
    Matrix4x4 copy = *this;
    copy.Transpose();
    return copy;
}

constexpr void Matrix4x4::Invert()
{
    const float* m = &Data[0][0];
    float inv[16] = {0.0f};

    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] +
             m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] -
             m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] +
             m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] -
              m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] -
             m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] +
             m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] -
             m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] +
              m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] +
             m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] -
             m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] +
              m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] -
              m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] -
             m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
              m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    const float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (math::IsZeroApprox(det))
    {
        const Vector4 nanv(math::FLOAT_NAN, math::FLOAT_NAN, math::FLOAT_NAN, math::FLOAT_NAN);
        Row0 = Row1 = Row2 = Row3 = nanv;
        return;
    }

    const float invDet = 1.0f / det;
    float* dst = &Data[0][0];

    for (int32_t i = 0; i < 16; ++i)
    {
        dst[i] = inv[i] * invDet;
    }
}

constexpr Matrix4x4 Matrix4x4::Inverse() const
{
    Matrix4x4 copy = *this;
    copy.Invert();
    return copy;
}

constexpr float Matrix4x4::Trace() const
{
    return Row0.X + Row1.Y + Row2.Z + Row3.W;
}

constexpr float Matrix4x4::Determinant() const
{
    const float subfactor00 = Row2.Z * Row3.W - Row3.Z * Row2.W;
    const float subfactor01 = Row2.Y * Row3.W - Row3.Y * Row2.W;
    const float subfactor02 = Row2.Y * Row3.Z - Row3.Y * Row2.Z;
    const float subfactor03 = Row2.X * Row3.W - Row3.X * Row2.W;
    const float subfactor04 = Row2.X * Row3.Z - Row3.X * Row2.Z;
    const float subfactor05 = Row2.X * Row3.Y - Row3.X * Row2.Y;

    return Row0.X * (Row1.Y * subfactor00 - Row1.Z * subfactor01 + Row1.W * subfactor02) -
           Row0.Y * (Row1.X * subfactor00 - Row1.Z * subfactor03 + Row1.W * subfactor04) +
           Row0.Z * (Row1.X * subfactor01 - Row1.Y * subfactor03 + Row1.W * subfactor05) -
           Row0.W * (Row1.X * subfactor02 - Row1.Y * subfactor04 + Row1.Z * subfactor05);
}

INLINE constexpr Matrix4x4 Matrix4x4::sIdentity = Matrix4x4(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                                                            Vector4(0.0f, 1.0f, 0.0f, 0.0f),
                                                            Vector4(0.0f, 0.0f, 1.0f, 0.0f),
                                                            Vector4(0.0f, 0.0f, 0.0f, 1.0f));

[[nodiscard]] FORCE_INLINE constexpr Matrix4x4 operator*(float s, const Matrix4x4& m)
{
    return m * s;
}

[[nodiscard]] FORCE_INLINE constexpr Vector4 operator*(const Matrix4x4& m, const Vector4& v)
{
    return Vector4(m.Row0.Dot(v), m.Row1.Dot(v), m.Row2.Dot(v), m.Row3.Dot(v));
}
} // namespace ho
