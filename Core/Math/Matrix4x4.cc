#include "Matrix4x4.h"

#include "Matrix3x3.h"
#include "Vector3.h"

namespace ho
{
Matrix3x3 Matrix4x4::ToMatrix3x3() const
{
    Matrix3x3 res;
    res.Row0 = Vector3(Row0.X, Row0.Y, Row0.Z);
    res.Row1 = Vector3(Row1.X, Row1.Y, Row1.Z);
    res.Row2 = Vector3(Row2.X, Row2.Y, Row2.Z);
    return res;
}

std::string Matrix4x4::ToString() const
{
    char res[300];
    std::snprintf(res,
                  sizeof(res),
                  "| %.3f , %.3f , %.3f, %.3f |\n| %.3f , %.3f , %.3f, %.3f |\n| "
                  "%.3f , %.3f , %.3f, %.3f |\n| %.3f , %.3f , %.3f, %.3f |",
                  static_cast<float>(Row0.X),
                  static_cast<float>(Row0.Y),
                  static_cast<float>(Row0.Z),
                  static_cast<float>(Row0.W),
                  static_cast<float>(Row1.X),
                  static_cast<float>(Row1.Y),
                  static_cast<float>(Row1.Z),
                  static_cast<float>(Row1.W),
                  static_cast<float>(Row2.X),
                  static_cast<float>(Row2.Y),
                  static_cast<float>(Row2.Z),
                  static_cast<float>(Row2.W),
                  static_cast<float>(Row3.X),
                  static_cast<float>(Row3.Y),
                  static_cast<float>(Row3.Z),
                  static_cast<float>(Row3.W));

    return res;
}

} // namespace ho