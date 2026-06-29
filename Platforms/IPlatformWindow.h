#pragma once

#include <cstdint>
#include <string>

#include "Macros.h"

namespace ho
{
struct TitleBarTheme;

class IPlatformWindow
{
  public:
    IPlatformWindow(int32_t posX, int32_t posY, int32_t titleBarHeight, int32_t clientWidth, int32_t clientHeight)
      : mPosX(posX)
      , mPosY(posY)
      , mTitleBarHeight(titleBarHeight)
      , mClientWidth(clientWidth)
      , mClientHeight(clientHeight)
    {
        HO_ASSERT(mTitleBarHeight >= 0, "title bar height can't be negative.");
        HO_ASSERT(mClientWidth >= 0, "Window width can't be negative.");
        HO_ASSERT(mClientHeight >= 0, "Window height can't be negative.");
    }

    virtual ~IPlatformWindow() = default;

    virtual void* GetNativeHandle() const = 0;

    void SetTitle(const std::string& titleStr)
    {
        mTitleStr = titleStr;
    }

    const std::string& GetTitle() const
    {
        return mTitleStr;
    }

    void SetPosX(int32_t posX)
    {
        mPosX = posX;
    }

    void SetPosY(int32_t posY)
    {
        mPosY = posY;
    }

    int32_t GetPosX() const
    {
        return mPosX;
    }

    int32_t GetPosY() const
    {
        return mPosY;
    }

    int32_t GetTitleBarHeight() const
    {
        return mTitleBarHeight;
    }

    int32_t GetClientWidth() const
    {
        return mClientWidth;
    }

    int32_t GetClientHeight() const
    {
        return mClientHeight;
    }

    void Resize(int32_t clientWidth, int32_t clientHeight)
    {
        HO_ASSERT(clientWidth >= 0, "Window width can't be negative.");
        HO_ASSERT(clientHeight >= 0, "Window height can't be negative.");
        mClientWidth = clientWidth;
        mClientHeight = clientHeight;
    }

    void SetWindowMinimized(bool bMinimized)
    {
        mbMinimized = bMinimized;
    }

    void SetWindowMaximized(bool bMaximized)
    {
        mbMaximized = bMaximized;
    }

    void SetWindowResizing(bool bResizing)
    {
        mbResizing = bResizing;
    }

    bool IsWindowMinimized() const
    {
        return mbMinimized;
    }

    bool IsWindowMaximized() const
    {
        return mbMaximized;
    }

    bool IsWindowResizing() const
    {
        return mbResizing;
    }

    virtual void ActivateContext() const = 0;

    virtual void DeactivateContext() const = 0;

    virtual void Present() const = 0;

    virtual void ProcessWindowDragging() const = 0;
    virtual void Maximize() const = 0;
    virtual void Minimize() const = 0;
    virtual void Restore() const = 0;
    virtual void Close() const = 0;

  protected:
    std::string mTitleStr;

    int32_t mPosX = 0;
    int32_t mPosY = 0;

    int32_t mTitleBarHeight = 0;
    int32_t mClientWidth = 0;
    int32_t mClientHeight = 0;

    bool mbMinimized = false;
    bool mbMaximized = false;
    bool mbResizing = false;
};
} // namespace ho