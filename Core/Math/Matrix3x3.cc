#include "Matrix3x3.h"

#include "MathFuncs.h"
#include "Quaternion.h"

namespace ho
{
Matrix3x3 Matrix3x3::FromAxisAngle(const Vector3& axis, float angle)
{
    const Vector3 n = axis.Normalized();
    const float x = n.X;
    const float y = n.Y;
    const float z = n.Z;
    float sin = 0.0f;
    float cos = 0.0f;
    math::SinCos(&sin, &cos, angle);
    const float t = 1.0f - cos;

    Matrix3x3 m;
    m.Data[0][0] = cos + t * x * x;
    m.Data[0][1] = t * x * y - sin * z;
    m.Data[0][2] = t * x * z + sin * y;

    m.Data[1][0] = t * x * y + sin * z;
    m.Data[1][1] = cos + t * y * y;
    m.Data[1][2] = t * y * z - sin * x;

    m.Data[2][0] = t * x * z - sin * y;
    m.Data[2][1] = t * y * z + sin * x;
    m.Data[2][2] = cos + t * z * z;

    return m;
}

Matrix3x3 Matrix3x3::FromEuler(float x, float y, float z, math::eEulerOrder order)
{
    float sinX = 0.0f;
    float cosX = 0.0f;
    math::SinCos(&sinX, &cosX, x);
    const Matrix3x3 rotationMatX =
        Matrix3x3(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, cosX, sinX), Vector3(0.0f, -sinX, cosX));
    float sinY = 0.0f;
    float cosY = 0.0f;
    math::SinCos(&sinY, &cosY, y);
    const Matrix3x3 rotationMatY =
        Matrix3x3(Vector3(cosY, 0.0f, -sinY), Vector3(0.0f, 1.0f, 0.0f), Vector3(sinY, 0.0f, cosY));
    float sinZ = 0.0f;
    float cosZ = 0.0f;
    math::SinCos(&sinZ, &cosZ, z);
    const Matrix3x3 rotationMatZ =
        Matrix3x3(Vector3(cosZ, sinZ, 0.0f), Vector3(-sinZ, cosZ, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    Matrix3x3 m;
    switch (order)
    {
        case math::eEulerOrder::XYZ:
            m = rotationMatZ * rotationMatY * rotationMatX;
            break;
        case math::eEulerOrder::XZY:
            m = rotationMatY * rotationMatZ * rotationMatX;
            break;
        case math::eEulerOrder::YXZ:
            m = rotationMatY * rotationMatX * rotationMatZ;
            break;
        case math::eEulerOrder::YZX:
            m = rotationMatX * rotationMatZ * rotationMatY;
            break;
        case math::eEulerOrder::ZXY:
            m = rotationMatZ * rotationMatX * rotationMatY;
            break;
        case math::eEulerOrder::ZYX:
            m = rotationMatX * rotationMatY * rotationMatZ;
            break;
        default:
            m = Matrix3x3();
            break;
    }
    return m;
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
