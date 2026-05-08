#pragma once

#include <cmath>
#include <cstdint>
#if defined(_MSC_VER) || defined(__SSE__)
#include <xmmintrin.h>
#endif

#include "MathDefs.h"

namespace ho
{
namespace math
{
template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Max(T x, T y)
{
    return x < y ? y : x;
}

template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Max(T x, T y, T z)
{
    return Max(Max(x, y), z);
}

template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Min(T x, T y)
{
    return x < y ? x : y;
}

template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Min(T x, T y, T z)
{
    return Min(Min(x, y), z);
}

template <class T>
[[nodiscard]] FORCE_INLINE constexpr T Clamp(T target, T min, T max)
{
    HO_ASSERT(min <= max, "min parameter must be less than max.");
    return target < min ? min : (target > max ? max : target);
}

template <typename T>
[[nodiscard]] FORCE_INLINE constexpr T Abs(T x)
{
    return (x < static_cast<T>(0)) ? -x : x;
}

[[nodiscard]] FORCE_INLINE constexpr float Fmod(float x, float y)
{
    if (y == 0.0f)
    {
        return static_cast<float>(0);
    }
    const float quotient = static_cast<float>(static_cast<long long>(x / y));
    return x - quotient * y;
}

[[nodiscard]] FORCE_INLINE float Round(float x)
{
    return std::round(x);
}

[[nodiscard]] FORCE_INLINE constexpr float Floor(float x)
{
    const long long i = static_cast<long long>(x);
    return (x < 0 && static_cast<float>(i) != x) ? static_cast<float>(i - 1) : static_cast<float>(i);
}

[[nodiscard]] FORCE_INLINE constexpr float Ceil(float x)
{
    const long long i = static_cast<long long>(x);
    return (x > 0 && static_cast<float>(i) != x) ? static_cast<float>(i + 1) : static_cast<float>(i);
}

[[nodiscard]] FORCE_INLINE constexpr bool IsEqualApprox(float lhs, float rhs, float tolerance)
{
    if (lhs == rhs)
    {
        return true;
    }
    return math::Abs(lhs - rhs) < tolerance;
}

[[nodiscard]] FORCE_INLINE constexpr bool IsEqualApprox(float lhs, float rhs)
{
    if (lhs == rhs)
    {
        return true;
    }
    float tolerance = EPSILON_CMP * math::Abs(lhs);
    if (tolerance < EPSILON_CMP)
    {
        tolerance = EPSILON_CMP;
    }
    return math::Abs(lhs - rhs) < tolerance;
}

[[nodiscard]] FORCE_INLINE constexpr bool IsNotEqualApprox(float lhs, float rhs, float tolerance)
{
    return !IsEqualApprox(lhs, rhs, tolerance);
}

[[nodiscard]] FORCE_INLINE constexpr bool IsNotEqualApprox(float lhs, float rhs)
{
    return !IsEqualApprox(lhs, rhs);
}

[[nodiscard]] FORCE_INLINE constexpr bool IsZeroApprox(float value)
{
    return math::Abs(value) < EPSILON_CMP;
}

[[nodiscard]] FORCE_INLINE bool IsFinite(float n)
{
    return std::isfinite(n);
}

[[nodiscard]] FORCE_INLINE bool IsInf(float n)
{
    return std::isinf(n);
}

[[nodiscard]] FORCE_INLINE bool IsNaN(float n)
{
    return std::isnan(n);
}

[[nodiscard]] FORCE_INLINE float Pow(float base, float exp)
{
    return std::pow(base, exp);
}

[[nodiscard]] INLINE constexpr float PowFast(float base, int32_t exp)
{
    if (exp == 0)
    {
        return 1.0f;
    }

    if (base == 0.0f)
    {
        return (exp < 0) ? math::FLOAT_INF : 0.0f;
    }

    bool bNegativeExp = (exp < 0);
    int32_t e = bNegativeExp ? -(exp + 1) : exp;
    if (bNegativeExp)
    {
        ++e;
    }

    float b = base;
    float result = 1.0f;

    if (bNegativeExp)
    {
        b = 1.0f / b;
    }

    while (e)
    {
        if (e & 1u)
        {
            result *= b;
        }
        b *= b;
        e >>= 1u;
    }

    return result;
}

[[nodiscard]] FORCE_INLINE float InvSqrtFast(float f) noexcept
{
    // Performs two passes of Newton-Raphson iteration on the hardware estimate
    //    v^-0.5 = x
    // => x^2 = v^-1
    // => 1/(x^2) = v
    // => F(x) = x^-2 - v
    //    F'(x) = -2x^-3

    //    x1 = x0 - F(x0)/F'(x0)
    // => x1 = x0 + 0.5 * (x0^-2 - Vec) * x0^3
    // => x1 = x0 + 0.5 * (x0 - Vec * x0^3)
    // => x1 = x0 + x0 * (0.5 - 0.5 * Vec * x0^2)
    //
    // This final form has one more operation than the legacy factorization (X1 =
    // 0.5*X0*(3-(Y*X0)*X0) but retains better accuracy (namely InvSqrt(1) = 1
    // exactly).
    if (f < 0.0f)
    {
        return math::FLOAT_NaN;
    }
    if (f == 0.0f)
    {
        return math::FLOAT_INF;
    }

#if defined(_MSC_VER) || defined(__SSE__)
    const __m128 fOneHalf = _mm_set_ss(0.5f);
    __m128 y0 = _mm_set_ss(f);
    __m128 x0 = _mm_rsqrt_ss(y0);
    __m128 fOver2 = _mm_mul_ss(y0, fOneHalf);

    // 1st Newton-Raphson iteration
    __m128 x1 = _mm_mul_ss(x0, x0);
    x1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(fOver2, x1));
    x1 = _mm_add_ss(x0, _mm_mul_ss(x0, x1));

    // 2nd Newton-Raphson iteration
    __m128 x2 = _mm_mul_ss(x1, x1);
    x2 = _mm_sub_ss(fOneHalf, _mm_mul_ss(fOver2, x2));
    x2 = _mm_add_ss(x1, _mm_mul_ss(x1, x2));
    float out;
    _mm_store_ss(&out, x2);
    return out;
#else

    return 1.0f / std::sqrt(f);
#endif
}

[[nodiscard]] FORCE_INLINE double InvSqrtFast(double f) noexcept
{
    return 1.0 / std::sqrt(f);
}

[[nodiscard]] FORCE_INLINE float SqrtFast(float f) noexcept
{
    return f * InvSqrtFast(f);
}

[[nodiscard]] FORCE_INLINE float Sqrt(float f) noexcept
{
    return std::sqrt(f);
}

[[nodiscard]] FORCE_INLINE double Sqrt(double f) noexcept
{
    return std::sqrt(f);
}

[[nodiscard]] FORCE_INLINE float Sin(float radian)
{
    return std::sin(radian);
}

[[nodiscard]] FORCE_INLINE float Cos(float radian)
{
    return std::cos(radian);
}

[[nodiscard]] FORCE_INLINE float Tan(float radian)
{
    return std::tan(radian);
}

[[nodiscard]] FORCE_INLINE float Asin(float radian)
{
    return std::asin(radian);
}

[[nodiscard]] FORCE_INLINE float Acos(float radian)
{
    return std::acos(radian);
}

[[nodiscard]] FORCE_INLINE float Atan(float radian)
{
    return std::atan(radian);
}

[[nodiscard]] FORCE_INLINE float Atan2(float y, float x)
{
    return std::atan2(y, x);
}

// NOLINTBEGIN(bugprone-incorrect-roundings)
FORCE_INLINE void SinCos(float* outSin, float* outCos, float radian)
{
    // Copied from UE4 Source Code
    // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
    float quotient = (INV_PI * 0.5f) * radian;
    if (radian >= 0.0f)
    {
        quotient = static_cast<float>(static_cast<int32_t>(quotient + 0.5f));
    }
    else
    {
        quotient = static_cast<float>(static_cast<int32_t>(quotient - 0.5f));
    }
    float y = radian - (2.0f * PI) * quotient;

    // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
    float sign = 0.0f;
    if (y > PI2)
    {
        y = PI - y;
        sign = -1.0f;
    }
    else if (y < -PI2)
    {
        y = -PI - y;
        sign = -1.0f;
    }
    else
    {
        sign = +1.0f;
    }

    float y2 = y * y;

    // 11-degree minimax approximation
    *outSin =
        (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) *
             y2 +
         1.0f) *
        y;

    // 10-degree minimax approximation
    float p =
        ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
    *outCos = sign * p;
}

// NOLINTEND(bugprone-incorrect-roundings)

[[nodiscard]] FORCE_INLINE constexpr float DegToRad(float deg)
{
    return deg * DEG1_IN_RAD;
}

[[nodiscard]] FORCE_INLINE constexpr float RadToDeg(float rad)
{
    return rad * RAD1_IN_DEG;
}

template <class T>
[[nodiscard]] INLINE float Distance(const T& vec1, const T& vec2)
{
    const T d = (vec1 - vec2);
    return Sqrt(d.Dot(d));
}

template <class T>
[[nodiscard]] INLINE constexpr float SqrdDistance(const T& vec1, const T& vec2)
{
    const T d = (vec1 - vec2);
    return d.Dot(d);
}

} // namespace math
} // namespace ho