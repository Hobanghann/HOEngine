#include "StaticMeshAsset.h"

#include "Subsystems/Reflection/ClassRegister.h"
#include "Subsystems/Reflection/StructRegister.h"

namespace ho
{
int32_t RStaticMeshAsset::GetByteSize() const
{
    int32_t byteSize = 0;
    if (TransferState == eAssetTransferState::Ready)
    {
        byteSize += sizeof(RStaticMeshAsset);
        byteSize += Positions.GetByteSize();
        byteSize += Normals.GetByteSize();
        byteSize += Tangents.GetByteSize();
        byteSize += UVs.GetByteSize();
        byteSize += Colors.GetByteSize();
        byteSize += Indices.GetByteSize();
        byteSize += SubMeshes.GetByteSize();
    }
    return byteSize;
}

HO_REGISTER_STRUCT_BEGIN(RStaticMeshAsset::SubMesh, "Submesh")
HO_REGISTER_STRUCT_PROPERTY(hName, "Name")
HO_REGISTER_STRUCT_PROPERTY(PrimitiveType, "PrimitiveType")
HO_REGISTER_STRUCT_PROPERTY(VertexCount, "VertexCount")
HO_REGISTER_STRUCT_PROPERTY(IndexCount, "IndexCount")
HO_REGISTER_STRUCT_PROPERTY(hRenderMaterialName, "RenderMaterialName")
HO_REGISTER_STRUCT_PROPERTY(hRenderMaterial, "RenderMaterial")
HO_REGISTER_STRUCT_PROPERTY(AabbVolume, "AABB")
HO_REGISTER_STRUCT_PROPERTY(SphereVolume, "Sphere")
HO_REGISTER_STRUCT_END()

HO_REGISTER_CLASS_DERIVED_BEGIN(RStaticMeshAsset, "Static Mesh", RIAsset)
HO_REGISTER_CLASS_PROPERTY(hName, "Name")
HO_REGISTER_CLASS_PROPERTY(SubMeshes, "SubMeshes")
HO_REGISTER_CLASS_PROPERTY(AabbVolume, "AABB")
HO_REGISTER_CLASS_PROPERTY(SphereVolume, "Sphere")
HO_REGISTER_CLASS_END()
} // namespace ho