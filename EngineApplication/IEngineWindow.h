#pragma once

#include "Core/String/StringHandle.h"
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

    virtual bool OnInit() = 0;
    virtual bool OnPreUpdate(float deltaTime) = 0;
    virtual bool OnUpdate(float deltaTime) = 0;
    virtual bool OnPostUpdate(float deltaTime) = 0;
    virtual bool OnRender() = 0;
    virtual void OnShutdown() = 0;

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

    // Implements the logic to render a single window, spanning from ImGui::Begin() to ImGui::End().
    // Derived classes must ensure that the Begin() and End() pair is properly matched to form a complete window scope.
    void DrawUI() override = 0;

  protected:
    StringHandle mhName;
    bool mbFocused = false;
    bool mbOpen = false;
};
} // namespace ho