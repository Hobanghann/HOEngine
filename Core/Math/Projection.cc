#include "projection.h"

namespace ho
{
Vector4 Projection::Project(const Vector4& v) const
{
    return Vector4(
        Matrix.Data[0][0] * v.X, Matrix.Data[1][1] * v.Y, Matrix.Data[2][2] * v.Z + Matrix.Data[2][3] * v.W, -v.Z);
}

std::string Projection::ToString() const
{
    char buf[200];
    std::snprintf(buf,
                  sizeof(buf),
                  "Projection(Fov/ViewHeight: %.6f rad, aspectRatio: %.6f, "
                  "Near: %.6f, Far: %.6f)",
                  static_cast<float>(Fov),
                  static_cast<float>(GetAspectRatio()),
                  static_cast<float>(GetNearDistance()),
                  static_cast<float>(GetFarDistance()));
    return std::string(buf);
}
} // namespace ho