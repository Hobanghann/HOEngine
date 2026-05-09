#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Core/IO/Image.h"

namespace ho
{
struct TextureIR
{
    enum class eTextureType
    {
        None,
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCubeMap,
        Texture1DArray,
        Texture2DArray,
        TextureCubeMapArray,
    };

    TextureIR(std::string&& nameStr, std::vector<std::unique_ptr<Image>>&& images, eTextureType type)
      : NameStr(std::move(nameStr))
      , pImages(std::move(images))
      , Type(type)
    {
    }

    TextureIR(TextureIR&& rhs) noexcept = default;
    TextureIR& operator=(TextureIR&& rhs) noexcept = default;

    TextureIR(const TextureIR&) = delete;
    TextureIR& operator=(const TextureIR&) = delete;

    std::string NameStr;
    std::vector<std::unique_ptr<Image>> pImages;
    eTextureType Type = eTextureType::None;
};
} // namespace ho