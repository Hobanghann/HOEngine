#include "Rect.h"

namespace ho
{
std::string Rect::ToString() const
{
    char buf[100];
    snprintf(buf, sizeof(buf), "X: %.3f, Y: %.3f, Width: %.3f, Height: %.3f", X, Y, Width, Height);
    return buf;
}
} // namespace ho