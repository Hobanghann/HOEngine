#pragma once

struct ImDrawData;

namespace ho
{
class IPlatformWindow;

struct UIViewportData
{
    UIViewportData();
    ~UIViewportData();

    UIViewportData(UIViewportData&) = delete;
    UIViewportData& operator=(UIViewportData&) = delete;

    UIViewportData(UIViewportData&&) noexcept;
    UIViewportData& operator=(UIViewportData&&) noexcept;

    const IPlatformWindow* pTargetWindow = nullptr;
    ImDrawData* pDrawData;
};

} // namespace ho