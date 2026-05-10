#include "Vector4.h"

#include "Color128.h"
#include "Quaternion.h"

namespace ho
{
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

std::string Vector4::ToString() const
{
    char buf[100];
    snprintf(buf, sizeof(buf), "(%.3f, %.3f, %.3f, %.3f)", X, Y, Z, W);
    return buf;
}
} // namespace ho
