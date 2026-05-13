#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Image.h"

namespace ho
{
namespace parser
{
struct TextureIR
{
    TextureIR(std::string&& nameStr, Image&& image)
      : NameStr(std::move(nameStr))
      , Img(std::move(image))
    {
    }

    TextureIR(const TextureIR&) = delete;
    TextureIR& operator=(const TextureIR&) = delete;

    TextureIR(TextureIR&& rhs) noexcept = default;
    TextureIR& operator=(TextureIR&& rhs) noexcept = default;

    std::string NameStr;
    Image Img;
};
} // namespace parser
} // namespace ho