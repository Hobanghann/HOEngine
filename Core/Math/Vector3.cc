#include "Vector3.h"

#include "Vector2.h"
#include "Vector4.h"

namespace ho
{
Vector3::Vector3(const Vector2& v)
  : X(v.X)
  , Y(v.Y)
  , Z(0.0f)
{
}

Vector3::Vector3(const Vector4& v)
  : X(v.X)
  , Y(v.Y)
  , Z(v.Z)
{
}

Vector2 Vector3::ToCartesian() const
{
    const float invZ = 1.0f / Z;
    return Vector2(X * invZ, Y * invZ);
}

Vector4 Vector3::ToHomogeneous() const
{
    return Vector4(X, Y, Z, 1.0f);
}

std::string Vector3::ToString() const
{
    char buf[100];
    snprintf(buf, sizeof(buf), "(%.3f, %.3f, %.3f)", X, Y, Z);
    return buf;
}
} // namespace ho