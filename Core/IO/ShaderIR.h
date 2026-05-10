#pragma once

#include <string>

namespace ho
{
struct ShaderIR
{
    ShaderIR() = default;

    ShaderIR(std::string&& nameStr, std::string&& sourceStr) noexcept
      : NameStr(std::move(nameStr))
      , SourceStr(std::move(sourceStr))
    {
    }

    ShaderIR(ShaderIR&& rhs) noexcept = default;
    ShaderIR& operator=(ShaderIR&& rhs) noexcept = default;

    ShaderIR(const ShaderIR&) = delete;
    ShaderIR& operator=(const ShaderIR&) = delete;

    std::string NameStr;

    std::string SourceStr;
};
} // namespace ho