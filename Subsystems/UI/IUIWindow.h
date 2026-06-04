#pragma once

#include <string>

#include "Core/Templates/GlobalPoolIndex.h"

namespace ho
{
class UISystem;

class IUIWindow
{
    friend UISystem;

  public:
    IUIWindow(StringHandle hName)
      : mhName(hName)
    {
    }

    IUIWindow(const IUIWindow&) = delete;
    IUIWindow& operator=(const IUIWindow&) = delete;
    virtual ~IUIWindow() = default;

    StringHandle GetName() const
    {
        return mhName;
    }

    bool IsVisible() const
    {
        return mbIsVisible;
    }

    void SetVisible(bool bFlag)
    {
        mbIsVisible = bFlag;
    }

  protected:
    virtual void onGUI() = 0;

    StringHandle mhName;
    bool mbIsVisible = false;
};

} // namespace ho