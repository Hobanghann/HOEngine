#include "Vector2.h"

#include "Vector3.h"
#include "Vector4.h"

namespace ho
{
Vector2::Vector2(const Vector3& v)
  : X(v.X)
  , Y(v.Y)
{
}

Vector2::Vector2(const Vector4& v)
  : X(v.X)
  , Y(v.Y)
{
}

Vector3 Vector2::ToHomogeneous() const
{
    return Vector3(X, Y, 1.0f);
}

std::string Vector2::ToString() const
{
    char buf[100];
    snprintf(buf, sizeof(buf), "(%.3f, %.3f)", X, Y);
    return buf;
}
} // namespace ho