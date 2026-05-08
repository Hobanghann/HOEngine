#include "quaternion.h"

#include "matrix3x3.h"

namespace ho
{

Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angle)
{
    const float d = axis.Magnitude();
    HO_ASSERT(!math::IsZeroApprox(d), "Axis can't be zero vector.");

    float sin = 0.0f;
    float cos = 0.0f;
    math::SinCos(&sin, &cos, angle * 0.5f);
    sin /= d;

    return Quaternion(axis.X * sin, axis.Y * sin, axis.Z * sin, cos);
}

Quaternion Quaternion::FromEuler(float pitch, float yaw, float roll, math::eEulerOrder order)
{
    const Quaternion qx = FromAxisAngle(Vector3::sUnitX, pitch);
    const Quaternion qy = FromAxisAngle(Vector3::sUnitY, yaw);
    const Quaternion qz = FromAxisAngle(Vector3::sUnitZ, roll);

    Quaternion q;
    switch (order)
    {
        case math::eEulerOrder::XYZ:
            q = qx * qy * qz;
            break;
        case math::eEulerOrder::XZY:
            q = qx * qz * qy;
            break;
        case math::eEulerOrder::YXZ:
            q = qy * qx * qz;
            break;
        case math::eEulerOrder::YZX:
            q = qy * qz * qx;
            break;
        case math::eEulerOrder::ZXY:
            q = qz * qx * qy;
            break;
        case math::eEulerOrder::ZYX:
            q = qz * qy * qx;
            break;
        default:
            HO_ASSERT(false, "Invalid euler order.");
            break;
    }
    return q;
}

// This function for converting a matrix3x3 to a quaternion is adapted from
// the get_quaternion() method in the Godot Engine's Basis class.
// Source: https://github.com/godotengine/godot/blob/master/core/math/basis.cpp
Quaternion Quaternion::FromMatrix(const Matrix3x3& m)
{
    HO_ASSERT(m.IsOrthonormal(), "The matrix must be orthonormal to be converted into a quaternion.");
    /* Allow getting a quaternion from an unnormalized transform */
    float trace = m.Trace();
    float temp[4];

    if (trace > 0.0f)
    {
        float s = math::Sqrt(trace + 1.0f);
        temp[3] = (s * 0.5f);
        s = 0.5f / s;

        temp[0] = ((m.Data[2][1] - m.Data[1][2]) * s);
        temp[1] = ((m.Data[0][2] - m.Data[2][0]) * s);
        temp[2] = ((m.Data[1][0] - m.Data[0][1]) * s);
    }
    else
    {
        int32_t i =
            m.Data[0][0] < m.Data[1][1] ? (m.Data[1][1] < m.Data[2][2] ? 2 : 1) : (m.Data[0][0] < m.Data[2][2] ? 2 : 0);
        int32_t j = (i + 1) % 3;
        int32_t k = (i + 2) % 3;

        float s = math::Sqrt(m.Data[i][i] - m.Data[j][j] - m.Data[k][k] + 1.0f);
        temp[i] = s * 0.5f;
        s = 0.5f / s;

        temp[3] = (m.Data[k][j] - m.Data[j][k]) * s;
        temp[j] = (m.Data[j][i] + m.Data[i][j]) * s;
        temp[k] = (m.Data[k][i] + m.Data[i][k]) * s;
    }

    return Quaternion(temp[0], temp[1], temp[2], temp[3]);
}

std::string Quaternion::ToString() const
{
    char buf[100];
    snprintf(buf, sizeof(buf), "%.3f + %.3f*i, + %.3f*j + %.3f*k", X, Y, Z, W);
    return buf;
}
} // namespace ho