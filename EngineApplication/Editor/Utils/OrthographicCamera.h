#pragma once

#include "Core/Math/Projection.h"
#include "Core/Math/Transform3D.h"
#include "Core/Math/Vector3.h"

namespace ho
{
class OrthographicCamera final
{
  public:
    FORCE_INLINE OrthographicCamera(float height, float aspectRatio, float near, float far);
    FORCE_INLINE OrthographicCamera(const OrthographicCamera&) = default;
    FORCE_INLINE OrthographicCamera& operator=(const OrthographicCamera& rhs) = default;
    ~OrthographicCamera() = default;

    [[nodiscard]] FORCE_INLINE Vector3 GetPosition() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetForward() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetRight() const;
    [[nodiscard]] FORCE_INLINE Vector3 GetUp() const;

    FORCE_INLINE void Zoom(float delta);

    FORCE_INLINE void MoveForward(float distance);
    FORCE_INLINE void MoveRight(float distance);
    FORCE_INLINE void MoveUp(float distance);

    FORCE_INLINE void SetRotation(float pitch, float yaw, float roll);
    FORCE_INLINE void SetPosition(const Vector3 position);

    FORCE_INLINE float GetHeight() const;
    FORCE_INLINE float GetNearDistance() const;
    FORCE_INLINE float GetFarDistance() const;

    FORCE_INLINE void SetHeight(float height);
    FORCE_INLINE void SetAspectRatio(float aspectRatio);
    FORCE_INLINE void SetNearDistance(float near);
    FORCE_INLINE void SetFarDistance(float far);

    [[nodiscard]] FORCE_INLINE Matrix4x4 GetViewMatrix() const;
    [[nodiscard]] FORCE_INLINE Matrix4x4 GetProjMatrix() const;
    [[nodiscard]] FORCE_INLINE const Projection& GetProjection() const;

  private:
    float mPitch = 0.0f;
    float mYaw = 0.0f;
    float mRoll = 0.0f;
    Vector3 mPosition;
    Projection mProjection;
};

OrthographicCamera::OrthographicCamera(float height, float aspectRatio, float near, float far)
  : mProjection(Projection::CreateOrthographic(height, aspectRatio, near, far))
{
}

Vector3 OrthographicCamera::GetPosition() const
{
    return mPosition;
}

Vector3 OrthographicCamera::GetForward() const
{
    Transform3D cameraWorld;
    cameraWorld.RotateEuler(mPitch, mYaw, mRoll);
    return cameraWorld.GetForward();
}

Vector3 OrthographicCamera::GetRight() const
{
    Transform3D cameraWorld;
    cameraWorld.RotateEuler(mPitch, mYaw, mRoll);
    return cameraWorld.GetRight();
}

Vector3 OrthographicCamera::GetUp() const
{
    Transform3D cameraWorld;
    cameraWorld.RotateEuler(mPitch, mYaw, mRoll);
    return cameraWorld.GetUp();
}

void OrthographicCamera::Zoom(float amount)
{
    const float nextHeight = mProjection.GetHeight() + amount;
    mProjection.SetHeight(math::Max(0.0001f, nextHeight));
}

void OrthographicCamera::MoveForward(float distance)
{
    mPosition += GetForward() * distance;
}

void OrthographicCamera::MoveRight(float distance)
{
    mPosition += GetRight() * distance;
}

void OrthographicCamera::MoveUp(float distance)
{
    mPosition += GetUp() * distance;
}

void OrthographicCamera::SetRotation(float pitch, float yaw, float roll)
{
    mPitch = pitch;
    mYaw = yaw;
    mRoll = roll;
}

void OrthographicCamera::SetPosition(const Vector3 position)
{
    mPosition = position;
}

float OrthographicCamera::GetHeight() const
{
    return mProjection.GetHeight();
}

float OrthographicCamera::GetNearDistance() const
{
    return mProjection.GetNearDistance();
}

float OrthographicCamera::GetFarDistance() const
{
    return mProjection.GetFarDistance();
}

void OrthographicCamera::SetHeight(float height)
{
    mProjection.SetHeight(height);
}

void OrthographicCamera::SetAspectRatio(float aspectRatio)
{
    mProjection.SetAspectRatio(aspectRatio);
}

void OrthographicCamera::SetNearDistance(float near)
{
    mProjection.SetNearDistance(near);
}

void OrthographicCamera::SetFarDistance(float far)
{
    mProjection.SetFarDistance(far);
}

Matrix4x4 OrthographicCamera::GetViewMatrix() const
{
    Transform3D cameraWorld;
    cameraWorld.RotateEuler(mPitch, mYaw, mRoll);
    cameraWorld.Translate(mPosition);
    cameraWorld.InvertFast();
    return cameraWorld.Matrix;
}

Matrix4x4 OrthographicCamera::GetProjMatrix() const
{
    const Matrix4x4 proj = mProjection.GetMatrix();
    return proj;
}

const Projection& OrthographicCamera::GetProjection() const
{
    return mProjection;
}

} // namespace ho