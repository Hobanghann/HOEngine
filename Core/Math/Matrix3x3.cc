#include "Matrix3x3.h"

#include "MathFuncs.h"
#include "Quaternion.h"

namespace ho
{
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
