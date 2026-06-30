#include "Basis2D.h"

#include "MathFuncs.h"

namespace ho
{
void Basis2D::Rotate(float angleRad)
{
    float sin = 0.0f;
    float cos = 0.0f;
    math::SinCos(&sin, &cos, angleRad);
    const Matrix2x2 rotationMat = Matrix2x2({cos, sin}, {-sin, cos});
    Matrix = rotationMat * Matrix;
}

Basis2D Basis2D::Rotated(float angleRad) const
{
    Basis2D copy = *this;
    copy.Rotate(angleRad);
    return copy;
}

void Basis2D::RotateLocal(float angleRad)
{
    float sin = 0.0f;
    float cos = 0.0f;
    math::SinCos(&sin, &cos, angleRad);
    const Matrix2x2 rotationMat = Matrix2x2({cos, sin}, {-sin, cos});
    Matrix = Matrix * rotationMat;
}

Basis2D Basis2D::RotatedLocal(float angleRad) const
{
    Basis2D copy = *this;
    copy.RotateLocal(angleRad);
    return copy;
}

void Basis2D::LookAt(const Vector2& at)
{
    Vector2 up = at;
    if (up.IsEqualApprox(Vector2::sZero))
    {
        return;
    }
    const float magX = GetScaleX();
    const float magY = GetScaleY();
    up.Normalize();
    const Vector2 right = Vector2(up.Y, -up.X);
    Matrix.SetCol0(magX * right);
    Matrix.SetCol1(magY * up);
}

Basis2D Basis2D::LookedAt(const Vector2& at) const
{
    Basis2D copy = *this;
    copy.LookAt(at);
    return copy;
}

std::string Basis2D::ToString() const
{
    char buf[160];
    std::snprintf(
        buf, sizeof(buf), "Basis2D Right: %s, Up: %s", GetRight().ToString().c_str(), GetUp().ToString().c_str());
    return std::string(buf);
}

} // namespace ho