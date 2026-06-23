#pragma once

#include "Core/Math/Color128.h"

namespace ho
{
struct TitleBarTheme
{
    Color128 BgColor = Color128(0.2f, 0.2f, 0.2f, 1.0f);
    int32_t Height = 30;

    Color128 TextColor = Color128(0.8f, 0.8f, 0.8f, 1.0f);
    float TextSize = 16.0f;

    int32_t IconWidth = 20;
    int32_t IconHeight = 20;

    Color128 ButtonIconColor = Color128(0.8f, 0.8f, 0.8f, 1.0f);

    Color128 ButtonHoveredColor = Color128(0.3f, 0.3f, 0.3f, 1.0f);
    Color128 ButtonActiveColor = Color128(0.5f, 0.5f, 0.5f, 1.0f);

    Color128 CloseButtonHoveredColor = Color128(0.85f, 0.15f, 0.15f, 1.0f);
    Color128 CloseButtonActiveColor = Color128(0.65f, 0.05f, 0.05f, 1.0f);
};
} // namespace ho