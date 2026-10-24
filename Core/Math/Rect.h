#pragma once

#include <string>

#include "Vector2.h"

namespace ho
{
struct Rect final
{
    constexpr Rect() = default;
    constexpr Rect(const Rect& rect);
    constexpr Rect(float x, float y, float width, float height);
    constexpr Rect(const Vector2& min, const Vector2& max);
    constexpr Rect& operator=(const Rect& rhs);
    ~Rect() = default;

    [[nodiscard]] constexpr Vector2 GetCenter() const;

    [[nodiscard]] constexpr bool operator==(const Rect& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Rect& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Rect& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Rect& rhs) const;

    [[nodiscard]] std::string ToString() const;

    float X = 0.0f;
    float Y = 0.0f;
    float Width = 0.0f;
    float Height = 0.0f;
};

constexpr Rect::Rect(const Rect& rect) = default;

constexpr Rect::Rect(float x, float y, float width, float height)
  : X(x)
  , Y(y)
  , Width(width)
  , Height(height)
{
}

constexpr Rect::Rect(const Vector2& min, const Vector2& max)
  : X(min.X)
  , Y(min.Y)
  , Width(max.X - min.X)
  , Height(max.Y - min.Y)
{
    HO_ASSERT(Width > 0.0f && Height > 0.0f, "max position must be greater than min position.");
}

constexpr Rect& Rect::operator=(const Rect& rhs) = default;

constexpr Vector2 Rect::GetCenter() const
{
    return Vector2(X, Y) + Vector2(Width, Height) * 0.5f;
}

constexpr bool Rect::operator==(const Rect& rhs) const
{
    return (X == rhs.X && Y == rhs.Y && Width == rhs.Width && Height == rhs.Height);
}

constexpr bool Rect::operator!=(const Rect& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Rect::IsEqualApprox(const Rect& rhs) const
{
    return (math::IsEqualApprox(X, rhs.X) && math::IsEqualApprox(Y, rhs.Y) && math::IsEqualApprox(Width, rhs.Width) &&
            math::IsEqualApprox(Height, rhs.Height));
}

constexpr bool Rect::IsNotEqualApprox(const Rect& rhs) const
{
    return !IsEqualApprox(rhs);
}

} // namespace ho