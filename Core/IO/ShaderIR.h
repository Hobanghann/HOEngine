#pragma once

#include <string>

#include "Path.h"

namespace ho
{
namespace parser
{
struct ShaderIR
{
    ShaderIR(const Path& path, std::string&& nameStr, std::string&& sourceStr) noexcept
      : ResourcePath(path)
      , NameStr(std::move(nameStr))
      , SourceStr(std::move(sourceStr))
    {
    }

    ShaderIR(const ShaderIR&) = delete;
    ShaderIR& operator=(const ShaderIR&) = delete;

    ShaderIR(ShaderIR&& rhs) noexcept = default;
    ShaderIR& operator=(ShaderIR&& rhs) noexcept = default;

    Path ResourcePath;
    std::string NameStr;
    std::string SourceStr;
};
} // namespace parser
} // namespace ho