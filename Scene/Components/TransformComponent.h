#pragma once

#include "Component.h"
#include "Core/Math/Transform3D.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
class RGameObject;

class RTransformComponent : public RComponent
{
    HO_INJECT_REFLECTION_CODE(RTransformComponent)

  public:
    RTransformComponent();
    ~RTransformComponent() override;

    void OnCreated() override;

    void OnAttached(ObjectHandle<RGameObject> hOwner) override;
    void OnDetached() override;

    void OnUpdate(float deltaTime) override;

    void OnPendingDestroyed() override;
    void OnDestroyed() override;

    [[nodiscard]] bool IsDirty() const;

    [[nodiscard]] const Vector3& GetScale() const;
    [[nodiscard]] const Quaternion& GetRotation() const;
    [[nodiscard]] const Vector3& GetPosition() const;
    [[nodiscard]] const Transform3D& GetLocalTransform() const;
    [[nodiscard]] const Transform3D& GetWorldTransform() const;

    void SetScale(const Vector3& scale);
    void SetRotation(const Quaternion& rotation);
    void SetPosition(const Vector3& position);
    void SetLocalTransform(const Transform3D& transform);
    void SetWorldTransform(const Transform3D& transform);

    void Scale(const Vector3& scale);
    void ScaleUniform(float scale);
    void RotateAxisAngle(const Vector3& axis, float angle);
    void RotateEuler(float angleXRad,
                     float angleYRad,
                     float angleZRad,
                     math::eEulerOrder order = math::eEulerOrder::ZYX);
    void RotateQuaternion(const Quaternion& q);
    void Translate(const Vector3& translationVec);

    void ScaleLocal(const Vector3& scale);
    void ScaleUniformLocal(float scale);
    void RotateAxisAngleLocal(const Vector3& axis, float angle);
    void RotateEulerLocal(float angleXRad,
                          float angleYRad,
                          float angleZRad,
                          math::eEulerOrder order = math::eEulerOrder::ZYX);
    void RotateQuaternionLocal(const Quaternion& q);
    void TranslateLocal(const Vector3& translationVec);

    void LookAt(const Vector3& at, const Vector3& up, bool bFacingAt = false);

    [[nodiscard]] ObjectHandle<RTransformComponent> GetParent() const;
    [[nodiscard]] ObjectHandle<RTransformComponent> GetFirstChild() const;
    [[nodiscard]] ObjectHandle<RTransformComponent> GetNextSibling() const;
    [[nodiscard]] ObjectHandle<RTransformComponent> GetPrevSibling() const;

    // AddChild ensure child has same world transform after adding.
    void AddChild(ObjectHandle<RTransformComponent> hChild);
    // RemoveChild ensures child has same world transform after removing.
    void RemoveChild(ObjectHandle<RTransformComponent> hChild);

    bool IsAncestor(ObjectHandle<RTransformComponent> hAncestor) const;
    bool IsDescendantRecursive(ObjectHandle<RTransformComponent> hDescendant) const;

  protected:
    void makeDirtyRecursive();
    void updateTransformRecursive();

    Vector3 mScale = Vector3(1.0f, 1.0f, 1.0f);
    Quaternion mRotation;
    Vector3 mPosition;

    Transform3D mLocalTransform;
    Transform3D mWorldTransform;

    // It means world transform need to be updated.
    bool mbIsDirty = true;

    ObjectHandle<RTransformComponent> mhParent;
    ObjectHandle<RTransformComponent> mhFirstChild;
    ObjectHandle<RTransformComponent> mhNextSibling;
    ObjectHandle<RTransformComponent> mhPrevSibling;
};
} // namespace ho