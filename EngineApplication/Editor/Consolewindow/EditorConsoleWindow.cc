#include "EditorConsoleWindow.h"

#include <IconsMaterialDesignIcons.h>

#include "Core/Log/Logger.h"
#include "Platforms/IPlatformApplication.h"

namespace ho
{
static const char* getLogLevelIcon(eLogLevel level);
static const char* getLogLevelName(eLogLevel level);
static const char* getLogCategoryName(eLogCategory category);

EditorConsoleWindow::EditorConsoleWindow()
  : IEngineWindow(StringHandle("EditorConsoleWindow"))
  , mLogs(mMaxLogCount)
  , mLogLevelFilterFlags(LogLevel_Trace | LogLevel_Debug | LogLevel_Info | LogLevel_Warn | LogLevel_Error |
                         LogLevel_Critical)
  , mLogCategoryFilterFlags(LogCategory_System | LogCategory_Memory | LogCategory_IO | LogCategory_Asset |
                            LogCategory_Render | LogCategory_Window | LogCategory_Audio | LogCategory_Physics |
                            LogCategory_Scene | LogCategory_Script | LogCategory_UI)
{
    mLogColors[0] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);      // Trace
    mLogColors[1] = ImVec4(0.60f, 0.90f, 0.30f, 1.00f);      // Debug
    mLogColors[2] = ImGui::GetStyle().Colors[ImGuiCol_Text]; // Info
    mLogColors[3] = ImVec4(0.95f, 0.85f, 0.15f, 1.00f);      // Warn
    mLogColors[4] = ImVec4(0.95f, 0.25f, 0.25f, 1.00f);      // Error
    mLogColors[5] = ImVec4(1.00f, 0.30f, 1.00f, 1.00f);      // Critical
}

bool EditorConsoleWindow::OnInit()
{
    return true;
}

bool EditorConsoleWindow::OnPreUpdate(float deltaTime)
{
    return true;
}

bool EditorConsoleWindow::OnUpdate(float deltaTime)
{
    if (!mbOpen)
    {
        return true;
    }

    if (LogSinkImgui* pSink = Logger::GetImGuiSink())
    {
        LogEntry entry;
        while (!pSink->IsEmpty())
        {
            pSink->PopLog(&entry);
            if (mLogCount == mMaxLogCount)
            {
                mHeadIdx = (mHeadIdx + 1) % mMaxLogCount;
                --mLogCount;
            }
            ConsoleLog& log = mLogs[(mHeadIdx + mLogCount) % mMaxLogCount];
            log.Log = entry;
            log.bVisible = log.bVisible = !mLogFilter.IsActive() || mLogFilter.PassFilter(log.Log.Message.c_str());

            if (mbAutoScroll)
            {
                mbScrollToBottom = true;
            }

            ++mLogCount;
        }
    }

    return true;
}

bool EditorConsoleWindow::OnPostUpdate(float deltaTime)
{
    return true;
}

bool EditorConsoleWindow::OnRender()
{
    return true;
}

void EditorConsoleWindow::OnShutdown() {}

void EditorConsoleWindow::DrawUI()
{
    drawMainWindow();
}

void EditorConsoleWindow::drawMainWindow()
{
    if (ImGui::Begin("Console", &mbOpen, ImGuiWindowFlags_NoScrollbar))
    {
        SetFocused(ImGui::IsWindowFocused());

        drawMenuBar();
        drawOutputArea();
        drawCommandInput();
    }
    ImGui::End();
}

void EditorConsoleWindow::drawMenuBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);

    if (ImGui::BeginChild("##MenuBar", ImVec2(0.0f, 30.0f), 0, 0))
    {
        const float childHeight = 30.0f;
        const float widgetHeight = ImGui::GetFrameHeight();
        const float targetOffsetY = (childHeight - widgetHeight) * 0.5f;

        ImGui::SetCursorPos({ImGui::GetStyle().FramePadding.x, targetOffsetY});

        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::InputTextWithHint("##LogFilter",
                                     ICON_MDI_MAGNIFY "    Search Log",
                                     mLogFilter.InputBuf,
                                     IM_COUNTOF(mLogFilter.InputBuf),
                                     ImGuiInputTextFlags_EscapeClearsAll))
        {
            mLogFilter.Build();
            updateLogVisibilityOnTextFilter();
        }

        ImGui::SameLine();
        ImGui::SetCursorPosY(targetOffsetY);
        if (ImGui::Button(ICON_MDI_FILTER " Filters"))
        {
            mbOpenMenuBarFiltersPopup = true;
        }
        drawMenuBarFiltersPopup();

        const float settingsButtonWidth =
            ImGui::CalcTextSize(ICON_MDI_COG_OUTLINE " Settings").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        const float rightMargin = ImGui::GetStyle().ItemSpacing.x;
        const float targetPosX = ImGui::GetWindowWidth() - settingsButtonWidth - rightMargin;

        ImGui::SameLine();
        ImGui::SetCursorPosX(targetPosX);
        ImGui::SetCursorPosY(targetOffsetY);

        if (ImGui::Button(ICON_MDI_COG_OUTLINE " Settings"))
        {
            mbOpenMenuBarSettingsPopup = true;
        }
        drawMenuBarSettingsPopup();
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(2);
}

void EditorConsoleWindow::drawOutputArea()
{
    const float commandRegionHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("##OutputArea", ImVec2(0, -commandRegionHeight), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        for (int32_t i = 0; i < mLogCount; ++i)
        {
            const ConsoleLog& log = mLogs[(mHeadIdx + i) % mMaxLogCount];

            if (!log.bVisible)
            {
                continue;
            }

            if (!(log.Log.Level & mLogLevelFilterFlags) || !(log.Log.Category & mLogCategoryFilterFlags))
            {
                continue;
            }

            if (mbColoredByLevel)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, getLogLevelColor(log.Log.Level));
            }

            if (mbShowTime)
            {
                ImGui::Text("[%s]", log.Log.Time);
                ImGui::SameLine();
            }

            if (mbShowLogLevel)
            {
                ImGui::Text("[%s %s]", getLogLevelIcon(log.Log.Level), getLogLevelName(log.Log.Level));
                ImGui::SameLine();
            }

            if (mbShowLogCategory)
            {
                ImGui::Text("[%s]", getLogCategoryName(log.Log.Category));
                ImGui::SameLine();
            }

            if (mbShowSourceLocation)
            {
                ImGui::Text("[%s]", log.Log.SourceLocation);
                ImGui::SameLine();
            }

            if (mbShowMessage)
            {
                ImGui::Text("%s", log.Log.Message.c_str());
                ImGui::SameLine();
            }

            if (mbColoredByLevel)
            {
                ImGui::PopStyleColor();
            }

            ImGui::NewLine();
        }

        if (mbScrollToBottom || (mbAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
        {
            ImGui::SetScrollHereY(1.0f);
        }
        mbScrollToBottom = false;
    }
    ImGui::EndChild();
}

void EditorConsoleWindow::drawCommandInput()
{
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushItemWidth(-1);
    bool bReclaimFocus = false;
    if (ImGui::InputTextWithHint("##CommandInput",
                                 "> Enter Console Command",
                                 mCommandBuffer,
                                 IM_ARRAYSIZE(mCommandBuffer),
                                 ImGuiInputTextFlags_EnterReturnsTrue))
    {
        bReclaimFocus = true;
    }
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();

    ImGui::SetItemDefaultFocus();
    if (bReclaimFocus)
    {
        ImGui::SetKeyboardFocusHere(-1);
    }
}

void EditorConsoleWindow::drawMenuBarFiltersPopup()
{
    if (mbOpenMenuBarFiltersPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});

        ImGui::OpenPopup("##ConsoleMenuBarFiltersPopup");
        mbOpenMenuBarFiltersPopup = false;
    }

    ImGui::SetNextWindowSize(ImVec2(200.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));
    if (ImGui::BeginPopup("##ConsoleMenuBarFiltersPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::SeparatorText("LOG LEVEL");
        ImGui::Spacing();

        if (ImGui::MenuItem(ICON_MDI_TIMER "  Trace", nullptr, mLogLevelFilterFlags & LogLevel_Trace))
        {
            mLogLevelFilterFlags ^= LogLevel_Trace;
        }

        if (ImGui::MenuItem(ICON_MDI_BUG "  Debug", nullptr, mLogLevelFilterFlags & LogLevel_Debug))
        {
            mLogLevelFilterFlags ^= LogLevel_Debug;
        }

        if (ImGui::MenuItem(ICON_MDI_INFORMATION_SLAB_CIRCLE "  Info", nullptr, mLogLevelFilterFlags & LogLevel_Info))
        {
            mLogLevelFilterFlags ^= LogLevel_Info;
        }

        if (ImGui::MenuItem(ICON_MDI_ALERT "  Warn", nullptr, mLogLevelFilterFlags & LogLevel_Warn))
        {
            mLogLevelFilterFlags ^= LogLevel_Warn;
        }

        if (ImGui::MenuItem(ICON_MDI_ALERT_OCTAGRAM "  Error", nullptr, mLogLevelFilterFlags & LogLevel_Error))
        {
            mLogLevelFilterFlags ^= LogLevel_Error;
        }

        if (ImGui::MenuItem(ICON_MDI_SKULL "  Critical", nullptr, mLogLevelFilterFlags & LogLevel_Critical))
        {
            mLogLevelFilterFlags ^= LogLevel_Critical;
        }

        ImGui::Spacing();
        ImGui::SeparatorText("LOG CATEGORY");
        ImGui::Spacing();

        if (ImGui::MenuItem("System", nullptr, mLogCategoryFilterFlags & LogCategory_System))
        {
            mLogCategoryFilterFlags ^= LogCategory_System;
        }
        if (ImGui::MenuItem("Memory", nullptr, mLogCategoryFilterFlags & LogCategory_Memory))
        {
            mLogCategoryFilterFlags ^= LogCategory_Memory;
        }
        if (ImGui::MenuItem("IO", nullptr, mLogCategoryFilterFlags & LogCategory_IO))
        {
            mLogCategoryFilterFlags ^= LogCategory_IO;
        }
        if (ImGui::MenuItem("Asset", nullptr, mLogCategoryFilterFlags & LogCategory_Asset))
        {
            mLogCategoryFilterFlags ^= LogCategory_Asset;
        }

        if (ImGui::MenuItem("Render", nullptr, mLogCategoryFilterFlags & LogCategory_Render))
        {
            mLogCategoryFilterFlags ^= LogCategory_Render;
        }
        if (ImGui::MenuItem("Window", nullptr, mLogCategoryFilterFlags & LogCategory_Window))
        {
            mLogCategoryFilterFlags ^= LogCategory_Window;
        }
        if (ImGui::MenuItem("Audio", nullptr, mLogCategoryFilterFlags & LogCategory_Audio))
        {
            mLogCategoryFilterFlags ^= LogCategory_Audio;
        }
        if (ImGui::MenuItem("Physics", nullptr, mLogCategoryFilterFlags & LogCategory_Physics))
        {
            mLogCategoryFilterFlags ^= LogCategory_Physics;
        }

        if (ImGui::MenuItem("Scene", nullptr, mLogCategoryFilterFlags & LogCategory_Scene))
        {
            mLogCategoryFilterFlags ^= LogCategory_Scene;
        }
        if (ImGui::MenuItem("Script", nullptr, mLogCategoryFilterFlags & LogCategory_Script))
        {
            mLogCategoryFilterFlags ^= LogCategory_Script;
        }
        if (ImGui::MenuItem("UI", nullptr, mLogCategoryFilterFlags & LogCategory_UI))
        {
            mLogCategoryFilterFlags ^= LogCategory_UI;
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}

void EditorConsoleWindow::drawMenuBarSettingsPopup()
{
    if (mbOpenMenuBarSettingsPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});

        ImGui::OpenPopup("##ConsoleMenuBarSettingsPopup");
        mbOpenMenuBarSettingsPopup = false;
    }

    ImGui::SetNextWindowSize(ImVec2(200.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));
    if (ImGui::BeginPopup("##ConsoleMenuBarSettingsPopup", ImGuiWindowFlags_NoDecoration))
    {
        ImGui::SeparatorText("GENERAL");
        ImGui::Spacing();

        if (ImGui::MenuItem("Colored Logs", nullptr, mbColoredByLevel))
        {
            mbColoredByLevel = !mbColoredByLevel;
        }

        if (ImGui::MenuItem("Auto Scroll", nullptr, mbAutoScroll))
        {
            mbAutoScroll = !mbAutoScroll;
        }

        ImGui::Spacing();
        ImGui::SeparatorText("LOG FIELDS");
        ImGui::Spacing();

        if (ImGui::MenuItem("Show Time", nullptr, mbShowTime))
        {
            mbShowTime = !mbShowTime;
        }

        if (ImGui::MenuItem("Show Level", nullptr, mbShowLogLevel))
        {
            mbShowLogLevel = !mbShowLogLevel;
        }

        if (ImGui::MenuItem("Show Category", nullptr, mbShowLogCategory))
        {
            mbShowLogCategory = !mbShowLogCategory;
        }

        if (ImGui::MenuItem("Show Location", nullptr, mbShowSourceLocation))
        {
            mbShowSourceLocation = !mbShowSourceLocation;
        }

        if (ImGui::MenuItem("Show Message", nullptr, mbShowMessage))
        {
            mbShowMessage = !mbShowMessage;
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}

ImVec4& EditorConsoleWindow::getLogLevelColor(eLogLevel level)
{
    switch (level)
    {
        case LogLevel_Trace:
            return mLogColors[0];
        case LogLevel_Debug:
            return mLogColors[1];
        case LogLevel_Info:
            return mLogColors[2];
        case LogLevel_Warn:
            return mLogColors[3];
        case LogLevel_Error:
            return mLogColors[4];
        case LogLevel_Critical:
            return mLogColors[5];
        default:
            HO_ASSERT(false, "Invalid LogLevel");
            return mLogColors[2];
    }
}

void EditorConsoleWindow::updateLogVisibilityOnTextFilter()
{
    if (!mLogFilter.IsActive())
    {
        for (int32_t i = 0; i < mLogCount; ++i)
        {
            mLogs[(mHeadIdx + i) % mMaxLogCount].bVisible = true;
        }
    }
    else
    {
        for (int32_t i = 0; i < mLogCount; ++i)
        {
            ConsoleLog& log = mLogs[(mHeadIdx + i) % mMaxLogCount];
            log.bVisible = false;
            if (mLogFilter.PassFilter(log.Log.Message.c_str()))
            {
                log.bVisible = true;
            }
        }
    }
}

const char* getLogLevelIcon(eLogLevel level)
{
    switch (level)
    {
        case LogLevel_Trace:
            return ICON_MDI_TIMER;
        case LogLevel_Debug:
            return ICON_MDI_BUG;
        case LogLevel_Info:
            return ICON_MDI_INFORMATION_SLAB_CIRCLE;
        case LogLevel_Warn:
            return ICON_MDI_ALERT;
        case LogLevel_Error:
            return ICON_MDI_ALERT_OCTAGRAM;
        case LogLevel_Critical:
            return ICON_MDI_SKULL;
        default:
            HO_ASSERT(false, "Invalid LogLevel");
            return "Unknown";
    }
}

const char* getLogLevelName(eLogLevel level)
{
    switch (level)
    {
        case LogLevel_Trace:
            return "Trace";
        case LogLevel_Debug:
            return "Debug";
        case LogLevel_Info:
            return "Info";
        case LogLevel_Warn:
            return "Warn";
        case LogLevel_Error:
            return "Error";
        case LogLevel_Critical:
            return "Critical";
        default:
            HO_ASSERT(false, "Invalid LogLevel");
            return "Unknown";
    }
}

const char* getLogCategoryName(eLogCategory category)
{
    switch (category)
    {
        case LogCategory_None:
            return "None";
        case LogCategory_System:
            return "System";
        case LogCategory_Memory:
            return "Memory";
        case LogCategory_IO:
            return "IO";
        case LogCategory_Asset:
            return "Asset";
        case LogCategory_Render:
            return "Render";
        case LogCategory_Window:
            return "Window";
        case LogCategory_Audio:
            return "Audio";
        case LogCategory_Physics:
            return "Physics";
        case LogCategory_Scene:
            return "Scene";
        case LogCategory_Script:
            return "Script";
        case LogCategory_UI:
            return "UI";
        default:
            HO_ASSERT(false, "Invalid LogCategory");
            return "Unknown";
    }
}
} // namespace ho