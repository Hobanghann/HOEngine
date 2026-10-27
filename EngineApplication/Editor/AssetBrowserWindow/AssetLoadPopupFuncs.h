#pragma once

#include "Macros.h"

namespace ho
{
struct ModelParseContext;
struct ModelImportContext;
struct ShaderParseContext;
struct ShaderImportContext;
struct TextureParseContext;
struct TextureImportContext;
struct TextureBakeContext;

enum class ePopupResult
{
    None,
    Confirm,
    Cancel
};

ePopupResult drawModelParseSettingPopup(ModelParseContext* pOutContext);
void drawModelParsingPopup(bool bClosePopup);
ePopupResult drawModelImportSettingPopup(ModelImportContext* pOutContext);
void drawModelImportingPopup(const ModelImportContext& context);
ePopupResult drawModelImportFailedPopup(const ModelImportContext& context);

ePopupResult drawShaderParseSettingPopup(ShaderParseContext* pOutContext);
void drawShaderParsingPopup(bool bClosePopup);
ePopupResult drawShaderImportSettingPopup(ShaderImportContext* pOutContext);
void drawShaderImportingPopup(bool bClosePopup);
ePopupResult drawShaderImportFailedPopup(const ShaderImportContext& context);

ePopupResult drawTextureParseSettingPopup(TextureParseContext* pOutContext);
void drawTextureParsingPopup(bool bClosePopup);
ePopupResult drawTextureImportSettingPopup(TextureImportContext* pOutContext);
void drawTextureImportingPopup(const TextureImportContext& context);
ePopupResult drawTextureImportFailedPopup(const TextureImportContext& context);

ePopupResult drawTextureBakeSettingPopup(TextureBakeContext* pOutContext);
void drawTextureBakingPopup(bool bClosePopup);
ePopupResult drawTextureBakeFailedPopup(const TextureBakeContext& context);
} // namespace ho