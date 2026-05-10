#pragma once

#include <string>

#include "Macros.h"
#include "MathDefs.h"
#include "MathFuncs.h"

namespace ho
{

struct Vector3;
struct Vector4;

struct alignas(8) Vector2 final
{
    constexpr Vector2();
    constexpr Vector2(float x, float y);
    explicit Vector2(const Vector3& v);
    explicit Vector2(const Vector4& v);
    constexpr Vector2(const Vector2&) = default;
    constexpr Vector2& operator=(const Vector2& rhs);
    ~Vector2() = default;

    [[nodiscard]] constexpr Vector2 operator+(const Vector2& rhs) const;
    constexpr Vector2& operator+=(const Vector2& rhs);

    [[nodiscard]] constexpr Vector2 operator-(const Vector2& rhs) const;
    constexpr Vector2& operator-=(const Vector2& rhs);

    [[nodiscard]] constexpr Vector2 operator*(const Vector2& rhs) const;
    constexpr Vector2& operator*=(const Vector2& rhs);

    [[nodiscard]] constexpr Vector2 operator/(const Vector2& rhs) const;
    constexpr Vector2& operator/=(const Vector2& rhs);

    [[nodiscard]] constexpr Vector2 operator*(float scalar) const;
    constexpr Vector2& operator*=(float scalar);

    [[nodiscard]] constexpr Vector2 operator/(float scalar) const;
    constexpr Vector2& operator/=(float scalar);

    [[nodiscard]] constexpr Vector2 operator-() const;

    [[nodiscard]] constexpr bool operator==(const Vector2& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Vector2& rhs) const;

    [[nodiscard]] constexpr bool operator<(const Vector2& rhs) const;
    [[nodiscard]] constexpr bool operator<=(const Vector2& rhs) const;
    [[nodiscard]] constexpr bool operator>(const Vector2& rhs) const;
    [[nodiscard]] constexpr bool operator>=(const Vector2& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Vector2& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Vector2& rhs) const;

    [[nodiscard]] constexpr float Dot(const Vector2& rhs) const;

    [[nodiscard]] constexpr float Cross(const Vector2& rhs) const;

    [[nodiscard]] FORCE_INLINE float Magnitude() const;
    [[nodiscard]] constexpr float SqrdMagnitude() const;

    FORCE_INLINE void Normalize();
    [[nodiscard]] FORCE_INLINE Vector2 Normalized() const;

    constexpr void Project(const Vector2& ontoVector);
    [[nodiscard]] constexpr Vector2 Projected(const Vector2& ontoVector) const;

    [[nodiscard]] constexpr bool IsUnit() const;
    [[nodiscard]] constexpr bool IsUnitApprox() const;

    [[nodiscard]] FORCE_INLINE bool IsFinite() const;

    [[nodiscard]] Vector3 ToHomogeneous() const;

    [[nodiscard]] std::string ToString() const;

    static const Vector2 sUnitX;
    static const Vector2 sUnitY;
    static const Vector2 sZero;

    union
    {
        struct
        {
            float X;
            float Y;
        };

        float Data[2] = {};
    };
};

constexpr Vector2::Vector2()
  : X(0.0f)
  , Y(0.0f)
{
}

constexpr Vector2::Vector2(float x, float y)
  : X(x)
  , Y(y)
{
}

INLINE constexpr Vector2 Vector2::sUnitX = Vector2(1.0f, 0.0f);
INLINE constexpr Vector2 Vector2::sUnitY = Vector2(0.0f, 1.0f);
INLINE constexpr Vector2 Vector2::sZero = Vector2(0.0f, 0.0f);

constexpr Vector2& Vector2::operator=(const Vector2& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    X = rhs.X;
    Y = rhs.Y;
    return *this;
}

constexpr Vector2 Vector2::operator+(const Vector2& rhs) const
{
    return Vector2(X + rhs.X, Y + rhs.Y);
}

constexpr Vector2& Vector2::operator+=(const Vector2& rhs)
{
    X += rhs.X;
    Y += rhs.Y;
    return *this;
}

constexpr Vector2 Vector2::operator-(const Vector2& rhs) const
{
    return Vector2(X - rhs.X, Y - rhs.Y);
}

constexpr Vector2& Vector2::operator-=(const Vector2& rhs)
{
    X -= rhs.X;
    Y -= rhs.Y;
    return *this;
}

constexpr Vector2 Vector2::operator*(const Vector2& rhs) const
{
    return Vector2(X * rhs.X, Y * rhs.Y);
}

constexpr Vector2& Vector2::operator*=(const Vector2& rhs)
{
    X *= rhs.X;
    Y *= rhs.Y;
    return *this;
}

constexpr Vector2 Vector2::operator/(const Vector2& rhs) const
{
    return Vector2(X / rhs.X, Y / rhs.Y);
}

constexpr Vector2& Vector2::operator/=(const Vector2& rhs)
{
    X /= rhs.X;
    Y /= rhs.Y;
    return *this;
}

constexpr Vector2 Vector2::operator*(float scalar) const
{
    return Vector2(scalar * X, scalar * Y);
}

constexpr Vector2& Vector2::operator*=(float scalar)
{
    X *= scalar;
    Y *= scalar;
    return *this;
}

constexpr Vector2 Vector2::operator/(float scalar) const
{
    const float inv = 1.0f / scalar;
    return *this * inv;
}

constexpr Vector2& Vector2::operator/=(float scalar)
{
    const float inv = 1.0f / scalar;
    *this *= inv;
    return *this;
}

constexpr Vector2 Vector2::operator-() const
{
    return Vector2(-X, -Y);
}

constexpr bool Vector2::operator==(const Vector2& rhs) const
{
    return X == rhs.X && Y == rhs.Y;
}

constexpr bool Vector2::operator!=(const Vector2& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Vector2::operator<(const Vector2& rhs) const
{
    return X == rhs.X ? (Y < rhs.Y) : (X < rhs.X);
}

constexpr bool Vector2::operator<=(const Vector2& rhs) const
{
    return X == rhs.X ? (Y <= rhs.Y) : (X < rhs.X);
}

constexpr bool Vector2::operator>(const Vector2& rhs) const
{
    return !(*this <= rhs);
}

constexpr bool Vector2::operator>=(const Vector2& rhs) const
{
    return !(*this < rhs);
}

constexpr bool Vector2::IsEqualApprox(const Vector2& rhs) const
{
    return math::IsEqualApprox(X, rhs.X) && math::IsEqualApprox(Y, rhs.Y);
}

constexpr bool Vector2::IsNotEqualApprox(const Vector2& rhs) const
{
    return !IsEqualApprox(rhs);
}

[[nodiscard]] constexpr Vector2 operator*(float scalar, const Vector2& vector)
{
    return vector * scalar;
}

[[nodiscard]] constexpr Vector2 operator/(float scalar, const Vector2& vector)
{
    return Vector2(scalar / vector.X, scalar / vector.Y);
}

constexpr float Vector2::Dot(const Vector2& rhs) const
{
    return X * rhs.X + Y * rhs.Y;
}

constexpr float Vector2::Cross(const Vector2& rhs) const
{
    return X * rhs.Y - Y * rhs.X;
}

constexpr float Vector2::SqrdMagnitude() const
{
    return this->Dot(*this);
}

float Vector2::Magnitude() const
{
    return math::Sqrt(SqrdMagnitude());
}

void Vector2::Normalize()
{
    const float invMag = math::InvSqrtFast(SqrdMagnitude());
    *this = invMag * *this;
}

Vector2 Vector2::Normalized() const
{
    const float invMag = math::InvSqrtFast(SqrdMagnitude());
    return invMag * *this;
}

constexpr void Vector2::Project(const Vector2& ontoVector)
{
    const float sqMag = ontoVector.SqrdMagnitude();

    if (math::IsZeroApprox(sqMag))
    {
        return;
    }

    const float dot = this->Dot(ontoVector);
    *this = ontoVector * (dot / sqMag);
}

constexpr Vector2 Vector2::Projected(const Vector2& ontoVector) const
{
    Vector2 copy = *this;
    copy.Project(ontoVector);
    return copy;
}

constexpr bool Vector2::IsUnit() const
{
    return SqrdMagnitude() == 1.0f;
}

constexpr bool Vector2::IsUnitApprox() const
{
    return math::IsEqualApprox(SqrdMagnitude(), 1.0f, math::EPSILON_UNIT);
}

bool Vector2::IsFinite() const
{
    return math::IsFinite(X) && math::IsFinite(Y);
}
} // namespace ho
