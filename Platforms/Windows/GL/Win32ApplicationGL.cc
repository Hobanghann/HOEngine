#include "Win32ApplicationGL.h"

#include <cmath>
#include <glad/glad.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <stb_image.h>

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

bool Win32ApplicationGL::Init(const std::string& iconPathStr)
{
    // Create GL context
    mhDummyWnd =
        CreateWindowW(L"STATIC", L"Dummy", 0, CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, nullptr, nullptr, mhApp, this);

    if (!mhDummyWnd)
    {
        HO_ASSERT(false, "Failed to create main window.");
        return false;
    }

    mhDummyDC = ::GetDC(mhDummyWnd);
    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pixelFormat = ::ChoosePixelFormat(mhDummyDC, &pfd);
    HO_ASSERT(pixelFormat != 0, "Current system not supports such pixel format.");
    const BOOL bSuccess = ::SetPixelFormat(mhDummyDC, pixelFormat, &pfd);
    HO_ASSERT(bSuccess != FALSE, "Failed to setting pixel format.");
    (void)bSuccess;

    HGLRC hDummyGLRC = wglCreateContext(mhDummyDC);
    wglMakeCurrent(mhDummyDC, hDummyGLRC);

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));

    wglMakeCurrent(mhDummyDC, nullptr);
    wglDeleteContext(hDummyGLRC);

    HO_ASSERT(wglCreateContextAttribsARB != nullptr, "Failed to get address of context creation function.");

    int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,
                     4,
                     WGL_CONTEXT_MINOR_VERSION_ARB,
                     6,
                     WGL_CONTEXT_PROFILE_MASK_ARB,
                     WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                     0};

    shGlContext = wglCreateContextAttribsARB(mhDummyDC, nullptr, attribs);

    HO_ASSERT(shGlContext != nullptr, "Failed to create GL context.");

    wglMakeCurrent(mhDummyDC, shGlContext);

    if (!gladLoadGL())
    {
        HO_ASSERT(false, "Failed to load GL functions.");
        return false;
    }

    // Load windows icon
    loadWindowsIcon(iconPathStr);

    // Upload icon texture
    uploadIconTexture(iconPathStr);

    // Init imgui
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    if (!ImGui::CreateContext())
    {
        HO_ASSERT(false, "Failed to create imgui context.");
        return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGui_ImplWin32_EnableDpiAwareness();
    const float mainScale =
        ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

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

    // Win32+GL needs specific hooks for viewport, as there are specific things needed to tie Win32 and GL api.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        HO_ASSERT(platform_io.Renderer_CreateWindow == nullptr, "");
        HO_ASSERT(platform_io.Renderer_DestroyWindow == nullptr, "");
        HO_ASSERT(platform_io.Renderer_SwapBuffers == nullptr, "");
        HO_ASSERT(platform_io.Platform_RenderWindow == nullptr, "");

        platform_io.Renderer_CreateWindow = RendererCreateWindow_Hook;
        platform_io.Renderer_DestroyWindow = RendererDestroyWindow_Hook;
        platform_io.Renderer_SwapBuffers = RendererSwapBuffers_Hook;
        platform_io.Platform_RenderWindow = PlatformRenderWindow_Hook;
    }

    return true;
}

bool Win32ApplicationGL::CreateMainWindow(const std::string& mainWindowTitleStr,
                                          int32_t titleBarHeight,
                                          int32_t mainWindowClientWidth,
                                          int32_t mainWindowClientHeight)
{
    // Initialize members
    mMainWndNameStr = mainWindowTitleStr;

    // Create window
    ImGui_ImplWin32_EnableDpiAwareness();
    const float mainScale =
        ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

    const DWORD dwStyle = WS_VISIBLE | WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    const int32_t totalClientHeight = mainWindowClientHeight + titleBarHeight;

    const int32_t scaledClientWidth = static_cast<int32_t>(static_cast<float>(mainWindowClientWidth) * mainScale);
    const int32_t scaledClientHeight = static_cast<int32_t>(static_cast<float>(totalClientHeight) * mainScale);

    RECT stWindowRect = {0, 0, static_cast<LONG>(scaledClientWidth), static_cast<LONG>(scaledClientHeight)};
    AdjustWindowRect(&stWindowRect, dwStyle, FALSE);

    stWindowRect.top = 0;

    const int32_t scaledWidth = stWindowRect.right - stWindowRect.left;
    const int32_t scaledHeight = stWindowRect.bottom - stWindowRect.top;

    const std::wstring mainWndNameStrUTF16 = toUTF16(mMainWndNameStr);

    WNDCLASSEXW stWndClass{};
    stWndClass.cbSize = sizeof(WNDCLASSEXW);
    stWndClass.style = CS_OWNDC;
    stWndClass.lpfnWndProc = wndProc;
    stWndClass.hInstance = mhApp;
    stWndClass.hIcon = mhIcon;
    stWndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    stWndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    stWndClass.lpszClassName = mainWndNameStrUTF16.c_str();
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
                              mainWndNameStrUTF16.c_str(),
                              dwStyle,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              scaledWidth,
                              scaledHeight,
                              nullptr,
                              nullptr,
                              mhApp,
                              this);
    if (mhMainWnd == nullptr)
    {
        HO_ASSERT(false, "Failed to create main window.");
        ::UnregisterClassW(mainWndNameStrUTF16.c_str(), mhApp);
        return false;
    }

    wglMakeCurrent(mhDummyDC, nullptr);
    ::ReleaseDC(mhDummyWnd, mhDummyDC);

    if (!::DestroyWindow(mhDummyWnd))
    {
        HO_ASSERT(false, "Failed to destroy dummy window.");
        return false;
    }

    RECT windowRect{};

    ::GetWindowRect(mhMainWnd, &windowRect);

    const int32_t posX = windowRect.left;
    const int32_t posY = windowRect.top;

    spMainWindow = std::make_unique<Win32WindowGL>(
        posX, posY, titleBarHeight, mainWindowClientWidth, mainWindowClientHeight, mhMainWnd, shGlContext);
    spMainWindow->SetTitle(std::string().assign(mMainWndNameStr.begin(), mMainWndNameStr.end()));
    spMainWindow->ActivateContext();

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
    const std::wstring mainWndNameStrUTF16 = toUTF16(mMainWndNameStr);
    if (!::UnregisterClassW(mainWndNameStrUTF16.c_str(), mhApp))
    {
        HO_ASSERT(false, "Failed to unregister main window class.");
    }
    ::DestroyIcon(mhIcon);

    if (!wglDeleteContext(shGlContext))
    {
        HO_ASSERT(false, "Failed to delete GL context.");
    }

    const GLuint glIcon = static_cast<GLuint>(reinterpret_cast<uintptr_t>(mIconNativeHandle));

    glDeleteTextures(1, &glIcon);

    mIconNativeHandle = nullptr;
}

void Win32ApplicationGL::uploadIconTexture(const std::string& path)
{
    // Load icon image
    int width = 0;
    int height = 0;
    int channels = 0;
    uint8_t* pIconBitmap = stbi_load(path.c_str(), &width, &height, &channels, 4);

    if (pIconBitmap == nullptr)
    {
        HO_ASSERT(false, "Failed to load icon image.");
        return;
    }

    // Load icon texture
    if (mIconNativeHandle != nullptr)
    {
        const GLuint oldTextureID = static_cast<GLuint>(reinterpret_cast<uintptr_t>(mIconNativeHandle));
        glDeleteTextures(1, &oldTextureID);
        mIconNativeHandle = nullptr;
        mIconWidth = 0;
        mIconHeight = 0;
    }
    const int mipLevels = 1 + static_cast<int>(std::floor(std::log2(std::max(width, height))));

    GLuint glIcon = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &glIcon);

    glTextureStorage2D(glIcon, mipLevels, GL_RGBA8, width, height);

    glTextureParameteri(glIcon, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(glIcon, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(glIcon, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(glIcon, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureSubImage2D(glIcon, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pIconBitmap);

    glGenerateTextureMipmap(glIcon);

    HO_ASSERT(glGetError() == GL_NO_ERROR, "Failed to upload icon texture.");

    mIconWidth = width;
    mIconHeight = height;
    mIconNativeHandle = reinterpret_cast<void*>(static_cast<uintptr_t>(glIcon)); // NOLINT

    stbi_image_free(pIconBitmap);
}

void Win32ApplicationGL::RendererCreateWindow_Hook(ImGuiViewport* viewport)
{
    HO_ASSERT(viewport->RendererUserData == nullptr, "");

    Win32WindowGL* pWindow = new Win32WindowGL(static_cast<int32_t>(viewport->Pos.x),
                                               static_cast<int32_t>(viewport->Pos.y),
                                               0,
                                               static_cast<int32_t>(viewport->Size.x),
                                               static_cast<int32_t>(viewport->Size.y),
                                               static_cast<HWND>(viewport->PlatformHandle),
                                               shGlContext);

    pWindow->SetTitle(viewport->GetDebugName());

    viewport->RendererUserData = static_cast<void*>(pWindow);
}

void Win32ApplicationGL::RendererDestroyWindow_Hook(ImGuiViewport* viewport)
{
    if (viewport->RendererUserData != nullptr)
    {
        const Win32WindowGL* window = static_cast<Win32WindowGL*>(viewport->RendererUserData);
        delete window;
        viewport->RendererUserData = nullptr;
    }
}

void Win32ApplicationGL::RendererSwapBuffers_Hook(ImGuiViewport* viewport, void* unused)
{
    if (const Win32WindowGL* pWindow = static_cast<const Win32WindowGL*>(viewport->RendererUserData))
    {
        pWindow->Present();
    }
    (void)unused;
}

void Win32ApplicationGL::PlatformRenderWindow_Hook(ImGuiViewport* viewport, void* unused)
{
    // Activate the platform window DC in the OpenGL rendering context
    if (const Win32WindowGL* pWindow = static_cast<const Win32WindowGL*>(viewport->RendererUserData))
    {
        pWindow->ActivateContext();
    }
    (void)unused;
}

Win32ApplicationGL::Win32ApplicationGL(HINSTANCE hApp)
  : IWin32Application(hApp)
  , mhDummyWnd(nullptr)
  , mhDummyDC(nullptr)
{
}

void (*Win32ApplicationGL::pOriginalPlatformCreateWindow_Hook)(ImGuiViewport*) = nullptr;

HGLRC Win32ApplicationGL::shGlContext = nullptr;
} // namespace ho