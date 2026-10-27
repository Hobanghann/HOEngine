#include "Matrix3x3.h"

#include "Matrix2x2.h"
#include "Matrix4x4.h"

namespace ho
{
Matrix3x3::Matrix3x3(const Matrix2x2& mat)
  : Row0(mat.Row0)
  , Row1(mat.Row1)
  , Row2(Vector3::sUnitZ)
{
}

Matrix3x3::Matrix3x3(const Matrix4x4& mat)
  : Row0(mat.Row0)
  , Row1(mat.Row1)
  , Row2(mat.Row2)
{
}

std::string Matrix3x3::ToString() const
{
    char res[300];
    std::snprintf(res,
                  sizeof(res),
                  "| %.3f , %.3f , %.3f |\n| %.3f , %.3f , %.3f |\n| %.3f , %.3f , %.3f |",
                  Row0.X,
                  Row0.Y,
                  Row0.Z,
                  Row1.X,
                  Row1.Y,
                  Row1.Z,
                  Row2.X,
                  Row2.Y,
                  Row2.Z);

    return res;
}

} // namespace ho
