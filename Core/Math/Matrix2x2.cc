#include "Matrix2x2.h"

#include <string>

namespace ho
{
std::string Matrix2x2::ToString() const
{
    char res[200];
    std::snprintf(res, sizeof(res), "| %.3f , %.3f |\n| %.3f , %.3f |", Row0.X, Row0.Y, Row1.X, Row1.Y);

    return res;
}
} // namespace ho