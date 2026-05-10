#include "Projection.h"

namespace ho
{
Vector4 Projection::Project(const Vector4& v) const
{
    return Vector4(
        mMatrix.Data[0][0] * v.X, mMatrix.Data[1][1] * v.Y, mMatrix.Data[2][2] * v.Z + mMatrix.Data[2][3] * v.W, -v.Z);
}

std::string Projection::ToString() const
{
    char buf[200];
    std::snprintf(buf,
                  sizeof(buf),
                  "Projection(Fov/ViewHeight: %.6f rad, aspectRatio: %.6f, "
                  "Near: %.6f, Far: %.6f)",
                  static_cast<float>(mFov),
                  static_cast<float>(GetAspectRatio()),
                  static_cast<float>(GetNearDistance()),
                  static_cast<float>(GetFarDistance()));
    return std::string(buf);
}
} // namespace ho