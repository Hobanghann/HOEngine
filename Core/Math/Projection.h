#pragma once

#include <string>

#include "Matrix4x4.h"
#include "Transform3D.h"

namespace ho
{
enum class eProjectionType
{
    Orthographic = 0,
    Perspective
};

class Projection final
{
  public:
    [[nodiscard]] FORCE_INLINE static constexpr Projection CreateOrthographic(float viewHeight,
                                                                              float aspectRatio,
                                                                              float near,
                                                                              float far);
    [[nodiscard]] FORCE_INLINE static constexpr Projection CreateOrthographic(const Matrix4x4& m,
                                                                              float ndcNear,
                                                                              float ndcFar);
    [[nodiscard]] FORCE_INLINE static constexpr Projection CreatePerspective(float fov,
                                                                             float aspectRatio,
                                                                             float near,
                                                                             float far);
    [[nodiscard]] FORCE_INLINE static constexpr Projection CreatePerspective(const Matrix4x4& m,
                                                                             float ndcNear,
                                                                             float ndcFar);

    FORCE_INLINE constexpr Projection(const Projection&) = default;
    FORCE_INLINE constexpr Projection& operator=(const Projection& rhs);
    ~Projection() = default;

    [[nodiscard]] FORCE_INLINE constexpr eProjectionType GetType() const;
    [[nodiscard]] FORCE_INLINE constexpr float GetFOV() const;
    [[nodiscard]] FORCE_INLINE constexpr float GetHeight() const;
    [[nodiscard]] FORCE_INLINE constexpr float GetAspectRatio() const;
    [[nodiscard]] FORCE_INLINE constexpr float GetNearDistance() const;
    [[nodiscard]] FORCE_INLINE constexpr float GetFarDistance() const;
    [[nodiscard]] FORCE_INLINE constexpr Matrix4x4 GetMatrix(float ndcNear, float ndcFar) const;

    FORCE_INLINE constexpr void SetFOV(float fov);
    FORCE_INLINE constexpr void SetHeight(float height);
    FORCE_INLINE constexpr void SetAspectRatio(float aspectRatio);
    FORCE_INLINE void SetNearDistance(float near);
    FORCE_INLINE void SetFarDistance(float far);

    [[nodiscard]] constexpr bool operator==(const Projection& rhs) const;
    [[nodiscard]] constexpr bool operator!=(const Projection& rhs) const;

    [[nodiscard]] constexpr bool IsEqualApprox(const Projection& rhs) const;
    [[nodiscard]] constexpr bool IsNotEqualApprox(const Projection& rhs) const;

    [[nodiscard]] Vector4 Project(const Vector4& v, float ndcNear, float ndcFar) const;

    [[nodiscard]] std::string ToString() const;

  private:
    FORCE_INLINE constexpr Projection();

    eProjectionType mType;

    union
    {
        float mFov;
        float mViewHeight;
    };

    float mAspectRatio;
    float mNear;
    float mFar;
};

constexpr Projection Projection::CreateOrthographic(float viewHeight, float aspectRatio, float near, float far)
{
    HO_ASSERT(!math::IsEqualApprox(near, far), "near, far plane can't be same.");
    Projection proj;
    proj.mType = eProjectionType::Orthographic;
    proj.mViewHeight = viewHeight;
    proj.mAspectRatio = aspectRatio;
    proj.mNear = near;
    proj.mFar = far;

    return proj;
}

constexpr Projection Projection::CreateOrthographic(const Matrix4x4& m, float ndcNear, float ndcFar)
{
    Projection proj;
    proj.mType = eProjectionType::Orthographic;

    // Extract projection parameters from the Matrix.
    // Use double precision internally to minimize cancellation and rounding
    // errors.
    const double m00 = static_cast<double>(m.Data[0][0]);
    const double m11 = static_cast<double>(m.Data[1][1]);
    const double m22 = static_cast<double>(m.Data[2][2]);
    const double m23 = static_cast<double>(m.Data[2][3]);

    if (math::IsZeroApprox(static_cast<float>(m11)) || math::IsZeroApprox(static_cast<float>(m00)))
    {
        HO_ASSERT(false, "Matrix is invalid or uninitialized, fallback to zero projection.");
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
        proj.mNear = static_cast<float>((static_cast<double>(ndcNear) - m23) / m22);
        proj.mFar = static_cast<float>((static_cast<double>(ndcFar) - m23) / m22);
    }

    return proj;
}

constexpr Projection Projection::CreatePerspective(float fov, float aspectRatio, float near, float far)
{
    HO_ASSERT(!math::IsEqualApprox(near, far), "near, far plane can't be same.");
    Projection proj;
    proj.mType = eProjectionType::Perspective;
    proj.mFov = fov;
    proj.mAspectRatio = aspectRatio;
    proj.mNear = near;
    proj.mFar = far;

    return proj;
}

constexpr Projection Projection::CreatePerspective(const Matrix4x4& m, float ndcNear, float ndcFar)
{
    Projection proj;
    proj.mType = eProjectionType::Perspective;

    const double m00 = static_cast<double>(m.Data[0][0]);
    const double m11 = static_cast<double>(m.Data[1][1]);
    const double m22 = static_cast<double>(m.Data[2][2]);
    const double m23 = static_cast<double>(m.Data[2][3]);

    if (math::IsZeroApprox(static_cast<float>(m00)) || math::IsZeroApprox(static_cast<float>(m11)))
    {
        HO_ASSERT(false, "Matrix is invalid or uninitialized, fallback to zero projection.");
        proj.mFov = 0.0f;
        proj.mAspectRatio = 0.0f;
        proj.mNear = 0.0f;
        proj.mFar = 0.0f;
        return proj;
    }

    const double fovFactor = m11;
    proj.mFov = static_cast<float>(2.0 * std::atan(1.0 / fovFactor));

    proj.mAspectRatio = static_cast<float>(fovFactor / m00);

    if (math::IsZeroApprox(static_cast<float>(m22)))
    {
        proj.mNear = 0.0f;
        proj.mFar = 0.0f;
    }
    else
    {
        proj.mNear = static_cast<float>(m23 / (static_cast<double>(ndcNear) - m22));
        proj.mFar = static_cast<float>(m23 / (static_cast<double>(ndcFar) - m22));
    }

    return proj;
}

constexpr Projection::Projection()
  : mType(eProjectionType::Perspective)
  , mFov(0.0f)
  , mAspectRatio(0.0f)
  , mNear(0.0f)
  , mFar(0.0f)
{
}

constexpr Projection& Projection::operator=(const Projection& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    mType = rhs.mType;
    mFov = rhs.mFov;
    mAspectRatio = rhs.mAspectRatio;
    mNear = rhs.mNear;
    mFar = rhs.mFar;
    return *this;
}

constexpr eProjectionType Projection::GetType() const
{
    return mType;
}

constexpr float Projection::GetFOV() const
{
    HO_ASSERT(mType == eProjectionType::Perspective, "Orthographic projection doesn't have fov.");
    return mFov;
}

constexpr float Projection::GetHeight() const
{
    HO_ASSERT(mType == eProjectionType::Orthographic, "Perspective projection doesn't have height.");
    return mViewHeight;
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

constexpr Matrix4x4 Projection::GetMatrix(float ndcNear, float ndcFar) const
{
    switch (mType)
    {
        case eProjectionType::Orthographic:
        {
            const float viewWidth = mAspectRatio * mViewHeight;
            return Matrix4x4(Vector4(2.0f / viewWidth, 0.0f, 0.0f, 0.0f),
                             Vector4(0.0f, 2.0f / mViewHeight, 0.0f, 0.0f),
                             Vector4(0.0f, 0.0f, (ndcFar - ndcNear) / (mFar - mNear), 0.0f),
                             Vector4(0.0f, 0.0f, (ndcNear * mFar - ndcFar * mNear) / (mFar - mNear), 1.0f));
        }
        case eProjectionType::Perspective:
        {
            const float fovFactor = 1.0f / math::Tan(mFov * 0.5f);
            return Matrix4x4(Vector4(fovFactor / mAspectRatio, 0.0f, 0.0f, 0.0f),
                             Vector4(0.0f, fovFactor, 0.0f, 0.0f),
                             Vector4(0.0f, 0.0f, (ndcFar * mFar - ndcNear * mNear) / (mFar - mNear), 1.0f),
                             Vector4(0.0f, 0.0f, ((ndcNear - ndcFar) * mFar * mNear) / (mFar - mNear), 0.0f));
        }
        default:
            HO_ASSERT(false, "Invalid projection type.");
            return Matrix4x4();
    }
}

constexpr void Projection::SetFOV(float fov)
{
    HO_ASSERT(mType == eProjectionType::Perspective, "Orthographic projection doesn't have fov.");
    mFov = fov;
}

constexpr void Projection::SetHeight(float height)
{
    HO_ASSERT(mType == eProjectionType::Orthographic, "Perspective projection doesn't have height.");
    mViewHeight = height;
}

constexpr void Projection::SetAspectRatio(float aspectRatio)
{
    mAspectRatio = aspectRatio;
}

void Projection::SetNearDistance(float near)
{
    mNear = near;
}

void Projection::SetFarDistance(float far)
{
    mFar = far;
}

constexpr bool Projection::operator==(const Projection& rhs) const
{
    return mType == rhs.mType && mFov == rhs.mFov && mViewHeight == rhs.mViewHeight &&
           mAspectRatio == rhs.mAspectRatio && mNear == rhs.mNear && mFar == rhs.mFar;
}

constexpr bool Projection::operator!=(const Projection& rhs) const
{
    return !(*this == rhs);
}

constexpr bool Projection::IsEqualApprox(const Projection& rhs) const
{
    return mType == rhs.mType && math::IsEqualApprox(mFov, rhs.mFov) &&
           math::IsEqualApprox(mViewHeight, rhs.mViewHeight) && math::IsEqualApprox(mAspectRatio, rhs.mAspectRatio) &&
           math::IsEqualApprox(mNear, rhs.mNear) && math::IsEqualApprox(mFar, rhs.mFar);
}

constexpr bool Projection::IsNotEqualApprox(const Projection& rhs) const
{
    return !IsEqualApprox(rhs);
}
} // namespace ho