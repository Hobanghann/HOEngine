#pragma once

#include "Core/IO/Path.h"
#include "Core/String/StringHandle.h"
#include "Subsystems/Object/Object.h"

namespace ho
{
enum class eAssetTransferState
{
    Ready,
    PendingUpload,
    PendingDownload,
    Failed
};

struct RIAsset : public RObject
{
    HO_INJECT_REFLECTION_CODE(RIAsset)

    RIAsset() = default;
    RIAsset(const RIAsset&) = delete;
    RIAsset& operator=(const RIAsset&) = delete;
    ~RIAsset() override = default;

    RIAsset(RIAsset&&) noexcept = default;
    RIAsset& operator=(RIAsset&&) noexcept = default;

    virtual int32_t GetByteSize() const = 0;

    eAssetTransferState TransferState = eAssetTransferState::PendingUpload;
    Path AssetPath;
    StringHandle hName = StringHandle::sNULL;
};
} // namespace ho