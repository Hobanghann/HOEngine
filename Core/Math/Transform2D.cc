#include "Transform2D.h"

namespace ho
{
std::string Transform2D::ToString() const
{
    char buf[200];
    std::snprintf(buf,
                  sizeof(buf),
                  "Transform2D Position: %s, Basis : %s",
                  GetPosition().ToString().c_str(),
                  GetBasis().ToString().c_str());
    return std::string(buf);
}

} // namespace ho