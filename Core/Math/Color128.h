#pragma once

#include <string>

#include "Core/Macros.h"
#include "MathDefs.h"
#include "MathFuncs.h"

namespace ho
{
struct Color32;
struct Vector4;

struct alignas(16) Color128
{
    constexpr Color128();
    constexpr Color128(const Color128&) = default;
    constexpr Color128(float r, float g, float b, float a = 1.f);
    explicit Color128(const Color32& c);
    explicit Color128(const Vector4& v);
    constexpr Color128& operator=(const Color128& rhs);
    ~Color128() = default;

    [[nodiscard]] constexpr Color128 operator+(const Color128& rhs) const;
    constexpr Color128& operator+=(const Color128& rhs);

    [[nodiscard]] constexpr Color128 operator-(const Color128& rhs) const;
    constexpr Color128& operator-=(const Color128& rhs);

    [[nodiscard]] constexpr Color128 operator*(const Color128& rhs) const;
    constexpr Color128& operator*=(const Color128& rhs);

    [[nodiscard]] constexpr Color128 operator*(float scalar) const;
    constexpr Color128& operator*=(float scalar);

    [[nodiscard]] constexpr Color128 operator/(const Color128& rhs) const;
    constexpr Color128& operator/=(const Color128& rhs);

    [[nodiscard]] constexpr Color128 operator/(float scalar) const;
    constexpr Color128& operator/=(float scalar);

    [[nodiscard]] constexpr bool operator==(const Color128& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Color128& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Color128& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Color128& rhs) const;

    [[nodiscard]] constexpr float Luminance() const;
    [[nodiscard]] constexpr float Luminance(float rWeight, float gWeight, float bWeight) const;

    [[nodiscard]] constexpr Color128 Over(const Color128& background) const;
    [[nodiscard]] constexpr Color128 Under(const Color128& background) const;

    [[nodiscard]] FORCE_INLINE Color128 sRGBToLinear() const;
    [[nodiscard]] FORCE_INLINE Color128 LinearTosRGB() const;

    [[nodiscard]] std::string ToString() const;

    float R;
    float G;
    float B;
    float A;
};

constexpr Color128::Color128()
  : R(0.0f)
  , G(0.0f)
  , B(0.0f)
  , A(1.0f)
{
}

constexpr Color128::Color128(float r, float g, float b, float a)
  : R(r)
  , G(g)
  , B(b)
  , A(a)
{
}

constexpr Color128& Color128::operator=(const Color128& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    R = rhs.R;
    G = rhs.G;
    B = rhs.B;
    A = rhs.A;
    return *this;
}

constexpr bool Color128::operator==(const Color128& rhs) const
{
    return R == rhs.R && G == rhs.G && B == rhs.B && A == rhs.A;
}

constexpr bool Color128::operator!=(const Color128& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Color128::IsEqualApprox(const Color128& rhs) const
{
    return math::IsEqualApprox(R, rhs.R) && math::IsEqualApprox(G, rhs.G) && math::IsEqualApprox(B, rhs.B) &&
           math::IsEqualApprox(A, rhs.A);
}

constexpr bool Color128::IsNotEqualApprox(const Color128& rhs) const
{
    return !IsEqualApprox(rhs);
}

constexpr Color128 Color128::operator+(const Color128& rhs) const
{
    return Color128(R + rhs.R, G + rhs.G, B + rhs.B, A + rhs.A);
}

constexpr Color128& Color128::operator+=(const Color128& rhs)
{
    R += rhs.R;
    G += rhs.G;
    B += rhs.B;
    A += rhs.A;
    return *this;
}

constexpr Color128 Color128::operator-(const Color128& rhs) const
{
    return Color128(R - rhs.R, G - rhs.G, B - rhs.B, A - rhs.A);
}

constexpr Color128& Color128::operator-=(const Color128& rhs)
{
    R -= rhs.R;
    G -= rhs.G;
    B -= rhs.B;
    A -= rhs.A;
    return *this;
}

constexpr Color128 Color128::operator*(const Color128& rhs) const
{
    return Color128(R * rhs.R, G * rhs.G, B * rhs.B, A * rhs.A);
}

constexpr Color128& Color128::operator*=(const Color128& rhs)
{
    R *= rhs.R;
    G *= rhs.G;
    B *= rhs.B;
    A *= rhs.A;
    return *this;
}

constexpr Color128 Color128::operator*(float scalar) const
{
    return Color128(R * scalar, G * scalar, B * scalar, A * scalar);
}

constexpr Color128& Color128::operator*=(float scalar)
{
    R *= scalar;
    G *= scalar;
    B *= scalar;
    A *= scalar;
    return *this;
}

constexpr Color128 Color128::operator/(const Color128& rhs) const
{
    return Color128(R / rhs.R, G / rhs.G, B / rhs.B, A / rhs.A);
}

constexpr Color128& Color128::operator/=(const Color128& rhs)
{
    R /= rhs.R;
    G /= rhs.G;
    B /= rhs.B;
    A /= rhs.A;
    return *this;
}

constexpr Color128 Color128::operator/(float scalar) const
{
    return Color128(R / scalar, G / scalar, B / scalar, A / scalar);
}

constexpr Color128& Color128::operator/=(float scalar)
{
    R /= scalar;
    G /= scalar;
    B /= scalar;
    A /= scalar;
    return *this;
}

FORCE_INLINE constexpr Color128 operator*(float scalar, const Color128& c)
{
    return c * scalar;
}

// weights as listed in
// https://en.wikipedia.org/wiki/Relative_luminance
constexpr float Color128::Luminance() const
{
    return 0.2126f * R + 0.7152f * G + 0.0722f * B;
}

constexpr float Color128::Luminance(float rWeight, float gWeight, float bWeight) const
{
    return R * rWeight + G * gWeight + B * bWeight;
}

// Implement over operation(*this over background) in Porter-Duff Blending
// Model https://keithp.com/~keithp/porterduff/p253-porter.pdf
constexpr Color128 Color128::Over(const Color128& background) const
{
    Color128 res;
    const float transparency = 1.0f - A; // (1 - α_A)
    res.A = A + background.A * transparency;

    if (res.A == 0.f)
    {
        return Color128(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        const float invA = 1.0f / res.A;
        res.R = (R * A + background.R * background.A * transparency) * invA;
        res.G = (G * A + background.G * background.A * transparency) * invA;
        res.B = (B * A + background.B * background.A * transparency) * invA;
    }
    return res;
}

constexpr Color128 Color128::Under(const Color128& foreground) const
{
    {
        Color128 res;
        const float transparency = 1.0f - foreground.A;
        res.A = A * transparency + foreground.A;
        if (res.A == 0.0f)
        {
            return Color128(0.0f, 0.0f, 0.0f, 0.0f);
        }
        else
        {
            const float invA = 1.0f / res.A;
            res.R = (R * A * transparency + foreground.R * foreground.A) * invA;
            res.G = (G * A * transparency + foreground.G * foreground.A) * invA;
            res.B = (B * A * transparency + foreground.B * foreground.A) * invA;
        }
        return res;
    }
}

Color128 Color128::sRGBToLinear() const
{
    auto toLinear = [](float c)
    {
        if (c <= 0.04045f)
        {
            return c / 12.92f;
        }
        return math::Pow((c + 0.055f) / 1.055f, 2.4f);
    };
    return Color128(toLinear(R), toLinear(G), toLinear(B), A);
}

Color128 Color128::LinearTosRGB() const
{
    auto toSRGB = [](float c)
    {
        if (c <= 0.0031308f)
        {
            return c * 12.92f;
        }
        return 1.055f * math::Pow(c, 1.0f / 2.4f) - 0.055f;
    };
    return Color128(toSRGB(R), toSRGB(G), toSRGB(B), A);
}
} // namespace ho
