#pragma once

#include "Core/Templates/FixedQueue.h"
#include "IUIWindow.h"
#include "TitleBarTheme.h"

struct ImFont;

namespace ho
{
class Engine;

class UISystem final
{
    friend Engine;

  public:
    UISystem(const UISystem&) = delete;
    UISystem& operator=(const UISystem&) = delete;

    static UISystem& GetInstance();

    void SubmitRenderWindow(IUIWindow* pWindow);

    void SetTitleBarTheme(const TitleBarTheme& theme)
    {
        mTitleBarTheme = theme;
    }

    TitleBarTheme& GetTitleBarTheme()
    {
        return mTitleBarTheme;
    }

    const TitleBarTheme& GetTitleBarTheme() const
    {
        return mTitleBarTheme;
    }

    void ActivateFaSolid();
    void ActivateFaRegular();
    void ActivateFaBrands();
    void DeactivateIconFont();

  private:
    static const int32_t sRenderWindowQueueSize = 32;

    UISystem();
    ~UISystem() = default;

    static void createInstance();

    static void deleteInstance();

    bool init();

    void submitDrawCommandForUI();

    void shutdown();

    TitleBarTheme mTitleBarTheme;

    FixedQueue<IUIWindow*> mRenderWindowQueue;

    ImFont* mpFaSolidFont = nullptr;
    ImFont* mpFaRegularFont = nullptr;
    ImFont* mpFaBrandsFont = nullptr;

    static UISystem* spInstance;
};
} // namespace ho