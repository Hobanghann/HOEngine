#include "Transform2D.h"

namespace ho
{
Transform2D Transform2D::FromAngle(float angle)
{
    return Transform2D(Basis2D::FromAngle(angle), Vector2::sZero);
}

std::string Transform2D::ToString() const
{
    char buf[200];
    std::snprintf(buf,
                  sizeof(buf),
                  "Transform2D Origin: %s, Basis : %s",
                  GetOrigin().ToString().c_str(),
                  GetBasis().ToString().c_str());
    return std::string(buf);
}

} // namespace ho