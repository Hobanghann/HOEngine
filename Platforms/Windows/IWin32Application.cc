#include "IWin32Application.h"

#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ho
{

IWin32Application::~IWin32Application() {}

IWin32Application::IWin32Application(HINSTANCE hApp)
  : IPlatformApplication()
  , mhApp(hApp)
  , mhMainWnd(nullptr)
{
}

bool IWin32Application::ProcessPlatformMessages()
{
    static MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

bool IWin32Application::SetWindowTitle(std::wstring& title)
{
    return SetWindowTextW(mhMainWnd, title.c_str());
}

LRESULT CALLBACK IWin32Application::wndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, iMessage, wParam, lParam))
    {
        return true;
    }

    switch (iMessage)
    {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                sMainWindowWidth = LOWORD(lParam);
                sMainWindowHeight = HIWORD(lParam);
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
            {
                return 0;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }

    return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

} // namespace ho
