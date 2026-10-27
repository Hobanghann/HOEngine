#include "RenderBindingIndex.h"

#include "Subsystems/Asset/AssetDefs.h"

namespace ho
{
eTextureBindingIndex getTextureBindingIndex(eMaterialTextureUsage usage)
{
    switch (usage)
    {
        case eMaterialTextureUsage::Diffuse:
            return eTextureBindingIndex::Diffuse;
            break;
        case eMaterialTextureUsage::Specular:
            return eTextureBindingIndex::Specular;
            break;

        case eMaterialTextureUsage::Opacity:
            return eTextureBindingIndex::Opacity;
            break;
        case eMaterialTextureUsage::Normal:
            return eTextureBindingIndex::Normal;
            break;

        case eMaterialTextureUsage::Albedo:
            return eTextureBindingIndex::Albedo;
            break;
        case eMaterialTextureUsage::MetallicRoughness:
            return eTextureBindingIndex::MetallicRoughness;
            break;
        case eMaterialTextureUsage::Emissive:
            return eTextureBindingIndex::Emissive;
            break;
        case eMaterialTextureUsage::AmbientOcclusion:
            return eTextureBindingIndex::AmbientOcclusion;
            break;

        default:
            HO_ASSERT(false, "Invalid texture usage");
            return eTextureBindingIndex::Diffuse;
    }
}
} // namespace ho