#pragma once

namespace ho
{

using EnumFlags = int;

enum eEnumFlags
{
    EnumFlags_None = 0,
};

using StructFlags = int;

enum eStructFlags
{
    StructFlags_None = 0,
};

using ClassFlags = int;

enum eClassFlags
{
    ClassFlags_None = 0,
};

using PropertyFlags = int;

enum ePropertyFlags
{
    PropertyFlags_None = 0,
    PropertyFlags_VisibleInEditor = 1,
    PropertyFlags_Editable = 1 << 1,
};

enum class ePropertyUIStyle
{
    None = 0,
    DragInt,
    DragInt2,
    DragInt3,
    DragInt4,
    DragFloat,
    DragFloat2,
    DragFloat3,
    DragFloat4,
    DragVector,
    DragAngle,
    DragAndDropAsset, // MUST be used with a AssetHandle.
    Color3,
    Color4,
    Checkbox,
    ComboBox,    // It must used with reflection registed enum.
    EulerAngles, // Converts Quaternion to Degree (0~360°) Vector3 in Inspector. MUST be used with a Quaternion.
};

using MethodFlags = int;

enum eMethodFlags
{
    MethodFlags_None = 0,
};

} // namespace ho