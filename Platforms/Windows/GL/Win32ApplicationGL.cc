#include "Win32ApplicationGL.h"

#include <glad/glad.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>

#include "Win32WindowGL.h"

typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int* pAttribList);
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

namespace ho
{
void Win32ApplicationGL::CreateInstance(HINSTANCE hApp)
{
    HO_ASSERT(!spInstance, "Application was already created.");
    spInstance = new Win32ApplicationGL(hApp);
}

bool Win32ApplicationGL::Init(const std::wstring& mainWindowNameStr, int32_t mainWindowWidth, int32_t mainWindowHeight)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    const float mainScale =
        ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

    // Calc main window size
    RECT stWindowRect = {0, 0, static_cast<LONG>(mainWindowWidth), static_cast<LONG>(mainWindowHeight)};
    const DWORD dwStyle = WS_OVERLAPPEDWINDOW;

    AdjustWindowRect(&stWindowRect, dwStyle, FALSE);

    const int32_t finalWidth = stWindowRect.right - stWindowRect.left;
    const int32_t finalHeight = stWindowRect.bottom - stWindowRect.top;

    const int32_t scaledWidth = static_cast<int32_t>(static_cast<float>(finalWidth) * mainScale);
    const int32_t scaledHeight = static_cast<int32_t>(static_cast<float>(finalHeight) * mainScale);

    // Create GL context
    HWND hDummyWnd = CreateWindowW(L"STATIC",
                                   L"Dummy",
                                   dwStyle,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   scaledWidth,
                                   scaledHeight,
                                   nullptr,
                                   nullptr,
                                   mhApp,
                                   this);

    if (!hDummyWnd)
    {
        HO_ASSERT(false, "Failed to create main window.");
        return false;
    }

    HDC hDummyDC = ::GetDC(hDummyWnd);
    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pixelFormat = ::ChoosePixelFormat(hDummyDC, &pfd);
    HO_ASSERT(pixelFormat != 0, "Current system not supports such pixel format.");
    const BOOL bSuccess = ::SetPixelFormat(hDummyDC, pixelFormat, &pfd);
    HO_ASSERT(bSuccess != FALSE, "Failed to setting pixel format.");
    (void)bSuccess;

    HGLRC hDummyGLRC = wglCreateContext(hDummyDC);
    wglMakeCurrent(hDummyDC, hDummyGLRC);

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));

    wglMakeCurrent(hDummyDC, nullptr);
    wglDeleteContext(hDummyGLRC);

    HO_ASSERT(wglCreateContextAttribsARB != nullptr, "Failed to get address of context creation function.");

    int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,
                     4,
                     WGL_CONTEXT_MINOR_VERSION_ARB,
                     6,
                     WGL_CONTEXT_PROFILE_MASK_ARB,
                     WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                     0};

    shGlContext = wglCreateContextAttribsARB(hDummyDC, nullptr, attribs);

    HO_ASSERT(shGlContext != nullptr, "Failed to create GL context.");

    wglMakeCurrent(hDummyDC, shGlContext);

    if (!gladLoadGL())
    {
        HO_ASSERT(false, "Failed to load GL functions.");
        return false;
    }

    wglMakeCurrent(hDummyDC, nullptr);
    ::ReleaseDC(hDummyWnd, hDummyDC);

    if (!::DestroyWindow(hDummyWnd))
    {
        HO_ASSERT(false, "Failed to destroy dummy window.");
        return false;
    }

    // Create main window
    mMainWndNameStr = mainWindowNameStr;

    WNDCLASSEXW stWndClass{};
    stWndClass.cbSize = sizeof(WNDCLASSEXW);
    stWndClass.style = CS_OWNDC;
    stWndClass.lpfnWndProc = wndProc;
    stWndClass.hInstance = mhApp;
    stWndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    stWndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    stWndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    stWndClass.lpszClassName = mMainWndNameStr.c_str();
    stWndClass.hIconSm = static_cast<HICON>(LoadImage(mhApp,
                                                      MAKEINTRESOURCE(5),
                                                      IMAGE_ICON,
                                                      GetSystemMetrics(SM_CXSMICON),
                                                      GetSystemMetrics(SM_CYSMICON),
                                                      LR_DEFAULTCOLOR));

    if (!RegisterClassExW(&stWndClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        HO_ASSERT(false, "Failed to register main window class.");
        return false;
    }

    mhMainWnd = CreateWindowW(stWndClass.lpszClassName,
                              mMainWndNameStr.c_str(),
                              dwStyle,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              scaledWidth,
                              scaledHeight,
                              nullptr,
                              nullptr,
                              mhApp,
                              this);

    spMainWindow = std::make_unique<Win32WindowGL>(finalWidth, finalHeight, mhMainWnd, shGlContext);
    spMainWindow->ActivateContext();

    // Init imgui
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    if (!ImGui::CreateContext())
    {
        HO_ASSERT(false, "Failed to create imgui context.");
        return false;
    }
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
    if (!ImGui_ImplWin32_InitForOpenGL(mhMainWnd))
    {
        HO_ASSERT(false, "Failed to initialize imgui backend.");
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init())
    {
        HO_ASSERT(false, "Failed to initialize imgui render backend.");
        return false;
    }

    // Win32+GL needs specific hooks for viewport, as there are specific things needed to tie Win32 and GL api.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        HO_ASSERT(platform_io.Renderer_CreateWindow == nullptr, "");
        HO_ASSERT(platform_io.Renderer_DestroyWindow == nullptr, "");
        HO_ASSERT(platform_io.Renderer_SwapBuffers == nullptr, "");
        HO_ASSERT(platform_io.Platform_RenderWindow == nullptr, "");
        platform_io.Renderer_CreateWindow = Hook_CreateWindow;
        platform_io.Renderer_DestroyWindow = Hook_DestroyWindow;
        platform_io.Renderer_SwapBuffers = Hook_SwapBuffers;
        platform_io.Platform_RenderWindow = Hook_RenderWindow;
    }

    // Load and Merge fonts
    initImGuiFonts();

    // show main window
    ShowWindow(mhMainWnd, SW_SHOWDEFAULT);
    UpdateWindow(mhMainWnd);

    // It's to prevent imgui initialize GL in 'ImGui_ImplOpenGL3_NewFrame()'.
    ImGui_ImplOpenGL3_CreateDeviceObjects();

    spMainWindow->DeactivateContext();

    return true;
}

void Win32ApplicationGL::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Win32ApplicationGL::Shutdown()
{
    // 'ImGui_ImplOpenGL3_Shutdown()' calls GL APIs internally. So GL context must be brought back from render thread to
    // main thread.
    IPlatformApplication::GetInstance().GetMainWindow()->ActivateContext();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    spMainWindow.reset();
    if (!::DestroyWindow(mhMainWnd))
    {
        HO_ASSERT(false, "Failed to destroy main window.");
    }
    if (!::UnregisterClassW(mMainWndNameStr.c_str(), mhApp))
    {
        HO_ASSERT(false, "Failed to unregister main window class.");
    }

    if (!wglDeleteContext(shGlContext))
    {
        HO_ASSERT(false, "Failed to delete GL context.");
    }
}

void Win32ApplicationGL::Hook_CreateWindow(ImGuiViewport* viewport)
{
    HO_ASSERT(viewport->RendererUserData == nullptr, "");

    Win32WindowGL* pWindow = new Win32WindowGL(static_cast<int32_t>(viewport->Size.x),
                                               static_cast<int32_t>(viewport->Size.y),
                                               static_cast<HWND>(viewport->PlatformHandle),
                                               shGlContext);
    viewport->RendererUserData = static_cast<void*>(pWindow);
}

void Win32ApplicationGL::Hook_DestroyWindow(ImGuiViewport* viewport)
{
    if (viewport->RendererUserData != nullptr)
    {
        const Win32WindowGL* window = static_cast<Win32WindowGL*>(viewport->RendererUserData);
        delete window;
        viewport->RendererUserData = nullptr;
    }
}

void Win32ApplicationGL::Hook_RenderWindow(ImGuiViewport* viewport, void* unused)
{
    // Activate the platform window DC in the OpenGL rendering context
    if (const Win32WindowGL* pWindow = static_cast<const Win32WindowGL*>(viewport->RendererUserData))
    {
        pWindow->ActivateContext();
    }
    (void)unused;
}

void Win32ApplicationGL::Hook_SwapBuffers(ImGuiViewport* viewport, void* unused)
{
    if (const Win32WindowGL* pWindow = static_cast<const Win32WindowGL*>(viewport->RendererUserData))
    {
        pWindow->Present();
    }
    (void)unused;
}

Win32ApplicationGL::Win32ApplicationGL(HINSTANCE hApp)
  : IWin32Application(hApp)
{
}

HGLRC Win32ApplicationGL::shGlContext = nullptr;
} // namespace ho