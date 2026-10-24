#pragma once

#include "AssetDefs.h"

namespace ho
{
namespace texutils
{
bool isUncompressedTextureFormat(eTextureFormat format);
bool isLinearTextureFormat(eTextureFormat format);
bool isIntegerTextureFormat(eTextureFormat format);
int32_t getTexturePixelBytes(eTextureFormat format);
int32_t getTextureBlockCount(int32_t length);
int32_t getTextureBlockBytes(eTextureFormat format);
int32_t calculateMipLevels(int32_t width, int32_t height);
} // namespace texutils
} // namespace ho