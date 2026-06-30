#include "Matrix4x4.h"

#include "Matrix2x2.h"
#include "Matrix3x3.h"
#include "Vector3.h"

namespace ho
{
Matrix4x4::Matrix4x4(const Matrix2x2& mat)
  : Row0(mat.Row0)
  , Row1(mat.Row1)
  , Row2(Vector4::sUnitZ)
  , Row3(Vector4::sUnitW)
{
}

Matrix4x4::Matrix4x4(const Matrix3x3& mat)
  : Row0(mat.Row0)
  , Row1(mat.Row1)
  , Row2(mat.Row2)
  , Row3(Vector4::sUnitW)
{
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