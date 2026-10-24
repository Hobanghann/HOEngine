#include "TextureAsset.h"

#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{
int32_t RTextureAsset::GetByteSize() const
{
    int32_t byteSize = 0;
    if (TransferState == eAssetTransferState::Ready)
    {
        byteSize += sizeof(RTextureAsset);
        byteSize += Layouts.GetByteSize();
        byteSize += DataBlob.GetByteSize();
    }
    return byteSize;
}

HO_REGISTER_CLASS_DERIVED_BEGIN(RTextureAsset, "Texture", RIAsset)
HO_REGISTER_CLASS_PROPERTY(Width, "Width")
HO_REGISTER_CLASS_PROPERTY(Height, "Height")
HO_REGISTER_CLASS_PROPERTY(Format, "Format");
HO_REGISTER_CLASS_PROPERTY(Type, "Type");
HO_REGISTER_CLASS_END()
} // namespace ho
