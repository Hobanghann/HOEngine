#include "UIViewportData.h"

#include <imgui.h>

namespace ho
{
UIViewportData::UIViewportData()
  : pTargetWindow(nullptr)
  , pDrawData(IM_NEW(ImDrawData))
{
}

UIViewportData::~UIViewportData()
{
    if (pDrawData)
    {
        for (int i = 0; i < pDrawData->CmdLists.Size; ++i)
        {
            IM_DELETE(pDrawData->CmdLists[i]);
        }

        IM_DELETE(pDrawData);
        pDrawData = nullptr;
    }
}

UIViewportData::UIViewportData(UIViewportData&& other) noexcept
  : pTargetWindow(other.pTargetWindow)
  , pDrawData(other.pDrawData)
{
    other.pTargetWindow = nullptr;
    other.pDrawData = nullptr;
}

UIViewportData& UIViewportData::operator=(UIViewportData&& other) noexcept
{
    if (this != &other)
    {
        if (pDrawData)
        {
            for (int i = 0; i < pDrawData->CmdLists.Size; ++i)
            {
                IM_DELETE(pDrawData->CmdLists[i]);
            }
            IM_DELETE(pDrawData);
        }

        pTargetWindow = other.pTargetWindow;
        pDrawData = other.pDrawData;

        other.pTargetWindow = nullptr;
        other.pDrawData = nullptr;
    }
    return *this;
}
} // namespace ho