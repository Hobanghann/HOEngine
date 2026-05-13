#pragma once

#include <memory>
#include <string>

namespace ho
{
class Path;
class Image;

namespace parser
{
struct ModelIR;
struct TextureIR;
struct ShaderIR;

[[nodiscard]] std::unique_ptr<const ModelIR> parseModelFile(const std::string& nameStr,
                                                            const Path& path,
                                                            bool bMakeStatic,
                                                            bool bConvertToLeftHanded);

[[nodiscard]] std::unique_ptr<const TextureIR> parseTextureFile(const std::string& nameStr, const Path& path);

[[nodiscard]] std::unique_ptr<const ShaderIR> parseShaderFile(const std::string& nameStr, const Path& path);
} // namespace parser

} // namespace ho