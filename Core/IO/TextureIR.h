#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Core/IO/Image.h"

namespace ho
{
struct TextureIR
{
    TextureIR(std::string&& nameStr, Image&& image)
      : NameStr(std::move(nameStr))
      , Img(std::move(image))
    {
    }

    TextureIR(TextureIR&& rhs) noexcept = default;
    TextureIR& operator=(TextureIR&& rhs) noexcept = default;

    TextureIR(const TextureIR&) = delete;
    TextureIR& operator=(const TextureIR&) = delete;

    std::string NameStr;
    Image Img;
};
} // namespace ho