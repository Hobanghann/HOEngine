#include "MorphTargetsAsset.h"

#include "Subsystems/Reflection/ClassRegister.h"
#include "Subsystems/Reflection/StructRegister.h"

namespace ho
{
int32_t RMorphTargetsAsset::GetByteSize() const
{
    int32_t byteSize = 0;
    if (TransferState == eAssetTransferState::Ready)
    {
        byteSize += sizeof(RMorphTargetsAsset);
        byteSize += Positions.GetByteSize();
        byteSize += Normals.GetByteSize();
        byteSize += Tangents.GetByteSize();
        byteSize += SubMeshMorphTargets.GetByteSize();
        for (const auto& subMeshMorphTarget : SubMeshMorphTargets)
        {
            byteSize += subMeshMorphTarget.Targets.GetByteSize();
        }
    }
    return byteSize;
}

HO_REGISTER_STRUCT_BEGIN(RMorphTargetsAsset::MorphTarget, "Morph Target")
HO_REGISTER_STRUCT_PROPERTY(hName, "Name")
HO_REGISTER_STRUCT_PROPERTY(Weight, "Weight")
HO_REGISTER_STRUCT_END()

HO_REGISTER_STRUCT_BEGIN(RMorphTargetsAsset::MorphTargets, "Morph Targets Data")
HO_REGISTER_STRUCT_PROPERTY(SubMeshIndex, "SubMeshIndex")
HO_REGISTER_STRUCT_PROPERTY(VertexCount, "VertexCount")
HO_REGISTER_STRUCT_PROPERTY(Targets, "Targets")
HO_REGISTER_STRUCT_END()

HO_REGISTER_CLASS_DERIVED_BEGIN(RMorphTargetsAsset, "Morph Targets", RIAsset)
HO_REGISTER_CLASS_PROPERTY(hName, "Name")
HO_REGISTER_CLASS_PROPERTY(SubMeshMorphTargets, "SubMeshMorphTargets")
HO_REGISTER_CLASS_END()
} // namespace ho