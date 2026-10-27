#include "Matrix2x2.h"

#include <string>

#include "Matrix3x3.h"
#include "Matrix4x4.h"

namespace ho
{
Matrix2x2::Matrix2x2(const Matrix3x3& mat)
  : Row0(mat.Row0)
  , Row1(mat.Row1)
{
}

Matrix2x2::Matrix2x2(const Matrix4x4& mat)
  : Row0(mat.Row0)
  , Row1(mat.Row1)
{
}

std::string Matrix2x2::ToString() const
{
    char res[200];
    std::snprintf(res, sizeof(res), "| %.3f , %.3f |\n| %.3f , %.3f |", Row0.X, Row0.Y, Row1.X, Row1.Y);

    return res;
}
} // namespace ho