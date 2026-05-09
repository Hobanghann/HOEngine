#pragma once

#include <cstdint>
#include <string>

#include "MathDefs.h"
#include "MathFuncs.h"

namespace ho
{
struct Color128;

struct alignas(4) Color32
{
  public:
    constexpr Color32();
    constexpr Color32(const Color32&) = default;
    constexpr Color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    explicit Color32(const Color128& c);
    constexpr Color32& operator=(const Color32& rhs);
    ~Color32() = default;

    [[nodiscard]] constexpr Color32 operator+(const Color32 rhs) const;
    constexpr Color32& operator+=(const Color32 rhs);

    [[nodiscard]] constexpr Color32 operator-(const Color32 rhs) const;
    constexpr Color32& operator-=(const Color32 rhs);

    [[nodiscard]] constexpr Color32 operator*(const Color32 rhs) const;
    constexpr Color32& operator*=(const Color32 rhs);

    [[nodiscard]] constexpr Color32 operator*(uint8_t scalar) const;
    constexpr Color32& operator*=(uint8_t scalar);

    [[nodiscard]] constexpr Color32 operator/(const Color32 rhs) const;
    constexpr Color32& operator/=(const Color32 rhs);

    [[nodiscard]] constexpr Color32 operator/(uint8_t scalar) const;
    constexpr Color32& operator/=(uint8_t scalar);

    [[nodiscard]] constexpr bool operator==(const Color32 rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Color32 rhs) const;

    [[nodiscard]] constexpr float Luminance() const;
    [[nodiscard]] constexpr float Luminance(float rWeight, float gWeight, float bWeight) const;

    [[nodiscard]] FORCE_INLINE uint8_t ToR8() const;
    [[nodiscard]] FORCE_INLINE uint16_t ToRG8() const;
    [[nodiscard]] FORCE_INLINE uint32_t ToRGB8() const;
    [[nodiscard]] FORCE_INLINE uint32_t ToRGBA8() const;
    [[nodiscard]] FORCE_INLINE uint32_t ToBGRA8() const;
    [[nodiscard]] FORCE_INLINE uint32_t ToARGB8() const;
    [[nodiscard]] FORCE_INLINE uint32_t ToABGR8() const;
    [[nodiscard]] FORCE_INLINE uint8_t ToL8() const;
    [[nodiscard]] FORCE_INLINE uint16_t ToLA8() const;

    [[nodiscard]] std::string ToString() const;

    static const Color32 sAliceBlue;
    static const Color32 sAntiqueWhite;
    static const Color32 sAqua;
    static const Color32 sAquamarine;
    static const Color32 sAzure;
    static const Color32 sBeige;
    static const Color32 sBisque;
    static const Color32 sBlack;
    static const Color32 sBlanchedAlmond;
    static const Color32 sBlue;
    static const Color32 sBlueViolet;
    static const Color32 sBrown;
    static const Color32 sBurlywood;
    static const Color32 sCadetBlue;
    static const Color32 sChartreuse;
    static const Color32 sChocolate;
    static const Color32 sCoral;
    static const Color32 sCornflowerBlue;
    static const Color32 sCornsilk;
    static const Color32 sCrimson;
    static const Color32 sCyan;
    static const Color32 sDarkBlue;
    static const Color32 sDarkCyan;
    static const Color32 sDarkGoldenrod;
    static const Color32 sDarkGray;
    static const Color32 sDarkGreen;
    static const Color32 sDarkKhaki;
    static const Color32 sDarkMagenta;
    static const Color32 sDarkOliveGreen;
    static const Color32 sDarkOrange;
    static const Color32 sDarkOrchid;
    static const Color32 sDarkRed;
    static const Color32 sDarkSalmon;
    static const Color32 sDarkSeaGreen;
    static const Color32 sDarkSlateBlue;
    static const Color32 sDarkSlateGray;
    static const Color32 sDarkTurquoise;
    static const Color32 sDarkViolet;
    static const Color32 sDeepPink;
    static const Color32 sDeepSkyBlue;
    static const Color32 sDimGray;
    static const Color32 sDodgerBlue;
    static const Color32 sFirebrick;
    static const Color32 sFloralWhite;
    static const Color32 sForestGreen;
    static const Color32 sFuchsia;
    static const Color32 sGainsboro;
    static const Color32 sGhostWhite;
    static const Color32 sGold;
    static const Color32 sGoldenrod;
    static const Color32 sGray;
    static const Color32 sGreen;
    static const Color32 sGreenYellow;
    static const Color32 sHoneydew;
    static const Color32 sHotPink;
    static const Color32 sIndianRed;
    static const Color32 sIndigo;
    static const Color32 sIvory;
    static const Color32 sKhaki;
    static const Color32 sLavender;
    static const Color32 sLavenderBlush;
    static const Color32 sLawnGreen;
    static const Color32 sLemonChiffon;
    static const Color32 sLightBlue;
    static const Color32 sLightCoral;
    static const Color32 sLightCyan;
    static const Color32 sLightGoldenrod;
    static const Color32 sLightGray;
    static const Color32 sLightGreen;
    static const Color32 sLightPink;
    static const Color32 sLightSalmon;
    static const Color32 sLightSeaGreen;
    static const Color32 sLightSkyBlue;
    static const Color32 sLightSlateGray;
    static const Color32 sLightSteelBlue;
    static const Color32 sLightYellow;
    static const Color32 sLime;
    static const Color32 sLimeGreen;
    static const Color32 sLinen;
    static const Color32 sMagenta;
    static const Color32 sMaroon;
    static const Color32 sMediumAquamarine;
    static const Color32 sMediumBlue;
    static const Color32 sMediumOrchid;
    static const Color32 sMediumPurple;
    static const Color32 sMediumSeaGreen;
    static const Color32 sMediumSlateBlue;
    static const Color32 sMediumSpringGreen;
    static const Color32 sMediumTurquoise;
    static const Color32 sMediumVioletRed;
    static const Color32 sMidnightBlue;
    static const Color32 sMintCream;
    static const Color32 sMistyRose;
    static const Color32 sMoccasin;
    static const Color32 sNavajoWhite;
    static const Color32 sNavyBlue;
    static const Color32 sOldLace;
    static const Color32 sOlive;
    static const Color32 sOliveDrab;
    static const Color32 sOrange;
    static const Color32 sOrangeRed;
    static const Color32 sOrchid;
    static const Color32 sPaleGoldenrod;
    static const Color32 sPaleGreen;
    static const Color32 sPaleTurquoise;
    static const Color32 sPaleVioletRed;
    static const Color32 sPapayaWhip;
    static const Color32 sPeachPuff;
    static const Color32 sPeru;
    static const Color32 sPink;
    static const Color32 sPlum;
    static const Color32 sPowderBlue;
    static const Color32 sPurple;
    static const Color32 sRebeccaPurple;
    static const Color32 sRed;
    static const Color32 sRosyBrown;
    static const Color32 sRoyalBlue;
    static const Color32 sSaddleBrown;
    static const Color32 sSalmon;
    static const Color32 sSandyBrown;
    static const Color32 sSeaGreen;
    static const Color32 sSeashell;
    static const Color32 sSienna;
    static const Color32 sSilver;
    static const Color32 sSkyBlue;
    static const Color32 sSlateBlue;
    static const Color32 sSlateGray;
    static const Color32 sSnow;
    static const Color32 sSpringGreen;
    static const Color32 sSteelBlue;
    static const Color32 sTan;
    static const Color32 sTeal;
    static const Color32 sThistle;
    static const Color32 sTomato;
    static const Color32 sTurquoise;
    static const Color32 sViolet;
    static const Color32 sWebGray;
    static const Color32 sWebGreen;
    static const Color32 sWebMaroon;
    static const Color32 sWebPurple;
    static const Color32 sWheat;
    static const Color32 sWhite;
    static const Color32 sWhiteSmoke;
    static const Color32 sYellow;
    static const Color32 sYellowGreen;

    union
    {
        struct
        {
            uint8_t R, G, B, A;
        };

        uint32_t RGBA = 0;
    };

  private:
    // internal helpers
    [[nodiscard]] constexpr uint8_t ceilUint8(int v) const
    {
        return static_cast<uint8_t>(math::Min(255, v));
    }

    [[nodiscard]] constexpr uint8_t floorUint8(int v) const
    {
        return static_cast<uint8_t>(math::Max(v, 0));
    }

    [[nodiscard]] constexpr uint8_t divUint8(uint8_t x, uint8_t y) const
    {
        return y == 0 ? 255 : static_cast<uint8_t>(static_cast<float>(x) / static_cast<float>(y));
    }
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
constexpr Color32::Color32()
  : R(0)
  , G(0)
  , B(0)
  , A(255)
{
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
constexpr Color32::Color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
  : R(r)
  , G(g)
  , B(b)
  , A(a)
{
}

constexpr Color32& Color32::operator=(const Color32& rhs)
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

constexpr bool Color32::operator==(const Color32 rhs) const
{
    return R == rhs.R && G == rhs.G && B == rhs.B && A == rhs.A;
}

constexpr bool Color32::operator!=(const Color32 rhs) const
{
    return !(*this == rhs);
}

constexpr Color32 Color32::operator+(const Color32 rhs) const
{
    return Color32(ceilUint8(R + rhs.R), ceilUint8(G + rhs.G), ceilUint8(B + rhs.B), ceilUint8(A + rhs.A));
}

constexpr Color32& Color32::operator+=(const Color32 rhs)
{
    R = ceilUint8(R + rhs.R);
    G = ceilUint8(G + rhs.G);
    B = ceilUint8(B + rhs.B);
    A = ceilUint8(A + rhs.A);
    return *this;
}

constexpr Color32 Color32::operator-(const Color32 rhs) const
{
    return Color32(floorUint8(R - rhs.R), floorUint8(G - rhs.G), floorUint8(B - rhs.B), floorUint8(A - rhs.A));
}

constexpr Color32& Color32::operator-=(const Color32 rhs)
{
    R = floorUint8(R - rhs.R);
    G = floorUint8(G - rhs.G);
    B = floorUint8(B - rhs.B);
    A = floorUint8(A - rhs.A);
    return *this;
}

constexpr Color32 Color32::operator*(const Color32 rhs) const
{
    return Color32(ceilUint8(R * rhs.R), ceilUint8(G * rhs.G), ceilUint8(B * rhs.B), ceilUint8(A * rhs.A));
}

constexpr Color32& Color32::operator*=(const Color32 rhs)
{
    R = ceilUint8(R * rhs.R);
    G = ceilUint8(G * rhs.G);
    B = ceilUint8(B * rhs.B);
    A = ceilUint8(A * rhs.A);
    return *this;
}

constexpr Color32 Color32::operator*(uint8_t scalar) const
{
    return Color32(ceilUint8(R * scalar), ceilUint8(G * scalar), ceilUint8(B * scalar), ceilUint8(A * scalar));
}

constexpr Color32& Color32::operator*=(uint8_t scalar)
{
    R = ceilUint8(R * scalar);
    G = ceilUint8(G * scalar);
    B = ceilUint8(B * scalar);
    A = ceilUint8(A * scalar);
    return *this;
}

constexpr Color32 Color32::operator/(const Color32 rhs) const
{
    return Color32(divUint8(R, rhs.R), divUint8(G, rhs.G), divUint8(B, rhs.B), divUint8(A, rhs.A));
}

constexpr Color32& Color32::operator/=(const Color32 rhs)
{
    R = divUint8(R, rhs.R);
    G = divUint8(G, rhs.G);
    B = divUint8(B, rhs.B);
    A = divUint8(A, rhs.A);
    return *this;
}

constexpr Color32 Color32::operator/(uint8_t scalar) const
{
    if (scalar == 0)
    {
        return Color32(255, 255, 255, 255);
    }
    return Color32(divUint8(R, scalar), divUint8(G, scalar), divUint8(B, scalar), divUint8(A, scalar));
}

constexpr Color32& Color32::operator/=(uint8_t scalar)
{
    if (scalar == 0)
    {
        R = 255;
        G = 255;
        B = 255;
        A = 255;
    }
    R = divUint8(R, scalar);
    G = divUint8(G, scalar);
    B = divUint8(B, scalar);
    A = divUint8(A, scalar);
    return *this;
}

FORCE_INLINE constexpr Color32 operator*(uint8_t scalar, const Color32& c)
{
    return c * scalar;
}

constexpr float Color32::Luminance() const
{
    return 0.2126f * static_cast<float>(R) + 0.7152f * static_cast<float>(G) + 0.0722f * static_cast<float>(B);
}

constexpr float Color32::Luminance(float rWeight, float gWeight, float bWeight) const
{
    return static_cast<float>(R) * rWeight + static_cast<float>(G) * gWeight + static_cast<float>(B) * bWeight;
}

uint8_t Color32::ToR8() const
{
    return R;
}

uint16_t Color32::ToRG8() const
{
    return (static_cast<uint16_t>(G) << 8) | R;
}

uint32_t Color32::ToRGB8() const
{
    return (static_cast<uint32_t>(B) << 16) | (static_cast<uint32_t>(G) << 8) | R;
}

uint32_t Color32::ToRGBA8() const
{
    return RGBA;
}

uint32_t Color32::ToBGRA8() const
{
    return (static_cast<uint32_t>(A) << 24) | (static_cast<uint32_t>(R) << 16) | (static_cast<uint32_t>(G) << 8) | B;
}

uint32_t Color32::ToARGB8() const
{
    return (static_cast<uint32_t>(B) << 24) | (static_cast<uint32_t>(G) << 16) | (static_cast<uint32_t>(R) << 8) | A;
}

uint32_t Color32::ToABGR8() const
{
    return (static_cast<uint32_t>(R) << 24) | (static_cast<uint32_t>(G) << 16) | (static_cast<uint32_t>(B) << 8) | A;
}

uint8_t Color32::ToL8() const
{
    return R;
}

uint16_t Color32::ToLA8() const
{
    return (static_cast<uint16_t>(A) << 8) | R;
}

INLINE constexpr Color32 Color32::sAliceBlue = Color32(240, 248, 255, 255);
INLINE constexpr Color32 Color32::sAntiqueWhite = Color32(250, 235, 215, 255);
INLINE constexpr Color32 Color32::sAqua = Color32(0, 255, 255, 255);
INLINE constexpr Color32 Color32::sAquamarine = Color32(127, 255, 212, 255);
INLINE constexpr Color32 Color32::sAzure = Color32(240, 255, 255, 255);
INLINE constexpr Color32 Color32::sBeige = Color32(245, 245, 220, 255);
INLINE constexpr Color32 Color32::sBisque = Color32(255, 228, 196, 255);
INLINE constexpr Color32 Color32::sBlack = Color32(0, 0, 0, 255);
INLINE constexpr Color32 Color32::sBlanchedAlmond = Color32(255, 235, 205, 255);
INLINE constexpr Color32 Color32::sBlue = Color32(0, 0, 255, 255);
INLINE constexpr Color32 Color32::sBlueViolet = Color32(138, 43, 226, 255);
INLINE constexpr Color32 Color32::sBrown = Color32(165, 42, 42, 255);
INLINE constexpr Color32 Color32::sBurlywood = Color32(222, 184, 135, 255);
INLINE constexpr Color32 Color32::sCadetBlue = Color32(95, 158, 160, 255);
INLINE constexpr Color32 Color32::sChartreuse = Color32(127, 255, 0, 255);
INLINE constexpr Color32 Color32::sChocolate = Color32(210, 105, 30, 255);
INLINE constexpr Color32 Color32::sCoral = Color32(255, 127, 80, 255);
INLINE constexpr Color32 Color32::sCornflowerBlue = Color32(100, 149, 237, 255);
INLINE constexpr Color32 Color32::sCornsilk = Color32(255, 248, 220, 255);
INLINE constexpr Color32 Color32::sCrimson = Color32(220, 20, 60, 255);
INLINE constexpr Color32 Color32::sCyan = Color32(0, 255, 255, 255);
INLINE constexpr Color32 Color32::sDarkBlue = Color32(0, 0, 139, 255);
INLINE constexpr Color32 Color32::sDarkCyan = Color32(0, 139, 139, 255);
INLINE constexpr Color32 Color32::sDarkGoldenrod = Color32(184, 134, 11, 255);
INLINE constexpr Color32 Color32::sDarkGray = Color32(169, 169, 169, 255);
INLINE constexpr Color32 Color32::sDarkGreen = Color32(0, 100, 0, 255);
INLINE constexpr Color32 Color32::sDarkKhaki = Color32(189, 183, 107, 255);
INLINE constexpr Color32 Color32::sDarkMagenta = Color32(139, 0, 139, 255);
INLINE constexpr Color32 Color32::sDarkOliveGreen = Color32(85, 107, 47, 255);
INLINE constexpr Color32 Color32::sDarkOrange = Color32(255, 140, 0, 255);
INLINE constexpr Color32 Color32::sDarkOrchid = Color32(153, 50, 204, 255);
INLINE constexpr Color32 Color32::sDarkRed = Color32(139, 0, 0, 255);
INLINE constexpr Color32 Color32::sDarkSalmon = Color32(233, 150, 122, 255);
INLINE constexpr Color32 Color32::sDarkSeaGreen = Color32(143, 188, 143, 255);
INLINE constexpr Color32 Color32::sDarkSlateBlue = Color32(72, 61, 139, 255);
INLINE constexpr Color32 Color32::sDarkSlateGray = Color32(47, 79, 79, 255);
INLINE constexpr Color32 Color32::sDarkTurquoise = Color32(0, 206, 209, 255);
INLINE constexpr Color32 Color32::sDarkViolet = Color32(148, 0, 211, 255);
INLINE constexpr Color32 Color32::sDeepPink = Color32(255, 20, 147, 255);
INLINE constexpr Color32 Color32::sDeepSkyBlue = Color32(0, 191, 255, 255);
INLINE constexpr Color32 Color32::sDimGray = Color32(105, 105, 105, 255);
INLINE constexpr Color32 Color32::sDodgerBlue = Color32(30, 144, 255, 255);
INLINE constexpr Color32 Color32::sFirebrick = Color32(178, 34, 34, 255);
INLINE constexpr Color32 Color32::sFloralWhite = Color32(255, 250, 240, 255);
INLINE constexpr Color32 Color32::sForestGreen = Color32(34, 139, 34, 255);
INLINE constexpr Color32 Color32::sFuchsia = Color32(255, 0, 255, 255);
INLINE constexpr Color32 Color32::sGainsboro = Color32(220, 220, 220, 255);
INLINE constexpr Color32 Color32::sGhostWhite = Color32(248, 248, 255, 255);
INLINE constexpr Color32 Color32::sGold = Color32(255, 215, 0, 255);
INLINE constexpr Color32 Color32::sGoldenrod = Color32(218, 165, 32, 255);
INLINE constexpr Color32 Color32::sGray = Color32(190, 190, 190, 255);
INLINE constexpr Color32 Color32::sGreen = Color32(0, 255, 0, 255);
INLINE constexpr Color32 Color32::sGreenYellow = Color32(173, 255, 47, 255);
INLINE constexpr Color32 Color32::sHoneydew = Color32(240, 255, 240, 255);
INLINE constexpr Color32 Color32::sHotPink = Color32(255, 105, 180, 255);
INLINE constexpr Color32 Color32::sIndianRed = Color32(205, 92, 92, 255);
INLINE constexpr Color32 Color32::sIndigo = Color32(75, 0, 130, 255);
INLINE constexpr Color32 Color32::sIvory = Color32(255, 255, 240, 255);
INLINE constexpr Color32 Color32::sKhaki = Color32(240, 230, 140, 255);
INLINE constexpr Color32 Color32::sLavender = Color32(230, 230, 250, 255);
INLINE constexpr Color32 Color32::sLavenderBlush = Color32(255, 240, 245, 255);
INLINE constexpr Color32 Color32::sLawnGreen = Color32(124, 252, 0, 255);
INLINE constexpr Color32 Color32::sLemonChiffon = Color32(255, 250, 205, 255);
INLINE constexpr Color32 Color32::sLightBlue = Color32(173, 216, 230, 255);
INLINE constexpr Color32 Color32::sLightCoral = Color32(240, 128, 128, 255);
INLINE constexpr Color32 Color32::sLightCyan = Color32(224, 255, 255, 255);
INLINE constexpr Color32 Color32::sLightGoldenrod = Color32(250, 250, 210, 255);
INLINE constexpr Color32 Color32::sLightGray = Color32(211, 211, 211, 255);
INLINE constexpr Color32 Color32::sLightGreen = Color32(144, 238, 144, 255);
INLINE constexpr Color32 Color32::sLightPink = Color32(255, 182, 193, 255);
INLINE constexpr Color32 Color32::sLightSalmon = Color32(255, 160, 122, 255);
INLINE constexpr Color32 Color32::sLightSeaGreen = Color32(32, 178, 170, 255);
INLINE constexpr Color32 Color32::sLightSkyBlue = Color32(135, 206, 250, 255);
INLINE constexpr Color32 Color32::sLightSlateGray = Color32(119, 136, 153, 255);
INLINE constexpr Color32 Color32::sLightSteelBlue = Color32(176, 196, 222, 255);
INLINE constexpr Color32 Color32::sLightYellow = Color32(255, 255, 224, 255);
INLINE constexpr Color32 Color32::sLime = Color32(0, 255, 0, 255);
INLINE constexpr Color32 Color32::sLimeGreen = Color32(50, 205, 50, 255);
INLINE constexpr Color32 Color32::sLinen = Color32(250, 240, 230, 255);
INLINE constexpr Color32 Color32::sMagenta = Color32(255, 0, 255, 255);
INLINE constexpr Color32 Color32::sMaroon = Color32(176, 48, 96, 255);
INLINE constexpr Color32 Color32::sMediumAquamarine = Color32(102, 205, 170, 255);
INLINE constexpr Color32 Color32::sMediumBlue = Color32(0, 0, 205, 255);
INLINE constexpr Color32 Color32::sMediumOrchid = Color32(186, 85, 211, 255);
INLINE constexpr Color32 Color32::sMediumPurple = Color32(147, 112, 219, 255);
INLINE constexpr Color32 Color32::sMediumSeaGreen = Color32(60, 179, 113, 255);
INLINE constexpr Color32 Color32::sMediumSlateBlue = Color32(123, 104, 238, 255);
INLINE constexpr Color32 Color32::sMediumSpringGreen = Color32(0, 250, 154, 255);
INLINE constexpr Color32 Color32::sMediumTurquoise = Color32(72, 209, 204, 255);
INLINE constexpr Color32 Color32::sMediumVioletRed = Color32(199, 21, 133, 255);
INLINE constexpr Color32 Color32::sMidnightBlue = Color32(25, 25, 112, 255);
INLINE constexpr Color32 Color32::sMintCream = Color32(245, 255, 250, 255);
INLINE constexpr Color32 Color32::sMistyRose = Color32(255, 228, 225, 255);
INLINE constexpr Color32 Color32::sMoccasin = Color32(255, 228, 181, 255);
INLINE constexpr Color32 Color32::sNavajoWhite = Color32(255, 222, 173, 255);
INLINE constexpr Color32 Color32::sNavyBlue = Color32(0, 0, 128, 255);
INLINE constexpr Color32 Color32::sOldLace = Color32(253, 245, 230, 255);
INLINE constexpr Color32 Color32::sOlive = Color32(128, 128, 0, 255);
INLINE constexpr Color32 Color32::sOliveDrab = Color32(107, 142, 35, 255);
INLINE constexpr Color32 Color32::sOrange = Color32(255, 165, 0, 255);
INLINE constexpr Color32 Color32::sOrangeRed = Color32(255, 69, 0, 255);
INLINE constexpr Color32 Color32::sOrchid = Color32(218, 112, 214, 255);
INLINE constexpr Color32 Color32::sPaleGoldenrod = Color32(238, 232, 170, 255);
INLINE constexpr Color32 Color32::sPaleGreen = Color32(152, 251, 152, 255);
INLINE constexpr Color32 Color32::sPaleTurquoise = Color32(175, 238, 238, 255);
INLINE constexpr Color32 Color32::sPaleVioletRed = Color32(219, 112, 147, 255);
INLINE constexpr Color32 Color32::sPapayaWhip = Color32(255, 239, 213, 255);
INLINE constexpr Color32 Color32::sPeachPuff = Color32(255, 218, 185, 255);
INLINE constexpr Color32 Color32::sPeru = Color32(205, 133, 63, 255);
INLINE constexpr Color32 Color32::sPink = Color32(255, 192, 203, 255);
INLINE constexpr Color32 Color32::sPlum = Color32(221, 160, 221, 255);
INLINE constexpr Color32 Color32::sPowderBlue = Color32(176, 224, 230, 255);
INLINE constexpr Color32 Color32::sPurple = Color32(160, 32, 240, 255);
INLINE constexpr Color32 Color32::sRebeccaPurple = Color32(102, 51, 153, 255);
INLINE constexpr Color32 Color32::sRed = Color32(255, 0, 0, 255);
INLINE constexpr Color32 Color32::sRosyBrown = Color32(188, 143, 143, 255);
INLINE constexpr Color32 Color32::sRoyalBlue = Color32(65, 105, 225, 255);
INLINE constexpr Color32 Color32::sSaddleBrown = Color32(139, 69, 19, 255);
INLINE constexpr Color32 Color32::sSalmon = Color32(250, 128, 114, 255);
INLINE constexpr Color32 Color32::sSandyBrown = Color32(244, 164, 96, 255);
INLINE constexpr Color32 Color32::sSeaGreen = Color32(46, 139, 87, 255);
INLINE constexpr Color32 Color32::sSeashell = Color32(255, 245, 238, 255);
INLINE constexpr Color32 Color32::sSienna = Color32(160, 82, 45, 255);
INLINE constexpr Color32 Color32::sSilver = Color32(192, 192, 192, 255);
INLINE constexpr Color32 Color32::sSkyBlue = Color32(135, 206, 235, 255);
INLINE constexpr Color32 Color32::sSlateBlue = Color32(106, 90, 205, 255);
INLINE constexpr Color32 Color32::sSlateGray = Color32(112, 128, 144, 255);
INLINE constexpr Color32 Color32::sSnow = Color32(255, 250, 250, 255);
INLINE constexpr Color32 Color32::sSpringGreen = Color32(0, 255, 127, 255);
INLINE constexpr Color32 Color32::sSteelBlue = Color32(70, 130, 180, 255);
INLINE constexpr Color32 Color32::sTan = Color32(210, 180, 140, 255);
INLINE constexpr Color32 Color32::sTeal = Color32(0, 128, 128, 255);
INLINE constexpr Color32 Color32::sThistle = Color32(216, 191, 216, 255);
INLINE constexpr Color32 Color32::sTomato = Color32(255, 99, 71, 255);
INLINE constexpr Color32 Color32::sTurquoise = Color32(64, 224, 208, 255);
INLINE constexpr Color32 Color32::sViolet = Color32(238, 130, 238, 255);
INLINE constexpr Color32 Color32::sWebGray = Color32(128, 128, 128, 255);
INLINE constexpr Color32 Color32::sWebGreen = Color32(0, 128, 0, 255);
INLINE constexpr Color32 Color32::sWebMaroon = Color32(128, 0, 0, 255);
INLINE constexpr Color32 Color32::sWebPurple = Color32(128, 0, 128, 255);
INLINE constexpr Color32 Color32::sWheat = Color32(245, 222, 179, 255);
INLINE constexpr Color32 Color32::sWhite = Color32(255, 255, 255, 255);
INLINE constexpr Color32 Color32::sWhiteSmoke = Color32(245, 245, 245, 255);
INLINE constexpr Color32 Color32::sYellow = Color32(255, 255, 0, 255);
INLINE constexpr Color32 Color32::sYellowGreen = Color32(154, 205, 50, 255);

} // namespace ho
