#pragma once

#include <cstdint>
#include <string>

namespace ho
{
class IPlatformWindow
{
  public:
    IPlatformWindow(int32_t clientWidth, int32_t clientHeight)
      : mClientWidth(clientWidth)
      , mClientHeight(clientHeight)
    {
    }

    virtual ~IPlatformWindow() = default;

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
        mClientWidth = clientWidth;
        mClientHeight = clientHeight;
    }

    virtual void ActivateContext() const = 0;

    virtual void DeactivateContext() const = 0;

    virtual void Present() const = 0;

    virtual void SetTitle(const std::wstring& titleStr) const = 0;

  protected:
    int32_t mClientWidth = 0;
    int32_t mClientHeight = 0;
};
} // namespace ho