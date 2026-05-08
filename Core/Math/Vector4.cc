#include "Vector4.h"

#include "Color128.h"
#include "Quaternion.h"
#include "Vector2.h"
#include "Vector3.h"

namespace ho
{
Vector4::Vector4(const Vector2& v)
  : X(v.X)
  , Y(v.Y)
  , Z(0.0f)
  , W(0.0f)
{
}

Vector4::Vector4(const Vector3& v)
  : X(v.X)
  , Y(v.Y)
  , Z(v.Z)
  , W(0.0f)
{
}

Vector4::Vector4(const Quaternion& q)
  : X(q.X)
  , Y(q.Y)
  , Z(q.Z)
  , W(q.W)
{
}

Vector4::Vector4(const Color128& c)
  : X(c.R)
  , Y(c.G)
  , Z(c.B)
  , W(c.A)
{
}

Vector3 Vector4::ToCartesian() const
{
    const float invW = 1.0f / W;
    return Vector3(invW * X, invW * Y, invW * Z);
}

std::string Vector4::ToString() const
{
    char buf[100];
    snprintf(buf, sizeof(buf), "(%.3f, %.3f, %.3f, %.3f)", X, Y, Z, W);
    return buf;
}
} // namespace ho
