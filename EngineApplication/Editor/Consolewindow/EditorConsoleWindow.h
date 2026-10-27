#pragma once

#include <imgui.h>

#include "../../IEngineWindow.h"
#include "Core/Log/LogDefs.h"
#include "Core/Log/LogSinkImgui.h"
#include "Core/Templates/FixedArray.h"

namespace ho
{
class Editor;

class EditorConsoleWindow final : public IEngineWindow
{
  public:
    EditorConsoleWindow();

    EditorConsoleWindow(const EditorConsoleWindow&) = delete;
    EditorConsoleWindow& operator=(const EditorConsoleWindow&) = delete;

    ~EditorConsoleWindow() override = default;

    bool OnInit() override;
    bool OnPreUpdate(float deltaTime) override;
    bool OnUpdate(float deltaTime) override;
    bool OnPostUpdate(float deltaTime) override;
    bool OnRender() override;
    void OnShutdown() override;

    void DrawUI() override;

  private:
    struct ConsoleLog
    {
        LogEntry Log;
        bool bVisible = true;
    };

    void drawMainWindow();
    void drawMenuBar();
    void drawOutputArea();
    void drawCommandInput();

    bool mbOpenMenuBarFiltersPopup = false;
    void drawMenuBarFiltersPopup();
    bool mbOpenMenuBarSettingsPopup = false;
    void drawMenuBarSettingsPopup();

    ImVec4& getLogLevelColor(eLogLevel level);

    void updateLogVisibilityOnTextFilter();

    int32_t mMaxLogCount = 1024;
    FixedArray<ConsoleLog> mLogs;
    int32_t mLogCount = 0;
    int32_t mHeadIdx = 0;

    char mCommandBuffer[512] = "";

    LogLevel mLogLevelFilterFlags;
    LogCategory mLogCategoryFilterFlags;

    bool mbShowTime = true;
    bool mbShowLogLevel = true;
    bool mbShowLogCategory = true;
    bool mbShowSourceLocation = true;
    bool mbShowMessage = true;

    ImVec4 mLogColors[6] = {};

    bool mbColoredByLevel = true;
    bool mbAutoScroll = true;
    bool mbScrollToBottom = false;

    ImGuiTextFilter mLogFilter;
};
} // namespace ho