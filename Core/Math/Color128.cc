#include "Color128.h"

#include "Color32.h"
#include "Vector4.h"

namespace ho
{
Color128::Color128(const Color32& c)
  : R(static_cast<float>(c.R))
  , G(static_cast<float>(c.G))
  , B(static_cast<float>(c.B))
  , A(static_cast<float>(c.A))
{
    const float invMax = 1.0f / 255.0f;
    *this *= invMax;
}

Color128::Color128(const Vector4& v)
  : R(v.X)
  , G(v.Y)
  , B(v.Z)
  , A(v.W)
{
}

std::string Color128::ToString() const
{
    char buf[100];
    snprintf(buf, sizeof(buf), "(R: %.3f, G: %.3f, B: %.3f, A: %.3f)", R, G, B, A);
    return buf;
}
} // namespace ho