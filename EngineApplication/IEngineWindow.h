#pragma once

#include "Core/Templates/GlobalPoolIndex.h"
#include "Subsystems/UI/IUIDrawable.h"

namespace ho
{
class IEngineWindow : public IUIDrawable
{
  public:
    IEngineWindow(StringHandle hName)
      : mhName(hName)
    {
    }

    ~IEngineWindow() override = default;

    StringHandle GetName() const
    {
        return mhName;
    }

    void SetFocused(bool bFocused)
    {
        mbFocused = bFocused;
    }

    bool IsFocused() const
    {
        return mbFocused;
    }

    void SetOpen(bool bOpen)
    {
        mbOpen = bOpen;
    }

    bool IsOpen() const
    {
        return mbOpen;
    }

    void DrawUI() override = 0;

  protected:
    StringHandle mhName;
    bool mbFocused = false;
    bool mbOpen = false;
};
} // namespace ho