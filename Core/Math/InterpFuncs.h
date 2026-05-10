#pragma once

#include "MathDefs.h"
#include "MathFuncs.h"
#include "Quaternion.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

namespace ho
{
namespace math
{

template <class T>
[[nodiscard]] FORCE_INLINE Vector2 GetBarycentric(const T& target, const T& v1, const T& v2)
{
    const T toV2 = v2 - v1;
    const T toTarget = target - v1;

    const float sqMag = toV2.SqrdMagnitude();
    if (math::IsZeroApprox(sqMag))
    {
        return Vector2(1.0f, 0.0f);
    }

    const float lambda = toV2.Dot(toTarget) / sqMag;

    return Vector2(1.0f - lambda, lambda);
}

template <class T>
[[nodiscard]] FORCE_INLINE Vector3 GetBarycentric(const T& target, const T& v1, const T& v2, const T& v3)
{
    // Standard triangle barycentric formula:
    // a = v1 - v3, b = v2 - v3, c = target - v3
    // d00=a·a, d01=a·b, d11=b·b, d20=c·a, d21=c·b
    // denom = d00*d11 - d01*d01
    // λ1 = (d11*d20 - d01*d21)/denom
    // λ2 = (d00*d21 - d01*d20)/denom
    // λ3 = 1 - λ1 - λ2

    const T toV1 = v1 - v3;
    const T toV2 = v2 - v3;
    const T toTarget = target - v3;

    const float d00 = toV1.Dot(toV1);
    const float d11 = toV2.Dot(toV2);
    const float d01 = toV1.Dot(toV2);
    const float d20 = toTarget.Dot(toV1);
    const float d21 = toTarget.Dot(toV2);

    const float denom = d00 * d11 - d01 * d01;
    if (math::IsZeroApprox(denom))
    {
        // degenerated case
        return Vector3::sZero;
    }
    const float inv = 1.0f / denom;
    const float lambda1 = (d11 * d20 - d01 * d21) * inv;
    const float lambda2 = (d00 * d21 - d01 * d20) * inv;
    return {lambda1, lambda2, 1.0f - lambda1 - lambda2};
}

template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Lerp(const T& v1, const T& v2, const Vector2& barycentric)
{
    return barycentric.X * v1 + barycentric.Y * v2;
}

template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Lerp(const T& v1, const T& v2, const T& v3, const Vector3& barycentric)
{
    return barycentric.X * v1 + barycentric.Y * v2 + barycentric.Z * v3;
}

[[nodiscard]] FORCE_INLINE constexpr float PcerpW(const Vector2& invW, const Vector2& barycentric)
{
    const float denom = (barycentric.X * invW.X + barycentric.Y * invW.Y);

    return 1.0f / denom;
}

[[nodiscard]] FORCE_INLINE constexpr float PcerpW(const Vector3& invW, const Vector3& barycentric)
{
    const float denom = (barycentric.X * invW.X + barycentric.Y * invW.Y + barycentric.Z * invW.Z);
    return 1.0f / denom;
}

template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Pcerp(
    const T& a1, const T& a2, const Vector2& barycentric, const Vector2& invW, float interpolatedW)
{
    return interpolatedW * (invW.X * barycentric.X * a1 + invW.Y * barycentric.Y * a2);
}

template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Pcerp(
    const T& a1, const T& a2, const T& a3, const Vector3& barycentric, const Vector3& invW, float interpolatedW)
{
    return interpolatedW * (invW.X * barycentric.X * a1 + invW.Y * barycentric.Y * a2 + invW.Z * barycentric.Z * a3);
}

template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Pcerp(
    const T& a1, const T& a2, const Vector2& barycentric, const Vector4& v1, const Vector4& v2)
{
    const Vector2 invW = Vector2(1.0f / v1.W, 1.0f / v2.W);
    const float interpolatedW = PcerpW(invW, barycentric);
    return interpolatedW * (invW.X * barycentric.X * a1 + invW.Y * barycentric.Y * a2);
}

template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Pcerp(const T& a1,
                                             const T& a2,
                                             const T& a3,
                                             const Vector3& barycentric,
                                             const Vector4& v1,
                                             const Vector4& v2,
                                             const Vector4& v3)
{
    const Vector3 invW = Vector3(1.0f / v1.W, 1.0f / v2.W, 1.0f / v3.W);
    const float interpolatedW = PcerpW(invW, barycentric);
    return interpolatedW * (invW.X * barycentric.X * a1 + invW.Y * barycentric.Y * a2 + invW.Z * barycentric.Z * a3);
}

[[nodiscard]] FORCE_INLINE Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, const Vector2& barycentric)
{
    float cos = q1.Dot(q2);
    if (math::IsEqualApprox(cos, 1.0f) || math::IsEqualApprox(cos, -1.0f))
    {
        return q1;
    }

    Quaternion adjQ2 = q2;

    if (cos < 0.0f)
    {
        cos = -cos;
        adjQ2 = -q2;
    }

    if (cos > 0.9995f)
    {
        const Quaternion result = barycentric.X * q1 + barycentric.Y * adjQ2;
        return result.Normalized();
    }

    const float angle = math::Acos(cos);
    const float invSin = 1.0f / math::Sin(angle);
    const float alpha = math::Sin(barycentric.X * angle) * invSin;
    const float beta = math::Sin(barycentric.Y * angle) * invSin;
    return alpha * q1 + beta * adjQ2;
}

[[nodiscard]] FORCE_INLINE Quaternion SlerpLong(const Quaternion& q1, const Quaternion& q2, const Vector2& barycentric)
{
    float cos = q1.Dot(q2);
    if (math::IsEqualApprox(cos, 1.0f) || math::IsEqualApprox(cos, -1.0f))
    {
        return q1;
    }

    Quaternion adjQ2 = q2;

    if (cos > 0.0f)
    {
        cos = -cos;
        adjQ2 = -q2;
    }

    if (cos < -0.9995f)
    {
        return q1;
    }

    const float angle = math::Acos(cos);
    const float invSin = 1.0f / math::Sin(angle);
    const float alpha = math::Sin(barycentric.X * angle) * invSin;
    const float beta = math::Sin(barycentric.Y * angle) * invSin;
    return alpha * q1 + beta * adjQ2;
}

}; // namespace math
} // namespace ho
