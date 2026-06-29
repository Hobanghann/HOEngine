#include "Projection.h"

namespace ho
{
Vector4 Projection::Project(const Vector4& v, float ndcNear, float ndcFar) const
{
    return GetMatrix(ndcNear, ndcFar) * v;
}

std::string Projection::ToString() const
{
    char buf[200];
    std::snprintf(buf,
                  sizeof(buf),
                  "Projection(Type: %s, Fov/ViewHeight: %.6f rad, aspectRatio: %.6f, "
                  "Near: %.6f, Far: %.6f)",
                  mType == eProjectionType::Orthographic ? "Perspective" : "Orthographic",
                  static_cast<float>(mFov),
                  static_cast<float>(GetAspectRatio()),
                  static_cast<float>(GetNearDistance()),
                  static_cast<float>(GetFarDistance()));
    return std::string(buf);
}
} // namespace ho