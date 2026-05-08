#pragma once

#include <string>

#include "Matrix4x4.h"
#include "Transform3D.h"

namespace ho
{
struct Projection final
{
    [[nodiscard]] FORCE_INLINE static Projection CreateOrthographic(float viewWidth,
                                                                    float viewHeight,
                                                                    float near,
                                                                    float far);
    [[nodiscard]] FORCE_INLINE static Projection CreateOrthographic(const Matrix4x4& m);
    [[nodiscard]] FORCE_INLINE static Projection CreatePerspective(
        float fov, float viewWidth, float viewHeight, float near, float far);
    [[nodiscard]] FORCE_INLINE static Projection CreatePerspective(const Matrix4x4& m);

    FORCE_INLINE Projection();
    FORCE_INLINE Projection(const Projection&) = default;
    FORCE_INLINE Projection& operator=(const Projection& rhs);
    ~Projection() = default;

    [[nodiscard]] FORCE_INLINE float GetFOV() const;
    [[nodiscard]] FORCE_INLINE float GetAspectRatio() const;
    [[nodiscard]] FORCE_INLINE float GetNearDistance() const;
    [[nodiscard]] FORCE_INLINE float GetFarDistance() const;

    FORCE_INLINE void SetFOV(float fov);
    FORCE_INLINE void SetAspectRatio(float viewWidth, float viewHeight);
    FORCE_INLINE void SetNearDistance(float near);
    FORCE_INLINE void SetFarDistance(float far);

    [[nodiscard]] constexpr bool operator==(const Projection& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Projection& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Projection& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Projection& rhs) const;

    [[nodiscard]] Vector4 Project(const Vector4& v) const;

    [[nodiscard]] std::string ToString() const;

    Matrix4x4 Matrix;

  private:
    // Store projection parameters (fov, AspectRatio, Near, Far) directly to
    // avoid catastrophic cancellation when reconstructing them from the
    // projection Matrix. This ensures numerical stability and eliminates
    // precision loss in inverse computations.
    union
    {
        float Fov;
        float ViewHeight;
    };

    float AspectRatio;
    float Near;
    float Far;

    bool bOrthographic;
};

Projection Projection::CreateOrthographic(float viewWidth, float viewHeight, float near, float far)
{
    Projection proj;
    proj.bOrthographic = true;
    proj.ViewHeight = viewHeight;
    proj.AspectRatio = viewWidth / viewHeight;
    proj.Near = near;
    proj.Far = far;

    float range = far - near;

    if (math::IsEqualApprox(range, 0.0f))
    {
        range = math::FLOAT_MIN;
    }
    float invRange = 1.0f / range;

    proj.Matrix = Matrix4x4({2.0f / viewWidth, 0.0f, 0.0f, 0.0f},
                            {0.0f, 2.0f / viewHeight, 0.0f, 0.0f},
                            {0.0f, 0.0f, -2.0f * invRange, 0.f},
                            {0.0f, 0.0f, -(far + near) * invRange, 1.0f});

    return proj;
}

Projection Projection::CreateOrthographic(const Matrix4x4& m)
{
    Projection proj;
    proj.bOrthographic = true;
    proj.Matrix = m;

    // Extract projection parameters from the Matrix.
    // Use double precision internally to minimize cancellation and rounding
    // errors.
    const double m00 = static_cast<double>(m.Data[0][0]);
    const double m11 = static_cast<double>(m.Data[1][1]);
    const double m22 = static_cast<double>(m.Data[2][2]);
    const double m23 = static_cast<double>(m.Data[2][3]);

    // If Matrix is invalid or uninitialized, fallback to zero projection.
    if (math::IsZeroApprox(static_cast<float>(m11)) || math::IsZeroApprox(static_cast<float>(m00)))
    {
        proj.ViewHeight = 0.0f;
        proj.AspectRatio = 0.0f;
        proj.Near = 0.0f;
        proj.Far = 0.0f;
        return proj;
    }

    proj.ViewHeight = static_cast<float>(2.0 / m11);
    const double viewWidth = 2.0 / m00;
    proj.AspectRatio = static_cast<float>(viewWidth / proj.ViewHeight);

    if (math::IsZeroApprox(static_cast<float>(m22)))
    {
        proj.Near = 0.0f;
        proj.Far = 0.0f;
    }
    else
    {
        proj.Near = static_cast<float>((m23 + 1.0) / m22);
        proj.Far = static_cast<float>((m23 - 1.0) / m22);
    }

    return proj;
}

Projection Projection::CreatePerspective(float fov, float viewWidth, float viewHeight, float near, float far)
{
    Projection proj;
    proj.bOrthographic = false;
    proj.Fov = fov;
    proj.AspectRatio = viewWidth / viewHeight;
    proj.Near = near;
    proj.Far = far;
    const float fovFactor = 1.0f / math::Tan(proj.Fov * 0.5f);
    float nearMinusFar = (proj.Near - proj.Far);
    if (math::IsEqualApprox(nearMinusFar, 0.0f))
    {
        nearMinusFar = math::FLOAT_MIN;
    }
    const float invNearMinusFar = 1.0f / nearMinusFar;
    proj.Matrix = Matrix4x4({fovFactor / proj.AspectRatio, 0.0f, 0.0f, 0.0f},
                            {0.0f, fovFactor, 0.0f, 0.0f},
                            {0.0f, 0.0f, (proj.Near + proj.Far) * invNearMinusFar, -1.0f},
                            {0.0f, 0.0f, (2.0f * proj.Near * proj.Far) * invNearMinusFar, 0.0f});
    return proj;
}

Projection Projection::CreatePerspective(const Matrix4x4& m)
{
    Projection proj;
    proj.bOrthographic = false;
    proj.Matrix = m;

    // Extract projection parameters from the Matrix.
    // Use double precision internally to minimize cancellation and rounding
    // errors.
    const double m00 = static_cast<double>(m.Data[0][0]);
    const double m11 = static_cast<double>(m.Data[1][1]);
    const double m22 = static_cast<double>(m.Data[2][2]);
    const double m23 = static_cast<double>(m.Data[2][3]);

    // If Matrix is invalid or uninitialized, fallback to zero projection.
    if (math::IsZeroApprox(static_cast<float>(m00)))
    {
        proj.Fov = 0.0f;
        proj.AspectRatio = 0.0f;
        proj.Near = 0.0f;
        proj.Far = 0.0f;
        return proj;
    }

    const double fovFactor = m11;
    const double fovRad = 2.0 * std::atan(1.0 / fovFactor);
    proj.Fov = static_cast<float>(fovRad);

    proj.AspectRatio = static_cast<float>(fovFactor / m00);

    const double nearDist = m23 / (m22 - 1.0);
    const double farDist = m23 / (m22 + 1.0);

    proj.Near = static_cast<float>(nearDist);
    proj.Far = static_cast<float>(farDist);
    return proj;
}

Projection::Projection()
  : Matrix(Matrix4x4())
  , Fov(0.0f)
  , AspectRatio(0.0f)
  , Near(0.0f)
  , Far(0.0f)
  , bOrthographic(false)
{
}

Projection& Projection::operator=(const Projection& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    Matrix = rhs.Matrix;
    Fov = rhs.Fov;
    AspectRatio = rhs.AspectRatio;
    Near = rhs.Near;
    Far = rhs.Far;
    bOrthographic = rhs.bOrthographic;
    return *this;
}

float Projection::GetFOV() const
{
    if (bOrthographic)
    {
        return 0.0f;
    }
    return Fov;
}

float Projection::GetAspectRatio() const
{
    return AspectRatio;
}

float Projection::GetFarDistance() const
{
    return Far;
}

float Projection::GetNearDistance() const
{
    return Near;
}

void Projection::SetFOV(float fov)
{
    if (!bOrthographic)
    {
        Fov = fov;
        const float fovFactor = 1.0f / math::Tan(Fov * 0.5f);
        const float aspect = GetAspectRatio();
        Matrix.Data[0][0] = fovFactor / aspect;
        Matrix.Data[1][1] = fovFactor;
    }
}

void Projection::SetAspectRatio(float viewWidth, float viewHeight)
{
    AspectRatio = viewWidth / viewHeight;

    Matrix.Data[0][0] = Matrix.Data[1][1] / AspectRatio;
}

void Projection::SetNearDistance(float near)
{
    Near = near;
    const float far = GetFarDistance();

    const double nearDist = static_cast<double>(Near);
    const double farDist = static_cast<double>(far);

    if (bOrthographic)
    {
        double range = farDist - nearDist;
        if (range == 0.0)
        {
            range = 1e-10;
        }
        const double invRange = 1.0 / range;

        Matrix.Data[2][2] = static_cast<float>(-2.0 * invRange);
        Matrix.Data[2][3] = static_cast<float>(-(farDist + nearDist) * invRange);
    }
    else
    {
        const double invNearMinusFar = 1.0 / (nearDist - farDist);

        Matrix.Data[2][2] = static_cast<float>((nearDist + farDist) * invNearMinusFar);
        Matrix.Data[2][3] = static_cast<float>((2.0 * nearDist * farDist) * invNearMinusFar);
    }
}

void Projection::SetFarDistance(float far)
{
    Far = far;
    const float n = GetNearDistance();

    const double nearDist = static_cast<double>(n);
    const double farDist = static_cast<double>(Far);

    if (bOrthographic)
    {
        double range = farDist - nearDist;
        if (range == 0.0)
        {
            range = 1e-10;
        }
        const double invRange = 1.0 / range;

        Matrix.Data[2][2] = static_cast<float>(-2.0 * invRange);
        Matrix.Data[2][3] = static_cast<float>(-(farDist + nearDist) * invRange);
    }
    else
    {
        const double invNearMinusFar = 1.0 / (nearDist - farDist);

        Matrix.Data[2][2] = static_cast<float>((nearDist + farDist) * invNearMinusFar);
        Matrix.Data[2][3] = static_cast<float>((2.0 * nearDist * farDist) * invNearMinusFar);
    }
}

constexpr bool Projection::operator==(const Projection& rhs) const
{
    if (bOrthographic != rhs.bOrthographic)
    {
        return false;
    }
    return Matrix == rhs.Matrix && Fov == rhs.Fov && AspectRatio == rhs.AspectRatio && Near == rhs.Near &&
           Far == rhs.Far;
}

constexpr bool Projection::operator!=(const Projection& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Projection::IsEqualApprox(const Projection& rhs) const
{
    if (bOrthographic != rhs.bOrthographic)
    {
        return false;
    }
    return Matrix.IsEqualApprox(rhs.Matrix) && math::IsEqualApprox(Fov, rhs.Fov) &&
           math::IsEqualApprox(AspectRatio, rhs.AspectRatio) && math::IsEqualApprox(Near, rhs.Near) &&
           math::IsEqualApprox(Far, rhs.Far);
}

constexpr bool Projection::IsNotEqualApprox(const Projection& rhs) const
{
    return !IsEqualApprox(rhs);
}

FORCE_INLINE constexpr Matrix4x4 operator*(const Projection& p, const Transform3D& t)
{
    return p.Matrix * t.Matrix;
}

} // namespace ho