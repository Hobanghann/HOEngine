#pragma once

#include <string>

#include "Core/Macros.h"
#include "MathDefs.h"
#include "MathFuncs.h"
#include "Vector2.h"

namespace ho
{
struct Vector4;

struct alignas(16) Vector3 final
{
    constexpr Vector3();
    constexpr Vector3(float x, float y, float z);
    constexpr Vector3(const Vector3&) = default;
    constexpr explicit Vector3(const Vector2& v);
    explicit Vector3(const Vector4& v);
    constexpr Vector3& operator=(const Vector3& rhs);
    ~Vector3() = default;

    [[nodiscard]] constexpr Vector3 operator+(const Vector3& rhs) const;
    constexpr Vector3& operator+=(const Vector3& rhs);

    [[nodiscard]] constexpr Vector3 operator-(const Vector3& rhs) const;
    constexpr Vector3& operator-=(const Vector3& rhs);

    [[nodiscard]] constexpr Vector3 operator*(const Vector3& rhs) const;
    constexpr Vector3& operator*=(const Vector3& rhs);

    [[nodiscard]] constexpr Vector3 operator/(const Vector3& rhs) const;
    constexpr Vector3& operator/=(const Vector3& rhs);

    [[nodiscard]] constexpr Vector3 operator*(float scalar) const;
    constexpr Vector3& operator*=(float scalar);

    [[nodiscard]] constexpr Vector3 operator/(float scalar) const;
    constexpr Vector3& operator/=(float scalar);

    [[nodiscard]] constexpr Vector3 operator-() const;

    [[nodiscard]] constexpr bool operator==(const Vector3& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Vector3& rhs) const;

    [[nodiscard]] constexpr bool operator<(const Vector3& rhs) const;
    [[nodiscard]] constexpr bool operator<=(const Vector3& rhs) const;
    [[nodiscard]] constexpr bool operator>(const Vector3& rhs) const;
    [[nodiscard]] constexpr bool operator>=(const Vector3& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Vector3& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Vector3& rhs) const;

    [[nodiscard]] constexpr float Dot(const Vector3& rhs) const;

    [[nodiscard]] constexpr Vector3 Cross(const Vector3& rhs) const;

    [[nodiscard]] FORCE_INLINE float Magnitude() const;
    [[nodiscard]] constexpr float SqrdMagnitude() const;

    FORCE_INLINE void Normalize();
    [[nodiscard]] FORCE_INLINE Vector3 Normalized() const;

    constexpr void Project(const Vector3& ontoVector);
    [[nodiscard]] constexpr Vector3 Projected(const Vector3& ontoVector) const;

    [[nodiscard]] constexpr bool IsUnit() const;
    [[nodiscard]] constexpr bool IsUnitApprox() const;

    [[nodiscard]] FORCE_INLINE bool IsFinite() const;

    [[nodiscard]] constexpr Vector2 ToCartesian() const;

    [[nodiscard]] Vector4 ToHomogeneous() const;

    [[nodiscard]] std::string ToString() const;

    static const Vector3 sUnitX;
    static const Vector3 sUnitY;
    static const Vector3 sUnitZ;
    static const Vector3 sZero;

    union
    {
        struct
        {
            float X;
            float Y;
            float Z;
        };

        float Data[3] = {};
    };

    float Pad = 0.0f;
};

constexpr Vector3::Vector3()
  : X(0.0f)
  , Y(0.0f)
  , Z(0.0f)
{
}

constexpr Vector3::Vector3(float x, float y, float z)
  : X(x)
  , Y(y)
  , Z(z)
{
}

constexpr Vector3::Vector3(const Vector2& v)
  : X(v.X)
  , Y(v.Y)
  , Z(0.0f)
{
}

constexpr Vector3& Vector3::operator=(const Vector3& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    X = rhs.X;
    Y = rhs.Y;
    Z = rhs.Z;
    return *this;
}

INLINE constexpr const Vector3 Vector3::sUnitX = Vector3(1.0f, 0.0f, 0.0f);
INLINE constexpr const Vector3 Vector3::sUnitY = Vector3(0.0f, 1.0f, 0.0f);
INLINE constexpr const Vector3 Vector3::sUnitZ = Vector3(0.0f, 0.0f, 1.0f);
INLINE constexpr const Vector3 Vector3::sZero = Vector3(0.0f, 0.0f, 0.0f);

constexpr Vector3 Vector3::operator+(const Vector3& rhs) const
{
    return Vector3(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
}

constexpr Vector3& Vector3::operator+=(const Vector3& rhs)
{
    X += rhs.X;
    Y += rhs.Y;
    Z += rhs.Z;
    return *this;
}

constexpr Vector3 Vector3::operator-(const Vector3& rhs) const
{
    return Vector3(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
}

constexpr Vector3& Vector3::operator-=(const Vector3& rhs)
{
    X -= rhs.X;
    Y -= rhs.Y;
    Z -= rhs.Z;
    return *this;
}

constexpr Vector3 Vector3::operator*(const Vector3& rhs) const
{
    return Vector3(X * rhs.X, Y * rhs.Y, Z * rhs.Z);
}

constexpr Vector3& Vector3::operator*=(const Vector3& rhs)
{
    X *= rhs.X;
    Y *= rhs.Y;
    Z *= rhs.Z;
    return *this;
}

constexpr Vector3 Vector3::operator/(const Vector3& rhs) const
{
    return Vector3(X / rhs.X, Y / rhs.Y, Z / rhs.Z);
}

constexpr Vector3& Vector3::operator/=(const Vector3& rhs)
{
    X /= rhs.X;
    Y /= rhs.Y;
    Z /= rhs.Z;
    return *this;
}

constexpr Vector3 Vector3::operator*(float scalar) const
{
    return Vector3(scalar * X, scalar * Y, scalar * Z);
}

constexpr Vector3& Vector3::operator*=(float scalar)
{
    X *= scalar;
    Y *= scalar;
    Z *= scalar;
    return *this;
}

constexpr Vector3 Vector3::operator/(float scalar) const
{
    const float inv = 1.0f / scalar;
    return *this * inv;
}

constexpr Vector3& Vector3::operator/=(float scalar)
{
    const float inv = 1.0f / scalar;
    *this *= inv;
    return *this;
}

constexpr Vector3 Vector3::operator-() const
{
    return Vector3(-X, -Y, -Z);
}

[[nodiscard]] constexpr Vector3 operator*(float scalar, const Vector3& v)
{
    return v * scalar;
}

[[nodiscard]] constexpr Vector3 operator/(float scalar, const Vector3& vector)
{
    return Vector3(scalar / vector.X, scalar / vector.Y, scalar / vector.Z);
}

constexpr bool Vector3::operator==(const Vector3& rhs) const
{
    return X == rhs.X && Y == rhs.Y && Z == rhs.Z;
}

constexpr bool Vector3::operator!=(const Vector3& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Vector3::operator<(const Vector3& rhs) const
{
    if (X == rhs.X)
    {
        if (Y == rhs.Y)
        {
            return Z < rhs.Z;
        }
        return Y < rhs.Y;
    }
    return X < rhs.X;
}

constexpr bool Vector3::operator<=(const Vector3& rhs) const
{
    if (X == rhs.X)
    {
        if (Y == rhs.Y)
        {
            return Z <= rhs.Z;
        }
        return Y < rhs.Y;
    }
    return X < rhs.X;
}

constexpr bool Vector3::operator>(const Vector3& rhs) const
{
    return !(*this <= rhs);
}

constexpr bool Vector3::operator>=(const Vector3& rhs) const
{
    return !(*this < rhs);
}

constexpr bool Vector3::IsEqualApprox(const Vector3& rhs) const
{
    return math::IsEqualApprox(X, rhs.X) && math::IsEqualApprox(Y, rhs.Y) && math::IsEqualApprox(Z, rhs.Z);
}

constexpr bool Vector3::IsNotEqualApprox(const Vector3& rhs) const
{
    return !IsEqualApprox(rhs);
}

constexpr float Vector3::Dot(const Vector3& rhs) const
{
    return X * rhs.X + Y * rhs.Y + Z * rhs.Z;
}

constexpr Vector3 Vector3::Cross(const Vector3& rhs) const
{
    return Vector3((Y * rhs.Z) - (Z * rhs.Y), (Z * rhs.X) - (X * rhs.Z), (X * rhs.Y) - (Y * rhs.X));
}

float Vector3::Magnitude() const
{
    return math::Sqrt(SqrdMagnitude());
}

constexpr float Vector3::SqrdMagnitude() const
{
    return this->Dot(*this);
}

void Vector3::Normalize()
{
    const float invMag = math::InvSqrtFast(SqrdMagnitude());
    *this = invMag * *this;
}

Vector3 Vector3::Normalized() const
{
    const float invMag = math::InvSqrtFast(SqrdMagnitude());
    return invMag * *this;
}

constexpr void Vector3::Project(const Vector3& ontoVector)
{
    const float sqMag = ontoVector.SqrdMagnitude();

    if (math::IsZeroApprox(sqMag))
    {
        return;
    }

    const float dot = this->Dot(ontoVector);
    *this = ontoVector * (dot / sqMag);
}

constexpr Vector3 Vector3::Projected(const Vector3& ontoVector) const
{
    Vector3 copy = *this;
    copy.Project(ontoVector);
    return copy;
}

constexpr bool Vector3::IsUnit() const
{
    return SqrdMagnitude() == 1.0f;
}

constexpr bool Vector3::IsUnitApprox() const
{
    return math::IsEqualApprox(SqrdMagnitude(), 1.0f, math::EPSILON_UNIT);
}

bool Vector3::IsFinite() const
{
    return math::IsFinite(X) && math::IsFinite(Y) && math::IsFinite(Z);
}

constexpr Vector2 Vector3::ToCartesian() const
{
    const float invZ = 1.0f / Z;
    return Vector2(X * invZ, Y * invZ);
}
} // namespace ho
