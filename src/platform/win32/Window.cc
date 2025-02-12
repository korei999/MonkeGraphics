#include "Window.hh"

#include "platform/glfunc.hh"
#include "platform/win32/wglext.h"

#include "control.hh"

#include "adt/logs.hh"

#include <windowsx.h>


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
    auto loadFunc = [&](auto& pfn, const char* ntsFnName) {
        void* p = (void*)wglGetProcAddress(ntsFnName);

        if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
        {
            HMODULE module = LoadLibraryA("opengl32.dll");
            p = (void*)GetProcAddress(module, ntsFnName);
        }

        ADT_ASSERT_ALWAYS(p, "wglGetProcAddress() failed: pfn: %p", p);

        pfn = reinterpret_cast<decltype(pfn)>(p);
    };

#define LOAD_GL_FUNC(NAME) loadFunc(NAME, #NAME)

    LOAD_GL_FUNC(glActiveTexture);
    LOAD_GL_FUNC(glUseProgram);
    LOAD_GL_FUNC(glGetUniformLocation);
    LOAD_GL_FUNC(glUniformMatrix3fv);
    LOAD_GL_FUNC(glUniformMatrix4fv);
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
    LOAD_GL_FUNC(glEnableVertexAttribArray);
    LOAD_GL_FUNC(glDebugMessageCallback);

#undef LOAD_GL_FUNC
}

static LRESULT CALLBACK
windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* s = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg)
    {
        case WM_DESTROY:
            s->m_bRunning = false;
            return 0;

        case WM_SIZE:
            s->m_winHeight = LOWORD(lParam);
            s->m_winWidth = HIWORD(lParam);
            break;

        case WM_KILLFOCUS:
            memset(control::g_abPressed, 0, sizeof(control::g_abPressed));
            break;

        case WM_NCCREATE:
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
            break;

        case WM_LBUTTONDOWN:
            break;

        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_KEYDOWN:
            {
                WPARAM keyCode = wParam;
                bool bWasDown = ((lParam & (1 << 30)) != 0);
                bool bDown = ((lParam >> 31) & 1) == 0;

                if (bWasDown == bDown)
                    break;

                s->procKey(keyCode, bDown);
            }
            break;

        case WM_MOUSEMOVE:
            {
                control::g_mouse.abs.x = GET_X_LPARAM(lParam);
                control::g_mouse.abs.y = GET_Y_LPARAM(lParam);
            }
            break;

        case WM_INPUT:
            {
                u32 size = sizeof(RAWINPUT);
                static RAWINPUT raw[sizeof(RAWINPUT)] {};
                GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));

                if (raw->header.dwType == RIM_TYPEMOUSE)
                {
                    control::g_mouse.rel.x += raw->data.mouse.lLastX;
                    control::g_mouse.rel.y += raw->data.mouse.lLastY;
                }
            }
            break;

        default:
            break;
    }

    if (s && s->m_bPointerRelativeMode)
    {
        RECT r;
        GetWindowRect(s->m_hWindow, &r);

        SetCursorPos(
            s->m_winWidth / 2 + r.left,
            s->m_winHeight / 2 + r.top
        );
        SetCursor(nullptr);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

Window::Window(adt::IAllocator* pAlloc, const char* ntsName)
    : IWindow(pAlloc, ntsName)
{
    loadWGLFunctions();
}

void
Window::start(int width, int height)
{
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
    if (!atom) LOG_FATAL("RegisterClassExW failed\n");

    DWORD exstyle = WS_EX_APPWINDOW;
    DWORD style = WS_OVERLAPPEDWINDOW;

    // style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rect = { 0, 0, width, height };
    AdjustWindowRectEx(&rect, style, false, exstyle);
    m_width = rect.right - rect.left;
    m_height = rect.bottom - rect.top;

    static wchar_t s_aBuff[128] {};
    mbtowc(s_aBuff, m_ntsName, 0);

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

    if (!m_hWindow) LOG_FATAL("CreateWindowExW failed\n");

    m_hDeviceContext = GetDC(m_hWindow);
    if (!m_hDeviceContext) LOG_FATAL("GetDC failed\n");

    /* FIXME: find better way to toggle this on startup */
    // input::registerRawMouseDevice(this, true);

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
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
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
    ADT_ASSERT_ALWAYS(m_hGlContext, "Cannot create OpenGL context! OpenGL version 4.5 is not supported");

    bool okContext = wglMakeCurrent(m_hDeviceContext, m_hGlContext);
    ADT_ASSERT_ALWAYS(okContext, "wglMakeCurrent() failed");

    loadGLFunctions();

    unbindContext();

    m_vDepthBuffer.setSize(m_pAlloc, m_stride * m_height);
    m_vSurfaceBuffer.setSize(m_pAlloc, m_stride * m_height);

    m_bPointerRelativeMode = false;
    m_bPaused = false;
    m_bRunning = true;

    LOG_GOOD("window started...\n");
}

void
Window::disableRelativeMode()
{
    m_bPointerRelativeMode = 0;
}

void
Window::enableRelativeMode()
{
    m_bPointerRelativeMode = 1;
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
}

void
Window::hideCursor(bool bHide)
{
}

void
Window::setCursorImage(adt::String cursorType)
{
}

void
Window::setFullscreen()
{
}

void
Window::unsetFullscreen()
{
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

Span2D<ImagePixelRGBA>
Window::surfaceBuffer()
{
    return {reinterpret_cast<ImagePixelRGBA*>(m_vSurfaceBuffer.data()), m_width, m_height, m_stride};
}

void
Window::scheduleFrame()
{
}

} /* namespace platform::win32 */
