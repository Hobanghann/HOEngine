#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Image.h"
#include "Path.h"

namespace ho
{
namespace parser
{
struct TextureIR
{
    TextureIR(const Path& path, std::string&& nameStr, Image&& image)
      : ResourcePath(path)
      , NameStr(std::move(nameStr))
      , Img(std::move(image))
    {
    }

    TextureIR(const TextureIR&) = delete;
    TextureIR& operator=(const TextureIR&) = delete;

    TextureIR(TextureIR&& rhs) noexcept = default;
    TextureIR& operator=(TextureIR&& rhs) noexcept = default;

    Path ResourcePath;
    std::string NameStr;
    Image Img;
};
} // namespace parser
} // namespace ho