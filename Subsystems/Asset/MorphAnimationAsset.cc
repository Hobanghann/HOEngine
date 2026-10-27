#include "MorphAnimationAsset.h"

#include "Subsystems/Reflection/ClassRegister.h"
#include "Subsystems/Reflection/StructRegister.h"

namespace ho
{
int32_t RMorphAnimationAsset::GetByteSize() const
{
    int32_t byteSize = 0;
    if (TransferState == eAssetTransferState::Ready)
    {
        byteSize += sizeof(RMorphAnimationAsset);
        byteSize += Tracks.GetByteSize();

        for (const auto& track : Tracks)
        {
            byteSize += track.SubmeshIndices.GetByteSize();
            byteSize += track.KeySequence.GetByteSize();

            for (const auto& key : track.KeySequence)
            {
                byteSize += key.MorphTargetIndices.GetByteSize();
                byteSize += key.Weights.GetByteSize();
            }
        }
    }
    return byteSize;
}

HO_REGISTER_STRUCT_BEGIN(RMorphAnimationAsset::MorphKey, "Morph Key")
HO_REGISTER_STRUCT_PROPERTY(Time, "Time")
HO_REGISTER_STRUCT_PROPERTY(MorphTargetIndices, "MorphTargetIndices")
HO_REGISTER_STRUCT_PROPERTY(Weights, "Weights")
HO_REGISTER_STRUCT_END()

HO_REGISTER_STRUCT_BEGIN(RMorphAnimationAsset::MorphTrack, "Morph Track")
HO_REGISTER_STRUCT_PROPERTY(SubmeshIndices, "SubmeshIndices")
HO_REGISTER_STRUCT_PROPERTY(KeySequence, "KeySequence")
HO_REGISTER_STRUCT_END()

HO_REGISTER_CLASS_DERIVED_BEGIN(RMorphAnimationAsset, "Morph Animation", RIAsset)
HO_REGISTER_CLASS_PROPERTY(hName, "Name")
HO_REGISTER_CLASS_PROPERTY(Duration, "Duration")
HO_REGISTER_CLASS_PROPERTY(Tracks, "Tracks")
HO_REGISTER_CLASS_END()
} // namespace ho