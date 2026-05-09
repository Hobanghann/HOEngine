#pragma once

#include <string>

#include "Matrix4x4.h"
#include "Transform3D.h"

namespace ho
{
class Projection final
{
  public:
    [[nodiscard]] FORCE_INLINE static Projection CreateOrthographic(float viewWidth,
                                                                    float viewHeight,
                                                                    float near,
                                                                    float far);
    [[nodiscard]] FORCE_INLINE static Projection CreateOrthographic(const Matrix4x4& m);
    [[nodiscard]] FORCE_INLINE static Projection CreatePerspective(
        float fov, float viewWidth, float viewHeight, float near, float far);
    [[nodiscard]] FORCE_INLINE static Projection CreatePerspective(const Matrix4x4& m);

    FORCE_INLINE constexpr Projection();
    FORCE_INLINE constexpr Projection(const Projection&) = default;
    FORCE_INLINE constexpr Projection& operator=(const Projection& rhs);
    ~Projection() = default;

    [[nodiscard]] FORCE_INLINE constexpr Matrix4x4 operator*(const Transform3D& t) const;

    [[nodiscard]] FORCE_INLINE constexpr float GetFOV() const;
    [[nodiscard]] FORCE_INLINE constexpr float GetAspectRatio() const;
    [[nodiscard]] FORCE_INLINE constexpr float GetNearDistance() const;
    [[nodiscard]] FORCE_INLINE constexpr float GetFarDistance() const;
    [[nodiscard]] FORCE_INLINE constexpr Matrix4x4 GetMatrix() const;

    FORCE_INLINE constexpr void SetFOV(float fov);
    FORCE_INLINE constexpr void SetAspectRatio(float viewWidth, float viewHeight);
    FORCE_INLINE void SetNearDistance(float near);
    FORCE_INLINE void SetFarDistance(float far);

    [[nodiscard]] constexpr bool operator==(const Projection& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Projection& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Projection& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Projection& rhs) const;

    [[nodiscard]] Vector4 Project(const Vector4& v) const;

    [[nodiscard]] std::string ToString() const;

  private:
    // Store projection parameters (fov, AspectRatio, Near, Far) directly to
    // avoid catastrophic cancellation when reconstructing them from the
    // projection Matrix. This ensures numerical stability and eliminates
    // precision loss in inverse computations.
    union
    {
        float mFov;
        float mViewHeight;
    };

    float mAspectRatio;
    float mNear;
    float mFar;

    bool mbOrthographic;

    Matrix4x4 mMatrix;
};

Projection Projection::CreateOrthographic(float viewWidth, float viewHeight, float near, float far)
{
    Projection proj;
    proj.mbOrthographic = true;
    proj.mViewHeight = viewHeight;
    proj.mAspectRatio = viewWidth / viewHeight;
    proj.mNear = near;
    proj.mFar = far;

    float range = far - near;

    if (math::IsEqualApprox(range, 0.0f))
    {
        range = math::FLOAT_MIN;
    }
    const float invRange = 1.0f / range;

    proj.mMatrix = Matrix4x4({2.0f / viewWidth, 0.0f, 0.0f, 0.0f},
                             {0.0f, 2.0f / viewHeight, 0.0f, 0.0f},
                             {0.0f, 0.0f, -2.0f * invRange, 0.f},
                             {0.0f, 0.0f, -(far + near) * invRange, 1.0f});

    return proj;
}

Projection Projection::CreateOrthographic(const Matrix4x4& m)
{
    Projection proj;
    proj.mbOrthographic = true;
    proj.mMatrix = m;

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
        proj.mViewHeight = 0.0f;
        proj.mAspectRatio = 0.0f;
        proj.mNear = 0.0f;
        proj.mFar = 0.0f;
        return proj;
    }

    proj.mViewHeight = static_cast<float>(2.0 / m11);
    const double viewWidth = 2.0 / m00;
    proj.mAspectRatio = static_cast<float>(viewWidth / proj.mViewHeight);

    if (math::IsZeroApprox(static_cast<float>(m22)))
    {
        proj.mNear = 0.0f;
        proj.mFar = 0.0f;
    }
    else
    {
        proj.mNear = static_cast<float>((m23 + 1.0) / m22);
        proj.mFar = static_cast<float>((m23 - 1.0) / m22);
    }

    return proj;
}

Projection Projection::CreatePerspective(float fov, float viewWidth, float viewHeight, float near, float far)
{
    Projection proj;
    proj.mbOrthographic = false;
    proj.mFov = fov;
    proj.mAspectRatio = viewWidth / viewHeight;
    proj.mNear = near;
    proj.mFar = far;
    const float fovFactor = 1.0f / math::Tan(proj.mFov * 0.5f);
    float nearMinusFar = (proj.mNear - proj.mFar);
    if (math::IsEqualApprox(nearMinusFar, 0.0f))
    {
        nearMinusFar = math::FLOAT_MIN;
    }
    const float invNearMinusFar = 1.0f / nearMinusFar;
    proj.mMatrix = Matrix4x4({fovFactor / proj.mAspectRatio, 0.0f, 0.0f, 0.0f},
                             {0.0f, fovFactor, 0.0f, 0.0f},
                             {0.0f, 0.0f, (proj.mNear + proj.mFar) * invNearMinusFar, -1.0f},
                             {0.0f, 0.0f, (2.0f * proj.mNear * proj.mFar) * invNearMinusFar, 0.0f});
    return proj;
}

Projection Projection::CreatePerspective(const Matrix4x4& m)
{
    Projection proj;
    proj.mbOrthographic = false;
    proj.mMatrix = m;

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
        proj.mFov = 0.0f;
        proj.mAspectRatio = 0.0f;
        proj.mNear = 0.0f;
        proj.mFar = 0.0f;
        return proj;
    }

    const double fovFactor = m11;
    const double fovRad = 2.0 * std::atan(1.0 / fovFactor);
    proj.mFov = static_cast<float>(fovRad);

    proj.mAspectRatio = static_cast<float>(fovFactor / m00);

    const double nearDist = m23 / (m22 - 1.0);
    const double farDist = m23 / (m22 + 1.0);

    proj.mNear = static_cast<float>(nearDist);
    proj.mFar = static_cast<float>(farDist);
    return proj;
}

constexpr Projection::Projection()
  : mMatrix(Matrix4x4())
  , mFov(0.0f)
  , mAspectRatio(0.0f)
  , mNear(0.0f)
  , mFar(0.0f)
  , mbOrthographic(false)
{
}

constexpr Projection& Projection::operator=(const Projection& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    mMatrix = rhs.mMatrix;
    mFov = rhs.mFov;
    mAspectRatio = rhs.mAspectRatio;
    mNear = rhs.mNear;
    mFar = rhs.mFar;
    mbOrthographic = rhs.mbOrthographic;
    return *this;
}

constexpr Matrix4x4 Projection::operator*(const Transform3D& t) const
{
    return mMatrix * t.Matrix;
}

constexpr float Projection::GetFOV() const
{
    if (mbOrthographic)
    {
        return 0.0f;
    }
    return mFov;
}

constexpr float Projection::GetAspectRatio() const
{
    return mAspectRatio;
}

constexpr float Projection::GetFarDistance() const
{
    return mFar;
}

constexpr float Projection::GetNearDistance() const
{
    return mNear;
}

constexpr Matrix4x4 Projection::GetMatrix() const
{
    return mMatrix;
}

constexpr void Projection::SetFOV(float fov)
{
    if (!mbOrthographic)
    {
        mFov = fov;
        const float fovFactor = 1.0f / math::Tan(mFov * 0.5f);
        const float aspect = GetAspectRatio();
        mMatrix.Data[0][0] = fovFactor / aspect;
        mMatrix.Data[1][1] = fovFactor;
    }
}

constexpr void Projection::SetAspectRatio(float viewWidth, float viewHeight)
{
    mAspectRatio = viewWidth / viewHeight;

    mMatrix.Data[0][0] = mMatrix.Data[1][1] / mAspectRatio;
}

void Projection::SetNearDistance(float near)
{
    mNear = near;
    const float far = GetFarDistance();

    const double nearDist = static_cast<double>(mNear);
    const double farDist = static_cast<double>(far);

    if (mbOrthographic)
    {
        double range = farDist - nearDist;
        if (range == 0.0)
        {
            range = 1e-10;
        }
        const double invRange = 1.0 / range;

        mMatrix.Data[2][2] = static_cast<float>(-2.0 * invRange);
        mMatrix.Data[2][3] = static_cast<float>(-(farDist + nearDist) * invRange);
    }
    else
    {
        const double invNearMinusFar = 1.0 / (nearDist - farDist);

        mMatrix.Data[2][2] = static_cast<float>((nearDist + farDist) * invNearMinusFar);
        mMatrix.Data[2][3] = static_cast<float>((2.0 * nearDist * farDist) * invNearMinusFar);
    }
}

void Projection::SetFarDistance(float far)
{
    mFar = far;
    const float n = GetNearDistance();

    const double nearDist = static_cast<double>(n);
    const double farDist = static_cast<double>(mFar);

    if (mbOrthographic)
    {
        double range = farDist - nearDist;
        if (range == 0.0)
        {
            range = 1e-10;
        }
        const double invRange = 1.0 / range;

        mMatrix.Data[2][2] = static_cast<float>(-2.0 * invRange);
        mMatrix.Data[2][3] = static_cast<float>(-(farDist + nearDist) * invRange);
    }
    else
    {
        const double invNearMinusFar = 1.0 / (nearDist - farDist);

        mMatrix.Data[2][2] = static_cast<float>((nearDist + farDist) * invNearMinusFar);
        mMatrix.Data[2][3] = static_cast<float>((2.0 * nearDist * farDist) * invNearMinusFar);
    }
}

constexpr bool Projection::operator==(const Projection& rhs) const
{
    if (mbOrthographic != rhs.mbOrthographic)
    {
        return false;
    }
    return mMatrix == rhs.mMatrix && mFov == rhs.mFov && mAspectRatio == rhs.mAspectRatio && mNear == rhs.mNear &&
           mFar == rhs.mFar;
}

constexpr bool Projection::operator!=(const Projection& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Projection::IsEqualApprox(const Projection& rhs) const
{
    if (mbOrthographic != rhs.mbOrthographic)
    {
        return false;
    }
    return mMatrix.IsEqualApprox(rhs.mMatrix) && math::IsEqualApprox(mFov, rhs.mFov) &&
           math::IsEqualApprox(mAspectRatio, rhs.mAspectRatio) && math::IsEqualApprox(mNear, rhs.mNear) &&
           math::IsEqualApprox(mFar, rhs.mFar);
}

constexpr bool Projection::IsNotEqualApprox(const Projection& rhs) const
{
    return !IsEqualApprox(rhs);
}
} // namespace ho