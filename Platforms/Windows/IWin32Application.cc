#include "IWin32Application.h"

#include <imgui_impl_win32.h>
#include <objbase.h>
#include <shlobj.h>
#include <stb_image.h>
#include <vector>
#include <windowsx.h>

#include "IWin32Window.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ho
{
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

IWin32Application::IWin32Application(HINSTANCE hApp)
  : IPlatformApplication()
  , mhApp(hApp)
  , mhMainWnd(nullptr)
  , mhIcon(nullptr)
{
}

bool IWin32Application::ShowOpenFileDialog(std::string* pOutPathStr,
                                           const std::string& titleStr,
                                           const std::string* pFilterNamesStr,
                                           const std::string* pFilterExtensionsStr,
                                           int32_t filterCount,
                                           const std::string& initialDirPathStr)
{
    if (!pOutPathStr || !pFilterNamesStr || !pFilterExtensionsStr)
    {
        HO_ASSERT(false, "Invalid parameter.");
        return false;
    }

    IFileOpenDialog* pFileDialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (FAILED(hr))
    {
        HO_ASSERT(false, "Failed to create FileOpenDialog instance.");
        return false;
    }

    DWORD dwFlags;
    pFileDialog->GetOptions(&dwFlags);
    dwFlags |= FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_NOCHANGEDIR;
    pFileDialog->SetOptions(dwFlags);

    if (!titleStr.empty())
    {
        const std::wstring titleStrUTF16 = toUTF16(titleStr);
        pFileDialog->SetTitle(titleStrUTF16.c_str());
    }

    std::vector<COMDLG_FILTERSPEC> filterSpecs(filterCount);
    std::vector<std::vector<std::wstring>> filterStrsUTF16(filterCount);
    for (int32_t i = 0; i < filterCount; ++i)
    {
        filterStrsUTF16[i] = std::vector<std::wstring>(2);
        filterStrsUTF16[i][0] = toUTF16(pFilterNamesStr[i]);
        filterStrsUTF16[i][1] = toUTF16(pFilterExtensionsStr[i]);
        filterSpecs[i] = {filterStrsUTF16[i][0].c_str(), filterStrsUTF16[i][1].c_str()};
    }

    pFileDialog->SetFileTypes(static_cast<UINT>(filterSpecs.size()), filterSpecs.data());

    if (!initialDirPathStr.empty())
    {
        IShellItem* pInitialFolder = nullptr;
        const std::wstring initialDirPathStrUTF16 = toUTF16(initialDirPathStr);
        if (SUCCEEDED(
                SHCreateItemFromParsingName(initialDirPathStrUTF16.c_str(), nullptr, IID_PPV_ARGS(&pInitialFolder))))
        {
            pFileDialog->SetFolder(pInitialFolder);
            pInitialFolder->Release();
        }
    }

    HWND hwndOwner = static_cast<HWND>(spMainWindow->GetNativeHandle());

    hr = pFileDialog->Show(hwndOwner);

    bool bSuccess = false;

    if (SUCCEEDED(hr))
    {
        IShellItem* pItem = nullptr;

        if (SUCCEEDED(pFileDialog->GetResult(&pItem)))
        {
            PWSTR pFilePath = nullptr;
            if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath)))
            {
                *pOutPathStr = toUTF8(pFilePath);

                CoTaskMemFree(pFilePath);

                bSuccess = true;
            }
            pItem->Release();
        }
    }

    pFileDialog->Release();

    return bSuccess;
}

bool IWin32Application::ShowOpenFilesDialog(std::string* pOutPathsStr,
                                            int32_t* pOutPathCount,
                                            int32_t maxOutPathCount,
                                            const std::string& titleStr,
                                            const std::string* pFilterNamesStr,
                                            const std::string* pFilterExtensionsStr,
                                            int32_t filterCount,
                                            const std::string& initialDirPathStr)
{
    if (!pOutPathsStr || !pOutPathCount || maxOutPathCount <= 0 || !pFilterNamesStr || !pFilterExtensionsStr)
    {
        HO_ASSERT(false, "Invalid parameter.");
        return false;
    }

    *pOutPathCount = 0;

    IFileOpenDialog* pFileDialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (FAILED(hr))
    {
        HO_ASSERT(false, "Failed to create FileOpenDialog instance.");
        return false;
    }

    DWORD dwFlags;
    pFileDialog->GetOptions(&dwFlags);
    dwFlags |= FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_NOCHANGEDIR | FOS_ALLOWMULTISELECT;
    pFileDialog->SetOptions(dwFlags);

    if (!titleStr.empty())
    {
        const std::wstring titleStrUTF16 = toUTF16(titleStr);
        pFileDialog->SetTitle(titleStrUTF16.c_str());
    }

    std::vector<COMDLG_FILTERSPEC> filterSpecs(filterCount);
    std::vector<std::vector<std::wstring>> filterStrsUTF16(filterCount);
    for (int32_t i = 0; i < filterCount; ++i)
    {
        filterStrsUTF16[i] = std::vector<std::wstring>(2);
        filterStrsUTF16[i][0] = toUTF16(pFilterNamesStr[i]);
        filterStrsUTF16[i][1] = toUTF16(pFilterExtensionsStr[i]);
        filterSpecs[i] = {filterStrsUTF16[i][0].c_str(), filterStrsUTF16[i][1].c_str()};
    }
    pFileDialog->SetFileTypes(static_cast<UINT>(filterSpecs.size()), filterSpecs.data());

    if (!initialDirPathStr.empty())
    {
        IShellItem* pInitialFolder = nullptr;
        const std::wstring initialDirPathStrUTF16 = toUTF16(initialDirPathStr);
        if (SUCCEEDED(
                SHCreateItemFromParsingName(initialDirPathStrUTF16.c_str(), nullptr, IID_PPV_ARGS(&pInitialFolder))))
        {
            pFileDialog->SetFolder(pInitialFolder);
            pInitialFolder->Release();
        }
    }

    HWND hwndOwner = static_cast<HWND>(spMainWindow->GetNativeHandle());
    hr = pFileDialog->Show(hwndOwner);

    bool bSuccess = false;

    if (SUCCEEDED(hr))
    {
        IShellItemArray* pResults = nullptr;

        if (SUCCEEDED(pFileDialog->GetResults(&pResults)))
        {
            DWORD numSelected = 0;
            pResults->GetCount(&numSelected);

            const int32_t readCount = std::min(maxOutPathCount, static_cast<int32_t>(numSelected));

            for (int32_t i = 0; i < readCount; ++i)
            {
                IShellItem* pItem = nullptr;
                if (SUCCEEDED(pResults->GetItemAt(i, &pItem)))
                {
                    PWSTR pFilePath = nullptr;
                    if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath)))
                    {
                        pOutPathsStr[i] = toUTF8(pFilePath);
                        CoTaskMemFree(pFilePath);
                    }
                    pItem->Release();
                }
            }

            *pOutPathCount = readCount;
            bSuccess = (readCount > 0);

            pResults->Release();
        }
    }

    pFileDialog->Release();

    return bSuccess;
}

void IWin32Application::loadWindowsIcon(const std::string& iconPathStr)
{
    if (iconPathStr.empty())
    {
        return;
    }

    // Load icon image
    int width = 0;
    int height = 0;
    int channels = 0;
    uint8_t* pIconBitmap = stbi_load(iconPathStr.c_str(), &width, &height, &channels, 4);

    if (pIconBitmap == nullptr)
    {
        HO_ASSERT(false, "Failed to load icon image.");
        return;
    }

    // Load Windows icon from image
    BITMAPV5HEADER bi;
    ::ZeroMemory(&bi, sizeof(bi));
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = width;
    bi.bV5Height = height;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    HDC hdc = ::GetDC(nullptr);

    void* lpBits = nullptr;
    HBITMAP hColorBitmap =
        ::CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS, &lpBits, nullptr, 0);
    ::ReleaseDC(nullptr, hdc);

    if (hColorBitmap == nullptr || lpBits == nullptr)
    {
        HO_ASSERT(false, "Failed to create DIB section.");
        stbi_image_free(pIconBitmap);
        return;
    }

    DWORD* lpdwPixel = reinterpret_cast<DWORD*>(lpBits);
    for (int32_t y = 0; y < height; ++y)
    {
        const int32_t sourceY = height - 1 - y;

        for (int32_t x = 0; x < width; ++x)
        {
            const int32_t pixelIndex = (sourceY * width) + x;
            const uint8_t r = pIconBitmap[pixelIndex * 4 + 0];
            const uint8_t g = pIconBitmap[pixelIndex * 4 + 1];
            const uint8_t b = pIconBitmap[pixelIndex * 4 + 2];
            const uint8_t a = pIconBitmap[pixelIndex * 4 + 3];

            *lpdwPixel = (a << 24) | (r << 16) | (g << 8) | b;
            ++lpdwPixel;
        }
    }

    HBITMAP hMonoBitmap = ::CreateBitmap(width, height, 1, 1, nullptr);
    if (hMonoBitmap == nullptr)
    {
        HO_ASSERT(false, "Failed to create mono bitmap.");
        stbi_image_free(pIconBitmap);
        ::DeleteObject(hColorBitmap);
        return;
    }

    ICONINFO ii;
    ::ZeroMemory(&ii, sizeof(ii));
    ii.fIcon = TRUE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmMask = hMonoBitmap;
    ii.hbmColor = hColorBitmap;

    mhIcon = ::CreateIconIndirect(&ii);

    ::DeleteObject(hColorBitmap);
    ::DeleteObject(hMonoBitmap);
    stbi_image_free(pIconBitmap);
}

std::wstring IWin32Application::toUTF16(const std::string& utf8Str)
{
    if (utf8Str.empty())
    {
        return std::wstring();
    }

    const int targetLength =
        ::MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int>(utf8Str.length()), nullptr, 0);

    if (targetLength <= 0)
    {
        return std::wstring();
    }

    std::wstring utf16Str;
    utf16Str.resize(static_cast<size_t>(targetLength));

    ::MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int>(utf8Str.length()), &utf16Str[0], targetLength);

    return utf16Str;
}

std::string IWin32Application::toUTF8(const std::wstring& utf16Str)
{
    if (utf16Str.empty())
    {
        return std::string();
    }

    const int targetLength = ::WideCharToMultiByte(
        CP_UTF8, 0, utf16Str.c_str(), static_cast<int>(utf16Str.length()), nullptr, 0, nullptr, nullptr);

    if (targetLength <= 0)
    {
        return std::string();
    }

    std::string utf8Str;
    utf8Str.resize(static_cast<size_t>(targetLength));

    ::WideCharToMultiByte(
        CP_UTF8, 0, utf16Str.c_str(), static_cast<int>(utf16Str.length()), &utf8Str[0], targetLength, nullptr, nullptr);

    return utf8Str;
}

LRESULT CALLBACK IWin32Application::wndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, iMessage, wParam, lParam))
    {
        return true;
    }

    switch (iMessage)
    {
        // Handle WM_NCCALCSIZE to remove the unwanted top border/padding
        // that automatically appears when applying the WS_THICKFRAME style.
        // Reference: https://discourse.glfw.org/t/making-a-custom-titlebar/2392/6
        case WM_NCCALCSIZE:
        {
            if (wParam == TRUE && lParam != NULL)
            {
                NCCALCSIZE_PARAMS* pParams = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam); // NOLINT

                // Fine-tune the client area coordinates to override and eliminate
                // the DWM's invisible resize margins and ghost padding artifacts.
                pParams->rgrc[0].top += 1;
                pParams->rgrc[0].right -= 2;
                pParams->rgrc[0].bottom -= 2;
                pParams->rgrc[0].left += 2;
            }
            // Return 0 to indicate that the entire window area should be treated
            // as the client area, effectively hiding the default OS frame container.
            return 0;
        }

        // Prevent the OS from painting its own standard title bar and window frame.
        // This is crucial to suppress default layout artifacts and ensure
        // only our custom ImGui title bar is rendered.
        case WM_NCPAINT:
        {
            return 0;
        }
        case WM_MOVE:
        {
            if (spMainWindow)
            {
                const int32_t newX = static_cast<int32_t>(LOWORD(lParam));
                const int32_t newY = static_cast<int32_t>(HIWORD(lParam));

                spMainWindow->SetPosX(newX);
                spMainWindow->SetPosY(newY);
            }
            return 0;
        }
        case WM_NCHITTEST:
        {
            const LRESULT hitResult = ::DefWindowProcW(hWnd, iMessage, wParam, lParam);

            const POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            RECT winRect;
            ::GetWindowRect(hWnd, &winRect);

            const int borderGap = 5;
            const bool bOnLeftBorder = (pt.x >= winRect.left && pt.x < winRect.left + borderGap);
            const bool bOnRightBorder = (pt.x >= winRect.right - borderGap && pt.x < winRect.right);
            const bool bOnTopBorder = (pt.y >= winRect.top && pt.y < winRect.top + borderGap);
            const bool bOnBottomBorder = (pt.y >= winRect.bottom - borderGap && pt.y < winRect.bottom);

            if (bOnLeftBorder && bOnTopBorder)
            {
                return HTTOPLEFT;
            }
            if (bOnRightBorder && bOnTopBorder)
            {
                return HTTOPRIGHT;
            }
            if (bOnLeftBorder && bOnBottomBorder)
            {
                return HTBOTTOMLEFT;
            }
            if (bOnRightBorder && bOnBottomBorder)
            {
                return HTBOTTOMRIGHT;
            }

            if (bOnLeftBorder)
            {
                return HTLEFT;
            }
            if (bOnRightBorder)
            {
                return HTRIGHT;
            }
            if (bOnTopBorder)
            {
                return HTTOP;
            }
            if (bOnBottomBorder)
            {
                return HTBOTTOM;
            }

            return hitResult;
        }
        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* pInfo = reinterpret_cast<MINMAXINFO*>(lParam); // NOLINT

            const int32_t titleBarHeight = spMainWindow ? spMainWindow->GetTitleBarHeight() : 0;
            const float minWidth = static_cast<float>(titleBarHeight) * 1.5f * 4.0f;
            pInfo->ptMinTrackSize.x = static_cast<LONG>(minWidth);
            pInfo->ptMinTrackSize.y = static_cast<LONG>(titleBarHeight);

            return 0;
        }
        case WM_SIZE:
        {
            if (spMainWindow)
            {
                if (wParam != SIZE_MINIMIZED)
                {
                    spMainWindow->Resize(LOWORD(lParam), HIWORD(lParam));
                }

                if (wParam == SIZE_MINIMIZED)
                {
                    spInstance->SetPaused(true);
                    spMainWindow->SetWindowMinimized(true);
                    spMainWindow->SetWindowMaximized(false);
                }
                else if (wParam == SIZE_MAXIMIZED)
                {
                    spInstance->SetPaused(false);
                    spMainWindow->SetWindowMinimized(false);
                    spMainWindow->SetWindowMaximized(true);
                }
                else if (wParam == SIZE_RESTORED)
                {
                    spInstance->SetPaused(false);

                    if (spMainWindow->IsWindowMinimized())
                    {
                        spMainWindow->SetWindowMinimized(false);
                    }
                    else if (spMainWindow->IsWindowMaximized())
                    {
                        spMainWindow->SetWindowMaximized(false);
                    }
                }
            }
            return 0;
        }
        case WM_ENTERSIZEMOVE:
        {
            if (spMainWindow)
            {
                spMainWindow->SetWindowResizing(true);
            }
            return 0;
        }
        case WM_EXITSIZEMOVE:
        {
            if (spMainWindow)
            {
                spMainWindow->SetWindowResizing(false);
            }
            return 0;
        }
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
            {
                return 0;
            }
            break;
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        case WM_DESTROY:
            return 0;
        default:
            break;
    }

    return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

} // namespace ho
