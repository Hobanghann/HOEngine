#pragma once

#include "Core/Math/Projection.h"
#include "Core/Math/Transform3D.h"
#include "Core/Math/Vector3.h"

namespace ho
{
class PerspectiveCamera final
{
  public:
    FORCE_INLINE PerspectiveCamera(
        const Vector3& eye, const Vector3& at, const Vector3& up, float fov, float aspectRatio, float near, float far);
    FORCE_INLINE PerspectiveCamera(const PerspectiveCamera&) = default;
    FORCE_INLINE PerspectiveCamera& operator=(const PerspectiveCamera& rhs) = default;
    ~PerspectiveCamera() = default;

    [[nodiscard]] FORCE_INLINE Vector3 GetPosition() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetForward() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetRight() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetUp() const;

    FORCE_INLINE void AddPitch(float pitch);
    FORCE_INLINE void AddYaw(float yaw);

    FORCE_INLINE void MoveForward(float distance);
    FORCE_INLINE void MoveRight(float distance);
    FORCE_INLINE void MoveUp(float distance);

    FORCE_INLINE void SetPitch(float pitch);
    FORCE_INLINE void SetYaw(float yaw);
    FORCE_INLINE void SetPosition(const Vector3 position);

    FORCE_INLINE void LookAt(const Vector3& at);

    FORCE_INLINE float GetFOV() const;
    FORCE_INLINE float GetNearDistance() const;
    FORCE_INLINE float GetFarDistance() const;

    FORCE_INLINE void SetFOV(float fov);
    FORCE_INLINE void SetAspectRatio(float aspectRatio);
    FORCE_INLINE void SetNearDistance(float near);
    FORCE_INLINE void SetFarDistance(float far);

    [[nodiscard]] FORCE_INLINE Matrix4x4 GetViewMatrix() const;
    [[nodiscard]] FORCE_INLINE Matrix4x4 GetProjMatrix() const;
    [[nodiscard]] FORCE_INLINE const Projection& GetProjection() const;

  private:
    float mPitch = 0.0f;
    float mYaw = 0.0f;
    Vector3 mPosition;
    Projection mProjection;
};

PerspectiveCamera::PerspectiveCamera(
    const Vector3& eye, const Vector3& at, const Vector3& up, float fov, float aspectRatio, float near, float far)
  : mPosition(eye)
  , mProjection(Projection::CreatePerspective(fov, aspectRatio, near, far))
{
    LookAt(at);
}

Vector3 PerspectiveCamera::GetPosition() const
{
    return mPosition;
}

Vector3 PerspectiveCamera::GetForward() const
{
    Transform3D cameraWorld;
    cameraWorld.RotateEuler(mPitch, mYaw, 0.0f);
    return cameraWorld.GetForward();
}

Vector3 PerspectiveCamera::GetRight() const
{
    Transform3D cameraWorld;
    cameraWorld.RotateEuler(mPitch, mYaw, 0.0f);
    return cameraWorld.GetRight();
}

Vector3 PerspectiveCamera::GetUp() const
{
    Transform3D cameraWorld;
    cameraWorld.RotateEuler(mPitch, mYaw, 0.0f);
    return cameraWorld.GetUp();
}

void PerspectiveCamera::AddPitch(float pitch)
{
    mPitch += pitch;
    mPitch = math::Clamp(mPitch, -math::PI2 + 0.01f, math::PI2 - 0.01f);
}

void PerspectiveCamera::AddYaw(float yaw)
{
    mYaw += yaw;
}

void PerspectiveCamera::MoveForward(float distance)
{
    mPosition += GetForward() * distance;
}

void PerspectiveCamera::MoveRight(float distance)
{
    mPosition += GetRight() * distance;
}

void PerspectiveCamera::MoveUp(float distance)
{
    mPosition += Vector3::sUnitY * distance;
}

void PerspectiveCamera::SetPitch(float pitch)
{
    mPitch = pitch;
    mPitch = math::Clamp(mPitch, -math::PI2 + math::EPSILON_ORTHO, math::PI2 - math::EPSILON_ORTHO);
}

void PerspectiveCamera::SetYaw(float yaw)
{
    mYaw = yaw;
}

void PerspectiveCamera::SetPosition(const Vector3 position)
{
    mPosition = position;
}

void PerspectiveCamera::LookAt(const Vector3& at)
{
    const Vector3 forward = (at - mPosition).Normalized();
    mPitch = math::Asin(-forward.Y);
    mYaw = math::Atan2(forward.X, forward.Z);
    mPitch = math::Clamp(mPitch, -math::PI2 + math::EPSILON_ORTHO, math::PI2 - math::EPSILON_ORTHO);
}

float PerspectiveCamera::GetFOV() const
{
    return mProjection.GetFOV();
}

float PerspectiveCamera::GetNearDistance() const
{
    return mProjection.GetNearDistance();
}

float PerspectiveCamera::GetFarDistance() const
{
    return mProjection.GetFarDistance();
}

void PerspectiveCamera::SetFOV(float fov)
{
    mProjection.SetFOV(fov);
}

void PerspectiveCamera::SetAspectRatio(float aspectRatio)
{
    mProjection.SetAspectRatio(aspectRatio);
}

void PerspectiveCamera::SetNearDistance(float near)
{
    mProjection.SetNearDistance(near);
}

void PerspectiveCamera::SetFarDistance(float far)
{
    mProjection.SetFarDistance(far);
}

Matrix4x4 PerspectiveCamera::GetViewMatrix() const
{
    Transform3D cameraWorld;
    cameraWorld.RotateEuler(mPitch, mYaw, 0.0f);
    cameraWorld.Translate(mPosition);
    cameraWorld.InvertFast();
    return cameraWorld.Matrix;
}

Matrix4x4 PerspectiveCamera::GetProjMatrix() const
{
    const Matrix4x4 proj = mProjection.GetMatrix();
    return proj;
}

const Projection& PerspectiveCamera::GetProjection() const
{
    return mProjection;
}

} // namespace ho