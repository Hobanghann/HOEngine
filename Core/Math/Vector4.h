#pragma once

#include <string>

#include "Macros.h"
#include "MathDefs.h"
#include "MathFuncs.h"
#include "Vector3.h"

namespace ho
{
struct Vector2;
struct Vector3;
struct Quaternion;
struct Color128;

struct alignas(16) Vector4 final
{
    constexpr Vector4();
    constexpr Vector4(float x, float y, float z, float w);
    constexpr Vector4(const Vector2& v);
    constexpr Vector4(const Vector3& v);
    explicit Vector4(const Quaternion& q);
    explicit Vector4(const Color128& c);
    constexpr Vector4(const Vector4&) = default;
    constexpr Vector4& operator=(const Vector4& rhs);
    ~Vector4() = default;

    [[nodiscard]] constexpr Vector4 operator+(const Vector4& rhs) const;
    constexpr Vector4& operator+=(const Vector4& rhs);

    [[nodiscard]] constexpr Vector4 operator-(const Vector4& rhs) const;
    constexpr Vector4& operator-=(const Vector4& rhs);

    [[nodiscard]] constexpr Vector4 operator*(const Vector4& rhs) const;
    constexpr Vector4& operator*=(const Vector4& rhs);

    [[nodiscard]] constexpr Vector4 operator/(const Vector4& rhs) const;
    constexpr Vector4& operator/=(const Vector4& rhs);

    [[nodiscard]] constexpr Vector4 operator*(float scalar) const;
    constexpr Vector4& operator*=(float scalar);

    [[nodiscard]] constexpr Vector4 operator/(float scalar) const;
    constexpr Vector4& operator/=(float scalar);

    [[nodiscard]] constexpr Vector4 operator-() const;

    [[nodiscard]] constexpr bool operator==(const Vector4& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Vector4& rhs) const;

    [[nodiscard]] constexpr bool operator<(const Vector4& rhs) const;
    [[nodiscard]] constexpr bool operator<=(const Vector4& rhs) const;
    [[nodiscard]] constexpr bool operator>(const Vector4& rhs) const;
    [[nodiscard]] constexpr bool operator>=(const Vector4& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Vector4& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Vector4& rhs) const;

    [[nodiscard]] constexpr float Dot(const Vector4& rhs) const;

    [[nodiscard]] FORCE_INLINE float Magnitude() const;
    [[nodiscard]] constexpr float SqrdMagnitude() const;

    FORCE_INLINE void Normalize();
    [[nodiscard]] FORCE_INLINE Vector4 Normalized() const;

    constexpr void Project(const Vector4& ontoVector);
    constexpr Vector4 Projected(const Vector4& ontoVector) const;

    constexpr bool IsUnit() const;
    constexpr bool IsUnitApprox() const;

    [[nodiscard]] FORCE_INLINE bool IsFinite() const;

    constexpr Vector3 ToCartesian() const;

    std::string ToString() const;

    static const Vector4 sUnitX;
    static const Vector4 sUnitY;
    static const Vector4 sUnitZ;
    static const Vector4 sUnitW;
    static const Vector4 sZero;

    union
    {
        struct
        {
            float X;
            float Y;
            float Z;
            float W;
        };

        float Data[4] = {};
    };
};

constexpr Vector4::Vector4()
  : X(0.0f)
  , Y(0.0f)
  , Z(0.0f)
  , W(0.0f)
{
}

constexpr Vector4::Vector4(float x, float y, float z, float w)
  : X(x)
  , Y(y)
  , Z(z)
  , W(w)
{
}

constexpr Vector4::Vector4(const Vector2& v)
  : X(v.X)
  , Y(v.Y)
  , Z(0.0f)
  , W(0.0f)
{
}

constexpr Vector4::Vector4(const Vector3& v)
  : X(v.X)
  , Y(v.Y)
  , Z(v.Z)
  , W(0.0f)
{
}

INLINE constexpr Vector4 Vector4::sUnitX = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
INLINE constexpr Vector4 Vector4::sUnitY = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
INLINE constexpr Vector4 Vector4::sUnitZ = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
INLINE constexpr Vector4 Vector4::sUnitW = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
INLINE constexpr Vector4 Vector4::sZero = Vector4(0.0f, 0.0f, 0.0f, 0.0f);

constexpr Vector4& Vector4::operator=(const Vector4& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    X = rhs.X;
    Y = rhs.Y;
    Z = rhs.Z;
    W = rhs.W;
    return *this;
}

constexpr Vector4 Vector4::operator+(const Vector4& rhs) const
{
    return Vector4(X + rhs.X, Y + rhs.Y, Z + rhs.Z, W + rhs.W);
}

constexpr Vector4& Vector4::operator+=(const Vector4& rhs)
{
    X += rhs.X;
    Y += rhs.Y;
    Z += rhs.Z;
    W += rhs.W;
    return *this;
}

constexpr Vector4 Vector4::operator-(const Vector4& rhs) const
{
    return Vector4(X - rhs.X, Y - rhs.Y, Z - rhs.Z, W - rhs.W);
}

constexpr Vector4& Vector4::operator-=(const Vector4& rhs)
{
    X -= rhs.X;
    Y -= rhs.Y;
    Z -= rhs.Z;
    W -= rhs.W;
    return *this;
}

constexpr Vector4 Vector4::operator*(const Vector4& rhs) const
{
    return Vector4(X * rhs.X, Y * rhs.Y, Z * rhs.Z, W * rhs.W);
}

constexpr Vector4& Vector4::operator*=(const Vector4& rhs)
{
    X *= rhs.X;
    Y *= rhs.Y;
    Z *= rhs.Z;
    W *= rhs.W;
    return *this;
}

constexpr Vector4 Vector4::operator/(const Vector4& rhs) const
{
    return Vector4(X / rhs.X, Y / rhs.Y, Z / rhs.Z, W / rhs.W);
}

constexpr Vector4& Vector4::operator/=(const Vector4& rhs)
{
    X /= rhs.X;
    Y /= rhs.Y;
    Z /= rhs.Z;
    W /= rhs.W;
    return *this;
}

constexpr Vector4 Vector4::operator*(float scalar) const
{
    return Vector4(X * scalar, Y * scalar, Z * scalar, W * scalar);
}

constexpr Vector4& Vector4::operator*=(float scalar)
{
    X *= scalar;
    Y *= scalar;
    Z *= scalar;
    W *= scalar;
    return *this;
}

constexpr Vector4 Vector4::operator/(float scalar) const
{
    const float inv = 1.0f / scalar;
    return *this * inv;
}

constexpr Vector4& Vector4::operator/=(float scalar)
{
    const float inv = 1.0f / scalar;
    *this *= inv;
    return *this;
}

constexpr Vector4 Vector4::operator-() const
{
    return Vector4(-X, -Y, -Z, -W);
}

[[nodiscard]] FORCE_INLINE constexpr Vector4 operator*(float scalar, const Vector4& vector)
{
    return vector * scalar;
}

[[nodiscard]] constexpr Vector4 operator/(float scalar, const Vector4& vector)
{
    return Vector4(scalar / vector.X, scalar / vector.Y, scalar / vector.Z, scalar / vector.W);
}

constexpr bool Vector4::operator==(const Vector4& rhs) const
{
    return X == rhs.X && Y == rhs.Y && Z == rhs.Z && W == rhs.W;
}

constexpr bool Vector4::operator!=(const Vector4& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Vector4::operator<(const Vector4& rhs) const
{
    if (X == rhs.X)
    {
        if (Y == rhs.Y)
        {
            if (Z == rhs.Z)
            {
                return W < rhs.W;
            }
            return Z < rhs.Z;
        }
        return Y < rhs.Y;
    }
    return X < rhs.X;
}

constexpr bool Vector4::operator<=(const Vector4& rhs) const
{
    if (X == rhs.X)
    {
        if (Y == rhs.Y)
        {
            if (Z == rhs.Z)
            {
                return W <= rhs.W;
            }
            return Z < rhs.Z;
        }
        return Y < rhs.Y;
    }
    return X < rhs.X;
}

constexpr bool Vector4::operator>(const Vector4& rhs) const
{
    return !(*this <= rhs);
}

constexpr bool Vector4::operator>=(const Vector4& rhs) const
{
    return !(*this < rhs);
}

constexpr bool Vector4::IsEqualApprox(const Vector4& rhs) const
{
    return math::IsEqualApprox(X, rhs.X) && math::IsEqualApprox(Y, rhs.Y) && math::IsEqualApprox(Z, rhs.Z) &&
           math::IsEqualApprox(W, rhs.W);
}

constexpr bool Vector4::IsNotEqualApprox(const Vector4& rhs) const
{
    return !IsEqualApprox(rhs);
}

constexpr float ho::Vector4::Dot(const Vector4& rhs) const
{
    return X * rhs.X + Y * rhs.Y + Z * rhs.Z + W * rhs.W;
}

float Vector4::Magnitude() const
{
    return math::Sqrt(SqrdMagnitude());
}

constexpr float Vector4::SqrdMagnitude() const
{
    return this->Dot(*this);
}

void Vector4::Normalize()
{
    const float invMag = math::InvSqrtFast(SqrdMagnitude());
    *this = invMag * *this;
}

Vector4 Vector4::Normalized() const
{
    const float invMag = math::InvSqrtFast(SqrdMagnitude());
    return invMag * *this;
}

constexpr bool Vector4::IsUnit() const
{
    return SqrdMagnitude() == 1.0f;
}

constexpr bool Vector4::IsUnitApprox() const
{
    return math::IsEqualApprox(SqrdMagnitude(), 1.0f, math::EPSILON_UNIT);
}

constexpr void Vector4::Project(const Vector4& ontoVector)
{
    const float sqMag = ontoVector.SqrdMagnitude();

    if (math::IsZeroApprox(sqMag))
    {
        return;
    }

    const float dot = this->Dot(ontoVector);
    *this = ontoVector * (dot / sqMag);
}

constexpr Vector4 Vector4::Projected(const Vector4& ontoVector) const
{
    Vector4 copy = *this;
    copy.Project(ontoVector);
    return copy;
}

bool Vector4::IsFinite() const
{
    return math::IsFinite(X) && math::IsFinite(Y) && math::IsFinite(Z) && math::IsFinite(W);
}

constexpr Vector3 Vector4::ToCartesian() const
{
    const float invW = 1.0f / W;
    return Vector3(invW * X, invW * Y, invW * Z);
}

} // namespace ho
