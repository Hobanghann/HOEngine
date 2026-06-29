#include "IWin32Window.h"

#include <imgui.h>

namespace ho
{
void IWin32Window::ProcessWindowDragging() const
{
    const BOOL bSuccess = ::ReleaseCapture();
    HO_ASSERT(bSuccess != FALSE, "ReleaseCapture failed.");
    (void)bSuccess;
    if (FAILED(::SendMessageW(mhWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0)))
    {
        HO_ASSERT(false, "SendMessageW failed.");
    }

    ::SetActiveWindow(mhWnd);
    ::SetFocus(mhWnd);

    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGuiIO& io = ImGui::GetIO();

        io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    }
}

} // namespace ho