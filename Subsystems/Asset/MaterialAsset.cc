#include "MaterialAsset.h"

#include "Subsystems/Reflection/ClassRegister.h"
#include "Subsystems/Reflection/StructRegister.h"

namespace ho
{
int32_t RMaterialAsset::GetByteSize() const
{
    int32_t byteSize = 0;
    if (TransferState == eAssetTransferState::Ready)
    {
        byteSize += sizeof(RMaterialAsset);
    }
    return byteSize;
}

HO_REGISTER_STRUCT_BEGIN(RMaterialAsset::RenderPipelineState, "Pipeline State")
HO_REGISTER_STRUCT_PROPERTY(AlphaMode, "AlphaMode")
HO_REGISTER_STRUCT_PROPERTY(AlphaBlendMode, "AlphaBlendMode")
HO_REGISTER_STRUCT_PROPERTY(bWireframe, "Wireframe")
HO_REGISTER_STRUCT_PROPERTY(bBackfaceCulling, "BackfaceCulling")
HO_REGISTER_STRUCT_END()

HO_REGISTER_CLASS_DERIVED_BEGIN(RMaterialAsset, "Material", RIAsset)
HO_REGISTER_CLASS_PROPERTY(hName, "Name")
HO_REGISTER_CLASS_PROPERTY(Type, "Type")
HO_REGISTER_CLASS_PROPERTY(PipelineState, "PipelineState")

HO_REGISTER_CLASS_PROPERTY(hShaders, "Shaders")

HO_REGISTER_CLASS_PROPERTY(Ambient, "Ambient")
HO_REGISTER_CLASS_PROPERTY(Diffuse, "Diffuse")
HO_REGISTER_CLASS_PROPERTY(Specular, "Specular")
HO_REGISTER_CLASS_PROPERTY(Shininess, "Shininess")

HO_REGISTER_CLASS_PROPERTY(Albedo, "Albedo")
HO_REGISTER_CLASS_PROPERTY(Metallic, "Metallic")
HO_REGISTER_CLASS_PROPERTY(Roughness, "Roughness")
HO_REGISTER_CLASS_PROPERTY(IndexOfRefraction, "IndexOfRefraction")

HO_REGISTER_CLASS_PROPERTY(Opacity, "Opacity")
HO_REGISTER_CLASS_PROPERTY(AlphaThreshold, "AlphaThreshold")
HO_REGISTER_CLASS_PROPERTY(Emissive, "Emissive")
HO_REGISTER_CLASS_PROPERTY(EmissiveIntensity, "EmissiveIntensity")
HO_REGISTER_CLASS_PROPERTY(NormalScale, "NormalScale")
HO_REGISTER_CLASS_PROPERTY(OcclusionStrength, "OcclusionStrength")

HO_REGISTER_CLASS_PROPERTY(hTextureNames, "TextureNames")
HO_REGISTER_CLASS_PROPERTY(hTextures, "Textures")
HO_REGISTER_CLASS_PROPERTY(UVChannels, "UVChannels")
HO_REGISTER_CLASS_PROPERTY(UVTransforms, "UVTransforms")
HO_REGISTER_CLASS_END()
} // namespace ho