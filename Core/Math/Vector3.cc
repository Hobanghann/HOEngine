#include "Vector3.h"

#include "Vector4.h"

namespace ho
{
Vector3::Vector3(const Vector4& v)
  : X(v.X)
  , Y(v.Y)
  , Z(v.Z)
{
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