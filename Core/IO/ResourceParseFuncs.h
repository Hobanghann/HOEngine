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

[[nodiscard]] std::unique_ptr<const ModelIR> parseModelFile(const Path& path,
                                                            const std::string& nameStr,
                                                            bool bMakeStatic,
                                                            bool bConvertToLeftHanded);

[[nodiscard]] std::unique_ptr<const TextureIR> parseTextureFile(const Path& path,
                                                                const std::string& nameStr,
                                                                bool bIsLinear);

[[nodiscard]] std::unique_ptr<const ShaderIR> parseShaderFile(const Path& path, const std::string& nameStr);
} // namespace parser

} // namespace ho