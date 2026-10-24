#include "ShaderAsset.h"

#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{
int32_t RShaderAsset::GetByteSize() const
{
    int32_t byteSize = 0;
    if (TransferState == eAssetTransferState::Ready)
    {
        byteSize += sizeof(RShaderAsset);
        byteSize += static_cast<int32_t>(SourceStr.capacity() * sizeof(char));
        byteSize += Binary.GetByteSize();
    }
    return byteSize;
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RShaderAsset, "Shader", RIAsset)
HO_REGISTER_CLASS_PROPERTY(Stage, "Stage")
HO_REGISTER_CLASS_PROPERTY(CompatibleMaterialType, "Compatible Material Type")
HO_REGISTER_CLASS_END()
} // namespace ho