#pragma once

#include <string>

#include "Vector3.h"

namespace ho
{
struct Matrix3x3;

struct alignas(16) Quaternion final
{
  public:
    static const Quaternion sIdentity;

    [[nodiscard]] static Quaternion FromAxisAngle(const Vector3& axis, float angle);
    [[nodiscard]] static Quaternion FromEuler(float x,
                                              float y,
                                              float z,
                                              math::eEulerOrder order = math::eEulerOrder::ZYX);
    [[nodiscard]] static Quaternion FromMatrix(const Matrix3x3& m);

    constexpr Quaternion();
    constexpr Quaternion(const Quaternion& q) = default;
    constexpr Quaternion(float x, float y, float z, float w);
    constexpr Quaternion(const Vector3& vectorPart, float scalarPart = 0.0f);
    constexpr Quaternion& operator=(const Quaternion& rhs);
    ~Quaternion() = default;

    [[nodiscard]] FORCE_INLINE Vector3 GetVectorPart() const;
    [[nodiscard]] FORCE_INLINE float GetScalarPart() const;
    FORCE_INLINE void SetVectorPart(const Vector3& vector);
    FORCE_INLINE void SetScalarPart(float scalar);

    [[nodiscard]] FORCE_INLINE Vector3 GetAxis() const;
    [[nodiscard]] FORCE_INLINE float GetAngle() const;
    FORCE_INLINE void SetAxis(const Vector3& axis);
    FORCE_INLINE void SetAngle(float angle);

    [[nodiscard]] constexpr Quaternion operator+(const Quaternion& rhs) const;
    constexpr Quaternion& operator+=(const Quaternion& rhs);

    [[nodiscard]] constexpr Quaternion operator-(const Quaternion& rhs) const;
    constexpr Quaternion& operator-=(const Quaternion& rhs);

    [[nodiscard]] constexpr Quaternion operator*(const Quaternion& rhs) const;
    constexpr Quaternion& operator*=(const Quaternion& rhs);

    [[nodiscard]] constexpr Quaternion operator*(float scalar) const;
    constexpr Quaternion& operator*=(float scalar);

    [[nodiscard]] constexpr Quaternion operator/(float scalar) const;
    constexpr Quaternion& operator/=(float scalar);

    [[nodiscard]] constexpr Quaternion operator-() const;

    [[nodiscard]] constexpr bool operator==(const Quaternion& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Quaternion& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Quaternion& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Quaternion& rhs) const;

    [[nodiscard]] constexpr float Dot(const Quaternion& rhs) const;

    [[nodiscard]] FORCE_INLINE float Magnitude() const;
    [[nodiscard]] constexpr float SqrdMagnitude() const;

    FORCE_INLINE void Normalize();
    [[nodiscard]] FORCE_INLINE Quaternion Normalized() const;

    [[nodiscard]] constexpr bool IsUnit() const;
    [[nodiscard]] constexpr bool IsUnitApprox() const;

    constexpr void Conjugate();
    [[nodiscard]] constexpr Quaternion Conjugated() const;

    constexpr void Invert();
    [[nodiscard]] constexpr Quaternion Inverse() const;

    [[nodiscard]] constexpr bool IsPurelyImaginary() const;

    [[nodiscard]] constexpr Vector3 Transform(const Vector3& v) const;
    [[nodiscard]] constexpr Vector3 InvTransform(const Vector3& v) const;

    [[nodiscard]] std::string ToString() const;

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

constexpr Quaternion::Quaternion()
  : X(0.0f)
  , Y(0.0f)
  , Z(0.0f)
  , W(1.0f)
{
}

constexpr Quaternion::Quaternion(float x, float y, float z, float w)
  : X(x)
  , Y(y)
  , Z(z)
  , W(w)
{
}

constexpr Quaternion::Quaternion(const Vector3& vectorPart, float scalarPart)
  : X(vectorPart.X)
  , Y(vectorPart.Y)
  , Z(vectorPart.Z)
  , W(scalarPart)
{
}

constexpr Quaternion& Quaternion::operator=(const Quaternion& rhs)
{
    X = rhs.X;
    Y = rhs.Y;
    Z = rhs.Z;
    W = rhs.W;
    return *this;
}

INLINE constexpr Quaternion Quaternion::sIdentity = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

Vector3 Quaternion::GetVectorPart() const
{
    return Vector3(X, Y, Z);
}

float Quaternion::GetScalarPart() const
{
    return W;
}

void Quaternion::SetVectorPart(const Vector3& vector)
{
    X = vector.X;
    Y = vector.Y;
    Z = vector.Z;
}

void Quaternion::SetScalarPart(float scalar)
{
    W = scalar;
}

Vector3 Quaternion::GetAxis() const
{
    return Vector3(X, Y, Z).Normalized();
}

float Quaternion::GetAngle() const
{
    return math::Acos(W) * 2.0f;
}

void Quaternion::SetAxis(const Vector3& axis)
{
    const float d = axis.Magnitude();
    HO_ASSERT(!math::IsZeroApprox(d), "Axis can't be zero vector.");

    float sin = Vector3(X, Y, Z).Magnitude();
    sin /= d;
    X = axis.X * sin;
    Y = axis.Y * sin;
    Z = axis.Z * sin;
}

void Quaternion::SetAngle(float angle)
{
    const Vector3 vectorPart = GetVectorPart();
    const float d = vectorPart.Magnitude();
    float sin = 0.0f;
    float cos = 0.0f;
    math::SinCos(&sin, &cos, angle * 0.5f);
    sin /= d;

    X = vectorPart.X * sin;
    Y = vectorPart.Y * sin;
    Z = vectorPart.Z * sin;
    W = cos;
}

constexpr Quaternion Quaternion::operator+(const Quaternion& rhs) const
{
    return Quaternion(X + rhs.X, Y + rhs.Y, Z + rhs.Z, W + rhs.W);
}

constexpr Quaternion& Quaternion::operator+=(const Quaternion& rhs)
{
    X += rhs.X;
    Y += rhs.Y;
    Z += rhs.Z;
    W += rhs.W;
    return *this;
}

constexpr Quaternion Quaternion::operator-(const Quaternion& rhs) const
{
    return Quaternion(X - rhs.X, Y - rhs.Y, Z - rhs.Z, W - rhs.W);
}

constexpr Quaternion& Quaternion::operator-=(const Quaternion& rhs)
{
    X -= rhs.X;
    Y -= rhs.Y;
    Z -= rhs.Z;
    W -= rhs.W;
    return *this;
}

constexpr Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
    const float newX = W * rhs.X + X * rhs.W + Y * rhs.Z - Z * rhs.Y;
    const float newY = W * rhs.Y + Y * rhs.W + Z * rhs.X - X * rhs.Z;
    const float newZ = W * rhs.Z + Z * rhs.W + X * rhs.Y - Y * rhs.X;
    const float newW = W * rhs.W - X * rhs.X - Y * rhs.Y - Z * rhs.Z;
    return Quaternion(newX, newY, newZ, newW);
}

constexpr Quaternion& Quaternion::operator*=(const Quaternion& rhs)
{
    *this = *this * rhs;
    return *this;
}

constexpr Quaternion Quaternion::operator*(float scalar) const
{
    return Quaternion(scalar * X, scalar * Y, scalar * Z, scalar * W);
}

constexpr Quaternion& Quaternion::operator*=(float scalar)
{
    X *= scalar;
    Y *= scalar;
    Z *= scalar;
    W *= scalar;
    return *this;
}

constexpr Quaternion Quaternion::operator/(float scalar) const
{
    const float invScalar = 1.0f / scalar;
    return *this * invScalar;
}

constexpr Quaternion& Quaternion::operator/=(float scalar)
{
    const float invScalar = 1.0f / scalar;
    X *= invScalar;
    Y *= invScalar;
    Z *= invScalar;
    W *= invScalar;
    return *this;
}

constexpr Quaternion Quaternion::operator-() const
{
    return Quaternion(-X, -Y, -Z, -W);
}

[[nodiscard]] constexpr Quaternion operator*(float scalar, const Quaternion& q)
{
    return q * scalar;
}

constexpr bool Quaternion::operator==(const Quaternion& rhs) const
{
    return X == rhs.X && Y == rhs.Y && Z == rhs.Z && W == rhs.W;
}

constexpr bool Quaternion::operator!=(const Quaternion& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Quaternion::IsEqualApprox(const Quaternion& rhs) const
{
    return math::IsEqualApprox(X, rhs.X) && math::IsEqualApprox(Y, rhs.Y) && math::IsEqualApprox(Z, rhs.Z) &&
           math::IsEqualApprox(W, rhs.W);
}

constexpr bool Quaternion::IsNotEqualApprox(const Quaternion& rhs) const
{
    return !IsEqualApprox(rhs);
}

constexpr float Quaternion::Dot(const Quaternion& rhs) const
{
    return X * rhs.X + Y * rhs.Y + Z * rhs.Z + W * rhs.W;
}

float Quaternion::Magnitude() const
{
    return math::Sqrt(SqrdMagnitude());
}

constexpr float Quaternion::SqrdMagnitude() const
{
    return X * X + Y * Y + Z * Z + W * W;
}

void Quaternion::Normalize()
{
    const float invNorm = 1.0f / Magnitude();
    *this *= invNorm;
}

Quaternion Quaternion::Normalized() const
{
    Quaternion copy = *this;
    copy.Normalize();
    return copy;
}

constexpr void Quaternion::Conjugate()
{
    X = -X;
    Y = -Y;
    Z = -Z;
}

constexpr Quaternion Quaternion::Conjugated() const
{
    return Quaternion(-X, -Y, -Z, W);
}

constexpr bool Quaternion::IsUnit() const
{
    return SqrdMagnitude() == 1.0f;
}

constexpr bool Quaternion::IsUnitApprox() const
{
    return math::IsEqualApprox(SqrdMagnitude(), 1.0f);
}

constexpr void Quaternion::Invert()
{
    Conjugate();
    const float invMag = 1.0f / SqrdMagnitude();
    X *= invMag;
    Y *= invMag;
    Z *= invMag;
    W *= invMag;
}

constexpr Quaternion Quaternion::Inverse() const
{
    Quaternion copy = *this;
    copy.Invert();
    return copy;
}

constexpr bool Quaternion::IsPurelyImaginary() const
{
    return math::IsZeroApprox(W);
}

constexpr Vector3 Quaternion::Transform(const Vector3& v) const
{
    const Vector3 u(X, Y, Z);
    const Vector3 uv = u.Cross(v);
    return v + ((uv * W) + u.Cross(uv)) * 2.0f;
}

constexpr Vector3 Quaternion::InvTransform(const Vector3& v) const
{
    return Conjugated().Transform(v);
}
} // namespace ho
