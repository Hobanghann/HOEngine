#include "color32.h"

#include "color128.h"

namespace ho
{

Color32::Color32(const Color128& c)
  : R(static_cast<uint8_t>(math::Clamp(c.R, 0.0f, 1.0f) * 255.0f))
  , G(static_cast<uint8_t>(math::Clamp(c.G, 0.0f, 1.0f) * 255.0f))
  , B(static_cast<uint8_t>(math::Clamp(c.B, 0.0f, 1.0f) * 255.0f))
  , A(static_cast<uint8_t>(math::Clamp(c.A, 0.0f, 1.0f) * 255.0f))
{
}

std::string Color32::ToString() const
{
    char buf[100];
    snprintf(buf, sizeof(buf), "(R: %d, G: %d, B: %d, A: %d)", R, G, B, A);
    return buf;
}

} // namespace ho
