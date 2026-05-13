#pragma once

#include <string>

namespace ho
{
namespace parser
{
struct ShaderIR
{
    ShaderIR(std::string&& nameStr, std::string&& sourceStr) noexcept
      : NameStr(std::move(nameStr))
      , SourceStr(std::move(sourceStr))
    {
    }

    ShaderIR(const ShaderIR&) = delete;
    ShaderIR& operator=(const ShaderIR&) = delete;

    ShaderIR(ShaderIR&& rhs) noexcept = default;
    ShaderIR& operator=(ShaderIR&& rhs) noexcept = default;

    std::string NameStr;

    std::string SourceStr;
};
} // namespace parser
} // namespace ho