#include "Window.hh"

#include "control.hh"
#include "keys.hh"

#include "adt/logs.hh"
#include "render/gl/glfunc.hh" /* IWYU pragma: keep */

#include <windowsx.h>

using namespace adt;

namespace platform::win32
{

static const int s_mapAsciiToLinuxKeyCode[300] {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    KEY_TAB,
    0,
    0,
    0,
    0,
    KEY_LEFTSHIFT,
    KEY_LEFTCTRL,
    KEY_LEFTALT,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    KEY_ESC,
    0,
    0,
    0,
    0,
    KEY_SPACE,
    KEY_1,
    KEY_APOSTROPHE,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_7,
    KEY_APOSTROPHE,
    KEY_9,
    KEY_0,
    KEY_8,
    KEY_EQUAL,
    KEY_COMMA,
    KEY_MINUS,
    KEY_DOT,
    KEY_SLASH,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_SEMICOLON,
    KEY_SEMICOLON,
    KEY_COMMA,
    KEY_EQUAL,
    KEY_DOT,
    KEY_SLASH,
    KEY_2,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_O,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_LEFTBRACE,
    KEY_BACKSLASH,
    KEY_RIGHTBRACE,
    KEY_6,
    KEY_MINUS,
    KEY_GRAVE,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_O,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_LEFTBRACE,
    KEY_BACKSLASH,
    KEY_RIGHTBRACE,
    KEY_GRAVE,
    KEY_DELETE,
};

static void
setMods(WPARAM keyCode, bool bDown)
{
    switch (s_mapAsciiToLinuxKeyCode[keyCode])
    {
        case KEY_LEFTSHIFT:
        {
            if (bDown) control::g_ePressedMods |= MOD_STATE::SHIFT;
            else control::g_ePressedMods &= ~MOD_STATE::SHIFT;
        }
        break;

        case KEY_LEFTALT:
        {
            if (bDown) control::g_ePressedMods |= MOD_STATE::ALT;
            else control::g_ePressedMods &= ~MOD_STATE::ALT;
        }
        break;

        case KEY_LEFTCTRL:
        {
            if (bDown) control::g_ePressedMods |= MOD_STATE::CTRL;
            else control::g_ePressedMods &= ~MOD_STATE::CTRL;
        }
        break;
    }
}

void
Window::procKey(WPARAM keyCode, bool bDown)
{
    ADT_ASSERT(keyCode < utils::size(s_mapAsciiToLinuxKeyCode), " ");
    control::g_abPressed[ s_mapAsciiToLinuxKeyCode[keyCode] ] = bDown;
    setMods(keyCode, bDown);
}

/* https://gist.github.com/luluco250/ac79d72a734295f167851ffdb36d77ee */
void
Window::registerRawMouseDevice(bool bOn)
{
    DWORD flag = bOn ? RIDEV_NOLEGACY : RIDEV_REMOVE;

    m_rawInputDevices[0].usUsagePage = 0x01; /* HID_USAGE_PAGE_GENERIC */
    m_rawInputDevices[0].usUsage = 0x02;     /* HID_USAGE_GENERIC_MOUSE */
    m_rawInputDevices[0].dwFlags = flag;     /* adds mouse and also ignores legacy mouse messages */
    m_rawInputDevices[0].hwndTarget = 0;

    // pApp->rawInputDevices[1].usUsagePage = 0x01;       /* HID_USAGE_PAGE_GENERIC */
    // pApp->rawInputDevices[1].usUsage = 0x06;           /* HID_USAGE_GENERIC_KEYBOARD */
    // pApp->rawInputDevices[1].dwFlags = RIDEV_NOLEGACY; /* adds keyboard and also ignores legacy keyboard messages */
    // pApp->rawInputDevices[1].hwndTarget = 0;

    ADT_ASSERT_ALWAYS(
        RegisterRawInputDevices(
            m_rawInputDevices, 1, sizeof(m_rawInputDevices[0])),
        "RegisterRawInputDevices failed: %lu\n", GetLastError()
    );
}

bool
Window::enterFullscreen(HWND hwnd, int fullscreenWidth, int fullscreenHeight, int colorBits, int refreshRate)
{
    DEVMODE fullscreenSettings {};
    bool bSucces {};

    EnumDisplaySettings(NULL, 0, &fullscreenSettings);
    fullscreenSettings.dmPelsWidth = fullscreenWidth;
    fullscreenSettings.dmPelsHeight = fullscreenHeight;
    fullscreenSettings.dmBitsPerPel = colorBits;
    fullscreenSettings.dmDisplayFrequency = refreshRate;
    fullscreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

    SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
    SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, fullscreenWidth, fullscreenHeight, SWP_SHOWWINDOW);
    bSucces = ChangeDisplaySettings(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
    ShowWindow(hwnd, SW_MAXIMIZE);

    return bSucces;
}

bool
Window::exitFullscreen(HWND hwnd, int windowX, int windowY, int windowedWidth, int windowedHeight, int windowedPaddingX, int windowedPaddingY)
{
    bool bSucces {};

    SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_LEFT);
    SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    bSucces = ChangeDisplaySettings(NULL, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;
    SetWindowPos(hwnd, HWND_NOTOPMOST, windowX, windowY, windowedWidth + windowedPaddingX, windowedHeight + windowedPaddingY, SWP_SHOWWINDOW);
    ShowWindow(hwnd, SW_RESTORE);

    return bSucces;
}

LRESULT CALLBACK
Window::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* s = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg)
    {
        case WM_DESTROY:
        s->m_bRunning = false;
        return 0;

        case WM_SIZE:
        {
            s->m_winWidth = LOWORD(lParam);
            s->m_winHeight = HIWORD(lParam);
            LOG("WM_SIZE: [{}, {}]\n", s->m_winWidth, s->m_winHeight);
            glViewport(0, 0, s->m_winWidth, s->m_winHeight);
        }
        break;

        case WM_KILLFOCUS:
        {
            memset(control::g_abPressed, 0, sizeof(control::g_abPressed));
            ShowWindow(s->m_hWindow, SW_MINIMIZE);
        }
        break;

        case WM_NCCREATE:
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
        break;

        case WM_LBUTTONDOWN:
        control::g_abPressed[BTN_LEFT] = true;
        break;

        case WM_LBUTTONUP:
        control::g_abPressed[BTN_LEFT] = false;
        break;

        case WM_RBUTTONDOWN:
        control::g_abPressed[BTN_RIGHT] = true;
        break;

        case WM_RBUTTONUP:
        control::g_abPressed[BTN_RIGHT] = false;
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
            s->m_pointerSurfaceX = static_cast<f32>(GET_X_LPARAM(lParam));
            s->m_pointerSurfaceY = static_cast<f32>(GET_Y_LPARAM(lParam));
        }
        break;

        case WM_MBUTTONDOWN:
        {
            /* middle click */
        }
        break;

        case WM_MOUSEWHEEL:
        {
            /* wayland uses doubles and one scroll is +- 15, here its short and +- 120 */
            short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            s->m_qWheelEvents.forcePushBack(zDelta);
        }
        break;

        case WM_INPUT:
        {
            u32 size = sizeof(RAWINPUT);
            static RAWINPUT raw[sizeof(RAWINPUT)] {};
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));

            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                s->m_relMotionX += static_cast<f32>(raw->data.mouse.lLastX);
                s->m_relMotionY += static_cast<f32>(raw->data.mouse.lLastY);
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

} /* namespace platform::win32 */
