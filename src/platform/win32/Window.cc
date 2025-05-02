#include "Window.hh"

#include "render/gl/glfunc.hh"
#include "wglext.h"

#include "adt/logs.hh"

#include <clocale>
#include <windowsx.h>

#if defined __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#endif

using namespace adt;

static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB {};
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB {};
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT {};

namespace platform::win32
{

static void
loadWGLFunctions(void)
{
    /* to get WGL functions we need valid GL context, so create dummy window for dummy GL contetx */
    HWND dummy = CreateWindowExW(
        0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, nullptr, nullptr
    );
    ADT_ASSERT_ALWAYS(dummy, "CreateWindowExW() failed");

    HDC dc = GetDC(dummy);
    ADT_ASSERT_ALWAYS(dc, "GetDC() failed");

    PIXELFORMATDESCRIPTOR desc {};
    desc.nSize = sizeof(desc);
    desc.nVersion = 1;
    desc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desc.iPixelType = PFD_TYPE_RGBA;
    desc.cColorBits = 24;

    int format = ChoosePixelFormat(dc, &desc);
    ADT_ASSERT_ALWAYS(format, "Cannot choose OpenGL pixel format for dummy window");

    int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
    ADT_ASSERT_ALWAYS(ok, "DescribePixelFormat failed");

    /* reason to create dummy window is that SetPixelFormat can be called only once for the window */
    ok = SetPixelFormat(dc, format, &desc);
    ADT_ASSERT_ALWAYS(ok, "Cannot set OpenGL pixel format for dummy window");

    HGLRC rc = wglCreateContext(dc);
    ADT_ASSERT_ALWAYS(rc, "wglCreateContext failed");

    ok = wglMakeCurrent(dc, rc);
    ADT_ASSERT_ALWAYS(rc, "wglCreateContext failed");

    /* https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_extensions_string.txt */
    auto wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    ADT_ASSERT_ALWAYS(wglGetExtensionsStringARB, "OpenGL does not support WGL_ARB_extensions_string extension");

    const char* ext = wglGetExtensionsStringARB(dc);
    ADT_ASSERT_ALWAYS(ext, "wglGetExtensionsStringARB() failed");

    for (auto svWord : StringWordIt(ext, " "))
    {
        LOG("'{}'\n", svWord);

        if (svWord == "WGL_ARB_pixel_format")
            wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        else if (svWord == "WGL_ARB_create_context")
            wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        else if (svWord == "WGL_EXT_swap_control")
            wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    }

    ADT_ASSERT_ALWAYS(
        wglChoosePixelFormatARB && wglCreateContextAttribsARB && wglSwapIntervalEXT,
        "OpenGL does not support required WGL extensions context"
    );

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(rc);
    ReleaseDC(dummy, dc);
    DestroyWindow(dummy);
}

static void
loadGLFunctions()
{
    HMODULE module = LoadLibraryA("opengl32.dll");

    auto loadFunc = [&](auto& pfn, const char* ntsFnName) -> void
    {
        void* p = (void*)wglGetProcAddress(ntsFnName);

        if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
            p = (void*)GetProcAddress(module, ntsFnName);

        ADT_ASSERT_ALWAYS(p, "failed to load '{}' function", ntsFnName);

        pfn = reinterpret_cast<decltype(pfn)>(p);
    };

#define LOAD_GL_FUNC(NAME) loadFunc(NAME, #NAME)

    LOAD_GL_FUNC(glActiveTexture);
    LOAD_GL_FUNC(glUseProgram);
    LOAD_GL_FUNC(glGetUniformLocation);
    LOAD_GL_FUNC(glUniformMatrix3fv);
    LOAD_GL_FUNC(glUniformMatrix4fv);
    LOAD_GL_FUNC(glUniform2fv);
    LOAD_GL_FUNC(glUniform3fv);
    LOAD_GL_FUNC(glUniform4fv);
    LOAD_GL_FUNC(glUniform1iv);
    LOAD_GL_FUNC(glUniform1i);
    LOAD_GL_FUNC(glUniform1f);
    LOAD_GL_FUNC(glBindVertexArray);
    LOAD_GL_FUNC(glBindFramebuffer);
    LOAD_GL_FUNC(glCreateProgram);
    LOAD_GL_FUNC(glAttachShader);
    LOAD_GL_FUNC(glLinkProgram);
    LOAD_GL_FUNC(glGetProgramiv);
    LOAD_GL_FUNC(glGetProgramInfoLog);
    LOAD_GL_FUNC(glDeleteProgram);
    LOAD_GL_FUNC(glValidateProgram);
    LOAD_GL_FUNC(glDeleteShader);
    LOAD_GL_FUNC(glCreateShader);
    LOAD_GL_FUNC(glShaderSource);
    LOAD_GL_FUNC(glCompileShader);
    LOAD_GL_FUNC(glGetShaderiv);
    LOAD_GL_FUNC(glGetShaderInfoLog);
    LOAD_GL_FUNC(glGetActiveUniform);
    LOAD_GL_FUNC(glGenVertexArrays);
    LOAD_GL_FUNC(glGenBuffers);
    LOAD_GL_FUNC(glBindBuffer);
    LOAD_GL_FUNC(glBufferData);
    LOAD_GL_FUNC(glVertexAttribPointer);
    LOAD_GL_FUNC(glVertexAttribIPointer);
    LOAD_GL_FUNC(glEnableVertexAttribArray);
    LOAD_GL_FUNC(glBufferSubData);

    LOAD_GL_FUNC(glDebugMessageCallbackARB);

#undef LOAD_GL_FUNC
}

Window::Window(adt::IAllocator* pAlloc, const char* ntsName)
    : IWindow(pAlloc, ntsName)
{
    loadWGLFunctions();
}

void
Window::start(int width, int height)
{
    ADT_ASSERT_ALWAYS(setlocale(LC_ALL, "en_US.UTF-8"), " ");

    m_winWidth = m_width = width;
    m_winHeight = m_height = height;
    m_stride = m_width + 7; /* simd padding */

    m_windowClass = {};
    m_windowClass.cbSize = sizeof(m_windowClass);
    m_windowClass.lpfnWndProc = windowProc;
    m_windowClass.hInstance = m_hInstance;
    m_windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    m_windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    m_windowClass.lpszClassName = L"opengl_window_class";

    ATOM atom = RegisterClassExW(&m_windowClass);
    ADT_ASSERT_ALWAYS(atom, "RegisterClassExW() failed");


    DWORD exstyle = WS_EX_APPWINDOW;
    DWORD style = WS_OVERLAPPEDWINDOW;

    // style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rect = { 0, 0, width, height };
    AdjustWindowRectEx(&rect, style, false, exstyle);
    m_width = rect.right - rect.left;
    m_height = rect.bottom - rect.top;

    static wchar_t s_aBuff[128] {};
    mbstowcs(s_aBuff, m_ntsName, -1);

    m_hWindow = CreateWindowExW(exstyle,
        m_windowClass.lpszClassName,
        s_aBuff,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        m_width,
        m_height,
        nullptr,
        nullptr,
        m_windowClass.hInstance,
        this
    );

    ADT_ASSERT_ALWAYS(m_hWindow, "CreateWindowExW() failed");

    m_hDeviceContext = GetDC(m_hWindow);
    ADT_ASSERT_ALWAYS(m_hDeviceContext, "GetDC() failed");

    int attrib[] {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     24,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,

        WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,

        WGL_SAMPLE_BUFFERS_ARB, 1,
        WGL_SAMPLES_ARB,        4,
        0,
    };

    int format;
    UINT formats;
    int ok {};

    ok = wglChoosePixelFormatARB(m_hDeviceContext, attrib, nullptr, 1, &format, &formats);
    ADT_ASSERT_ALWAYS(ok && format, "OpenGL does not support required pixel format");

    PIXELFORMATDESCRIPTOR desc {};
    desc.nSize = sizeof(desc);
    ok = DescribePixelFormat(m_hDeviceContext, format, sizeof(desc), &desc);
    ADT_ASSERT_ALWAYS(ok, "DescribePixelFormat failed");

    ok = SetPixelFormat(m_hDeviceContext, format, &desc);
    ADT_ASSERT_ALWAYS(ok, "Cannot set OpenGL selected pixel format");

    int attribContext[] {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifndef NDEBUG
        // ask for debug context for non "Release" builds
        // this is so we can enable debug callback
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0,
    };

    m_hGlContext = wglCreateContextAttribsARB(m_hDeviceContext, nullptr, attribContext);
    ADT_ASSERT_ALWAYS(m_hGlContext, "Cannot create OpenGL context! OpenGL version 3.3 is not supported");

    bool okContext = wglMakeCurrent(m_hDeviceContext, m_hGlContext);
    ADT_ASSERT_ALWAYS(okContext, "wglMakeCurrent() failed");

    loadGLFunctions();

    unbindContext();

    m_bPointerRelativeMode = false;
    m_bPaused = false;
    m_bRunning = true;

    LOG_GOOD("window started...\n");
}

void
Window::disableRelativeMode()
{
    m_bPointerRelativeMode = 0;
    registerRawMouseDevice(false);
}

void
Window::enableRelativeMode()
{
    m_bPointerRelativeMode = 1;
    registerRawMouseDevice(true);
}

void
Window::togglePointerRelativeMode()
{
    m_bPointerRelativeMode == 0 ? enableRelativeMode() : disableRelativeMode();
    LOG_OK("relative mode: {}\n", m_bPointerRelativeMode);
}

void
Window::toggleFullscreen()
{
    m_bFullscreen == false ? setFullscreen() : unsetFullscreen();
}

void
Window::hideCursor([[maybe_unused]] bool bHide)
{
}

void
Window::setCursorImage([[maybe_unused]] adt::StringView svCursorType)
{
}

void
Window::setFullscreen()
{
    m_bFullscreen = true;

    enterFullscreen(
        m_hWindow,
        GetDeviceCaps(m_hDeviceContext, 0),
        GetDeviceCaps(m_hDeviceContext, 1),
        GetDeviceCaps(m_hDeviceContext, 2),
        GetDeviceCaps(m_hDeviceContext, 3)
    );
}

void
Window::unsetFullscreen()
{
    m_bFullscreen = false;

    exitFullscreen(m_hWindow, 0, 0, m_width, m_height, 0, 0);
}

void
Window::setSwapInterval(int interval)
{
    m_swapInterval = interval;
    wglSwapIntervalEXT(m_swapInterval);
    LOG_NOTIFY("swapInterval: {}\n", m_swapInterval);
}

void
Window::toggleVSync()
{
    m_swapInterval == 0 ? setSwapInterval(1) : setSwapInterval(0);
}

void
Window::swapBuffers()
{
    if (!SwapBuffers(m_hDeviceContext))
        LOG_WARN("SwapBuffers(dc): failed\n");
}

void
Window::procEvents()
{
    // MSG msg;
    // while (GetMessageA(&msg, nullptr, 0, 0))
    MSG msg;
    // WaitMessage();
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
            case WM_QUIT:
            m_bRunning = false;
            break;

            default:
            break;
        };

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void
Window::showWindow()
{
    ShowWindow(m_hWindow, SW_SHOWDEFAULT);
}

void
Window::destroy()
{
}

void
Window::bindContext()
{
    wglMakeCurrent(m_hDeviceContext, m_hGlContext);
}

void
Window::unbindContext()
{
    wglMakeCurrent(nullptr, nullptr);
}

} /* namespace platform::win32 */
