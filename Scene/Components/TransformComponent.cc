#include "TransformComponent.h"

#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{
RTransformComponent::RTransformComponent()
  : mhParent(ObjectHandle<RTransformComponent>::sNULL)
  , mhFirstChild(ObjectHandle<RTransformComponent>::sNULL)
  , mhNextSibling(ObjectHandle<RTransformComponent>::sNULL)
  , mhPrevSibling(ObjectHandle<RTransformComponent>::sNULL)
{
}

RTransformComponent::~RTransformComponent() = default;

void RTransformComponent::OnCreated()
{
    RComponent::OnCreated();

    mbIsDirty = true;
}

void RTransformComponent::OnAttached(ObjectHandle<RGameObject> hOwner)
{
    RComponent::OnAttached(hOwner);
}

void RTransformComponent::OnDetached()
{
    RComponent::OnDetached();
}

void RTransformComponent::OnUpdate(float deltaTime)
{
    RComponent::OnUpdate(deltaTime);

    updateTransformRecursive();
}

void RTransformComponent::OnPendingDestroyed()
{
    RComponent::OnPendingDestroyed();
}

void RTransformComponent::OnDestroyed()
{
    RComponent::OnDestroyed();

    ObjectHandle<RTransformComponent> hCurrChild = mhFirstChild;
    while (hCurrChild.IsValid())
    {
        const ObjectHandle<RTransformComponent> hNext = hCurrChild->mhNextSibling;

        RemoveChild(hCurrChild);

        if (mhParent.IsValid())
        {
            mhParent->AddChild(hCurrChild);
        }

        hCurrChild = hNext;
    }

    if (mhParent.IsValid())
    {
        mhParent->RemoveChild(GetHandle().Cast<RTransformComponent>());
    }

    mhFirstChild = ObjectHandle<RTransformComponent>::sNULL;
}

bool RTransformComponent::IsDirty() const
{
    return mbIsDirty;
}

const Vector3& RTransformComponent::GetScale() const
{
    return mScale;
}

const Quaternion& RTransformComponent::GetRotation() const
{
    return mRotation;
}

const Vector3& RTransformComponent::GetPosition() const
{
    return mPosition;
}

const Transform3D& RTransformComponent::GetLocalTransform() const
{
    HO_ASSERT(!mbIsDirty, "World Transform must be updated before getting.");
    return mLocalTransform;
}

const Transform3D& RTransformComponent::GetWorldTransform() const
{
    HO_ASSERT(!mbIsDirty, "World Transform must be updated before getting.");
    return mWorldTransform;
}

void RTransformComponent::SetScale(const Vector3& scale)
{
    mScale = scale;
    makeDirtyRecursive();
}

void RTransformComponent::SetRotation(const Quaternion& rotation)
{
    mRotation = rotation;
    makeDirtyRecursive();
}

void RTransformComponent::SetPosition(const Vector3& position)
{
    mPosition = position;
    makeDirtyRecursive();
}

void RTransformComponent::SetLocalTransform(const Transform3D& transform)
{
    mLocalTransform = transform;
    mScale = Vector3(mLocalTransform.GetScaleX(), mLocalTransform.GetScaleY(), mLocalTransform.GetScaleZ());
    const Matrix3x3 rotMat = Matrix3x3(mLocalTransform.Matrix).Orthonormalized();
    HO_ASSERT(rotMat.IsOrthonormal(), "Transform is not orthogonal.");
    mRotation = Quaternion::FromMatrix(rotMat);
    mPosition = transform.GetPosition();
    makeDirtyRecursive();
}

void RTransformComponent::SetWorldTransform(const Transform3D& transform)
{
    if (mhParent.IsValid())
    {
        mhParent->updateTransformRecursive();
        SetLocalTransform(mhParent->GetWorldTransform().Inverse() * transform);
    }
    else
    {
        SetLocalTransform(transform);
    }
}

void RTransformComponent::Scale(const Vector3& scale)
{
    mScale *= scale;
    mPosition *= scale;
    makeDirtyRecursive();
}

void RTransformComponent::ScaleUniform(float scale)
{
    Scale(Vector3(scale, scale, scale));
}

void RTransformComponent::RotateEuler(float angleXRad, float angleYRad, float angleZRad, math::eEulerOrder order)
{
    RotateQuaternion(Quaternion::FromEuler(angleXRad, angleYRad, angleZRad, order));
}

void RTransformComponent::RotateAxisAngle(const Vector3& axis, float angle)
{
    RotateQuaternion(Quaternion::FromAxisAngle(axis, angle));
}

void RTransformComponent::RotateQuaternion(const Quaternion& q)
{
    mRotation = q * mRotation;
    mRotation.Normalize();
    mPosition = q.Transform(mPosition);
    makeDirtyRecursive();
}

void RTransformComponent::Translate(const Vector3& translationVec)
{
    mPosition += translationVec;
    makeDirtyRecursive();
}

void RTransformComponent::ScaleLocal(const Vector3& scale)
{
    mScale *= scale;
    makeDirtyRecursive();
}

void RTransformComponent::ScaleUniformLocal(float scale)
{
    ScaleLocal(Vector3(scale, scale, scale));
}

void RTransformComponent::RotateEulerLocal(float angleXRad, float angleYRad, float angleZRad, math::eEulerOrder order)
{
    RotateQuaternionLocal(Quaternion::FromEuler(angleXRad, angleYRad, angleZRad, order));
}

void RTransformComponent::RotateAxisAngleLocal(const Vector3& axis, float angle)
{
    RotateQuaternionLocal(Quaternion::FromAxisAngle(axis, angle));
}

void RTransformComponent::RotateQuaternionLocal(const Quaternion& q)
{
    mRotation = mRotation * q;
    mRotation.Normalize();
    makeDirtyRecursive();
}

void RTransformComponent::TranslateLocal(const Vector3& translationVec)
{
    mPosition += mRotation.Transform(translationVec);
    makeDirtyRecursive();
}

void RTransformComponent::LookAt(const Vector3& at, const Vector3& up, bool bFacingAt)
{
    if (mbIsDirty)
    {
        updateTransformRecursive();
    }

    Transform3D worldTrans = mWorldTransform;
    worldTrans.LookAt(at, up, bFacingAt);

    SetWorldTransform(worldTrans);
}

void RTransformComponent::makeDirtyRecursive()
{
    if (mbIsDirty)
    {
        return;
    }

    mbIsDirty = true;

    ObjectHandle<RTransformComponent> hCurrChild = mhFirstChild;
    while (hCurrChild.IsValid())
    {
        hCurrChild->makeDirtyRecursive();
        hCurrChild = hCurrChild->mhNextSibling;
    }
}

void RTransformComponent::updateTransformRecursive()
{
    if (!mbIsDirty)
    {
        return;
    }

    mLocalTransform = Transform3D::FromScaleQuatertionPosition(mScale, mRotation, mPosition);

    if (mhParent.IsValid())
    {
        if (mhParent->mbIsDirty)
        {
            mhParent->updateTransformRecursive();
        }

        mWorldTransform = mhParent->mWorldTransform * mLocalTransform;
    }
    else
    {
        mWorldTransform = mLocalTransform;
    }

    mbIsDirty = false;
}

ObjectHandle<RTransformComponent> RTransformComponent::GetParent() const
{
    return mhParent;
}

ObjectHandle<RTransformComponent> RTransformComponent::GetFirstChild() const
{
    return mhFirstChild;
}

ObjectHandle<RTransformComponent> RTransformComponent::GetNextSibling() const
{
    return mhNextSibling;
}

ObjectHandle<RTransformComponent> RTransformComponent::GetPrevSibling() const
{
    return mhPrevSibling;
}

void RTransformComponent::AddChild(ObjectHandle<RTransformComponent> hChild)
{
    HO_ASSERT(hChild.IsValid(), "Component is invalid.");
    if (hChild->mhParent.IsValid())
    {
        HO_ASSERT(false, "hChild already has parent.");
        return;
    }

    updateTransformRecursive();
    hChild->updateTransformRecursive();
    const Transform3D childWorld = hChild->mWorldTransform;

    hChild->mhParent = GetHandle().Cast<RTransformComponent>();
    if (mhFirstChild.IsValid())
    {
        ObjectHandle<RTransformComponent> hLastChild = mhFirstChild;
        while (hLastChild->mhNextSibling.IsValid())
        {
            hLastChild = hLastChild->mhNextSibling;
        }
        hLastChild->mhNextSibling = hChild;
        hChild->mhPrevSibling = hLastChild;
    }
    else
    {
        mhFirstChild = hChild;
    }

    hChild->SetLocalTransform(mWorldTransform.Inverse() * childWorld);
}

void RTransformComponent::RemoveChild(ObjectHandle<RTransformComponent> hChild)
{
    HO_ASSERT(hChild.IsValid(), "Component is invalid.");
    if (hChild->mhParent != GetHandle().Cast<RTransformComponent>())
    {
        return;
    }

    hChild->updateTransformRecursive();

    if (hChild == mhFirstChild)
    {
        mhFirstChild = hChild->mhNextSibling;
    }
    if (hChild->mhPrevSibling.IsValid())
    {
        hChild->mhPrevSibling->mhNextSibling = hChild->mhNextSibling;
    }
    if (hChild->mhNextSibling.IsValid())
    {
        hChild->mhNextSibling->mhPrevSibling = hChild->mhPrevSibling;
    }

    hChild->mhParent = ObjectHandle<RTransformComponent>::sNULL;
    hChild->mhPrevSibling = ObjectHandle<RTransformComponent>::sNULL;
    hChild->mhNextSibling = ObjectHandle<RTransformComponent>::sNULL;

    hChild->SetLocalTransform(hChild->mWorldTransform);
}

bool RTransformComponent::IsAncestor(ObjectHandle<RTransformComponent> hAncestor) const
{
    ObjectHandle<RTransformComponent> hParent = mhParent;

    while (hParent.IsValid())
    {
        if (hParent == hAncestor)
        {
            return true;
        }
        hParent = hParent->mhParent;
    }

    return false;
}

bool RTransformComponent::IsDescendantRecursive(ObjectHandle<RTransformComponent> hDescendant) const
{
    ObjectHandle<RTransformComponent> hChild = mhFirstChild;

    while (hChild.IsValid())
    {
        if (hChild == hDescendant)
        {
            return true;
        }
        if (hChild->IsDescendantRecursive(hDescendant))
        {
            return true;
        }

        hChild = hChild->mhNextSibling;
    }

    return false;
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RTransformComponent, "Transform", RComponent)
HO_REGISTER_CLASS_PROPERTY(mScale,
                           "Scale",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragVector);
HO_REGISTER_CLASS_PROPERTY_MINMAX("Scale", 0.1f, FLT_MAX);

HO_REGISTER_CLASS_PROPERTY(mRotation,
                           "Rotation",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::EulerAngles);

HO_REGISTER_CLASS_PROPERTY(mPosition,
                           "Position",
                           PropertyFlags_VisibleInEditor | PropertyFlags_Editable,
                           ePropertyUIStyle::DragVector);
HO_REGISTER_CLASS_END()
} // namespace ho