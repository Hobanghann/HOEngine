#pragma once

#include "Core/Memory/WeakTypedHandle.h"
#include "Core/String/StringHandle.h"
#include "Subsystems/Asset/AssetDefs.h"

namespace ho
{

struct FrameBufferDesc
{
    static constexpr int32_t sMaxColorTargetCount = 8;

    StringHandle hName = StringHandle::sNULL;
    int32_t Width = 0;
    int32_t Height = 0;
    eTextureFormat ColorTargetFormats[sMaxColorTargetCount] = {};
    eTextureFormat DepthTargetFormat = eTextureFormat::None;
};

struct FrameBuffer;
using FrameBufferHandle = WeakTypedHandle<FrameBuffer, TypedObjectPool>;
} // namespace ho