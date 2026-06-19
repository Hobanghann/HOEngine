#include "IWin32Application.h"

#include <imgui_impl_win32.h>

#include "Core/IO/Path.h"
#include "Core/Templates/FixedArray.h"
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
{
}

bool IWin32Application::ShowOpenFileDialog(Path* pOutPaths,
                                           const std::wstring& titleStr,
                                           const std::wstring* pFilterNamesStr,
                                           const std::wstring* pFilterExtensionsStr,
                                           int32_t filterCount,
                                           const std::wstring& initialDirPathStr)
{
    if (!pOutPaths || !pFilterNamesStr || !pFilterExtensionsStr)
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
        pFileDialog->SetTitle(titleStr.c_str());
    }

    FixedArray<COMDLG_FILTERSPEC> filterSpecs(filterCount);

    for (int32_t i = 0; i < filterCount; ++i)
    {
        filterSpecs[i] = {pFilterNamesStr[i].c_str(), pFilterExtensionsStr[i].c_str()};
    }

    pFileDialog->SetFileTypes(filterSpecs.GetSize(), filterSpecs.Data());

    if (!initialDirPathStr.empty())
    {
        IShellItem* pInitialFolder = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(initialDirPathStr.c_str(), nullptr, IID_PPV_ARGS(&pInitialFolder))))
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
                *pOutPaths = Path(std::wstring(pFilePath));

                CoTaskMemFree(pFilePath);

                bSuccess = true;
            }
            pItem->Release();
        }
    }

    pFileDialog->Release();

    return bSuccess;
}

bool IWin32Application::ShowOpenFilesDialog(Path* pOutPaths,
                                            int32_t* pOutPathCount,
                                            int32_t maxOutPathCount,
                                            const std::wstring& titleStr,
                                            const std::wstring* pFilterNamesStr,
                                            const std::wstring* pFilterExtensionsStr,
                                            int32_t filterCount,
                                            const std::wstring& initialDirPathStr)
{
    if (!pOutPaths || !pOutPathCount || maxOutPathCount <= 0 || !pFilterNamesStr || !pFilterExtensionsStr)
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
        pFileDialog->SetTitle(titleStr.c_str());
    }

    FixedArray<COMDLG_FILTERSPEC> filterSpecs(filterCount);
    for (int32_t i = 0; i < filterCount; ++i)
    {
        filterSpecs[i] = {pFilterNamesStr[i].c_str(), pFilterExtensionsStr[i].c_str()};
    }
    pFileDialog->SetFileTypes(filterSpecs.GetSize(), filterSpecs.Data());

    if (!initialDirPathStr.empty())
    {
        IShellItem* pInitialFolder = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(initialDirPathStr.c_str(), nullptr, IID_PPV_ARGS(&pInitialFolder))))
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

            int32_t readCount = std::min(maxOutPathCount, static_cast<int32_t>(numSelected));

            for (int32_t i = 0; i < readCount; ++i)
            {
                IShellItem* pItem = nullptr;
                if (SUCCEEDED(pResults->GetItemAt(i, &pItem)))
                {
                    PWSTR pFilePath = nullptr;
                    if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath)))
                    {
                        pOutPaths[i] = Path(std::wstring(pFilePath));
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
                if (spMainWindow)
                {
                    spMainWindow->Resize(LOWORD(lParam), HIWORD(lParam));
                }
            }
            return 0;
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
