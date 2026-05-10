#include "Win32ApplicationGL.h"

#include <glad/glad.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>

namespace ho
{
void Win32ApplicationGL::CreateInstance(HINSTANCE hApp)
{
    HO_ASSERT(!spInstance, "Application was already created.");
    spInstance = new Win32ApplicationGL(hApp);
}

bool Win32ApplicationGL::Init(const std::wstring& mainWindowName, int32_t mainwWindowWidth, int32_t mainwWindowHeight)
{
    mMainWindowName = mainWindowName;
    sMainWindowWidth = mainwWindowWidth;
    sMainWindowHeight = mainwWindowHeight;

    ImGui_ImplWin32_EnableDpiAwareness();
    const float mainScale =
        ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

    // Register window
    WNDCLASSEXW stWndClass{};
    stWndClass.cbSize = sizeof(WNDCLASSEXW);
    stWndClass.style = CS_OWNDC;
    stWndClass.lpfnWndProc = wndProc;
    stWndClass.hInstance = mhApp;
    stWndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    stWndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    stWndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    stWndClass.lpszClassName = mMainWindowName.c_str();
    stWndClass.hIconSm = static_cast<HICON>(LoadImage(mhApp,
                                                      MAKEINTRESOURCE(5),
                                                      IMAGE_ICON,
                                                      GetSystemMetrics(SM_CXSMICON),
                                                      GetSystemMetrics(SM_CYSMICON),
                                                      LR_DEFAULTCOLOR));

    if (!RegisterClassExW(&stWndClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        HO_ASSERT(false, "Failed to register window class.");
        return false;
    }

    // Calc window size
    RECT stWindowRect = {0, 0, static_cast<LONG>(mainwWindowWidth), static_cast<LONG>(mainwWindowHeight)};
    const DWORD dwStyle = WS_OVERLAPPEDWINDOW;

    AdjustWindowRect(&stWindowRect, dwStyle, FALSE);

    const int32_t finalWidth = stWindowRect.right - stWindowRect.left;
    const int32_t finalHeight = stWindowRect.bottom - stWindowRect.top;

    const int32_t scaledWidth = static_cast<int32_t>(static_cast<float>(finalWidth) * mainScale);
    const int32_t scaledHeight = static_cast<int32_t>(static_cast<float>(finalHeight) * mainScale);

    // Create window
    mhMainWnd = CreateWindowW(stWndClass.lpszClassName,
                              mMainWindowName.c_str(),
                              dwStyle,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              scaledWidth,
                              scaledHeight,
                              nullptr,
                              nullptr,
                              mhApp,
                              this);

    HO_ASSERT(mhMainWnd, "Failed to create window.");

    pMainWindow = std::make_unique<Win32WindowGL>(mhMainWnd, sMainWindowWidth, sMainWindowHeight);

    // Create GL context
    pMainWindow->MakeCurrent();
    gladLoadGL();

    // Init imgui
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(mainScale);
    style.FontScaleDpi = mainScale;
    io.ConfigDpiScaleFonts = true;
    io.ConfigDpiScaleViewports = true;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(mhMainWnd);
    ImGui_ImplOpenGL3_Init();

    // Win32+GL needs specific hooks for viewport, as there are specific things needed to tie Win32 and GL api.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        HO_ASSERT(platform_io.Renderer_CreateWindow == nullptr, "");
        HO_ASSERT(platform_io.Renderer_DestroyWindow == nullptr, "");
        HO_ASSERT(platform_io.Renderer_SwapBuffers == nullptr, "");
        HO_ASSERT(platform_io.Platform_RenderWindow == nullptr, "");
        platform_io.Renderer_CreateWindow = Win32WindowGL::Hook_CreateWindow;
        platform_io.Renderer_DestroyWindow = Win32WindowGL::Hook_DestroyWindow;
        platform_io.Renderer_SwapBuffers = Win32WindowGL::Hook_SwapBuffers;
        platform_io.Platform_RenderWindow = Win32WindowGL::Hook_RenderWindow;
    }

    // Load and Merge fonts
    initImGuiFonts();

    // show main window
    ShowWindow(mhMainWnd, SW_SHOWDEFAULT);
    UpdateWindow(mhMainWnd);

    return true;
}

bool Win32ApplicationGL::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    return true;
}

bool Win32ApplicationGL::EndFrame()
{
    const ImGuiIO& io = ImGui::GetIO();
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        // Restore the OpenGL rendering context to the main window DC, since platform windows might have changed it.
        pMainWindow->MakeCurrent();
    }

    // Present
    pMainWindow->Present();

    return true;
}

bool Win32ApplicationGL::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    pMainWindow.reset();
    Win32WindowGL::DeleteGLContext();
    ::DestroyWindow(mhMainWnd);
    ::UnregisterClassW(mMainWindowName.c_str(), mhApp);
    return true;
}

Win32ApplicationGL::Win32ApplicationGL(HINSTANCE hApp)
  : IWin32Application(hApp)
{
}
} // namespace ho