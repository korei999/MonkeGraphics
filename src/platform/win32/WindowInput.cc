#include "Window.hh"

#include "control.hh"
#include "keys.hh"

using namespace adt;

namespace platform::win32
{

static const int s_aAsciiToLinuxKeyCodes[300] {
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
    switch (s_aAsciiToLinuxKeyCodes[keyCode])
    {
        case KEY_LEFTSHIFT:
        if (bDown) control::g_ePressedMods |= MOD_STATE::SHIFT;
        else control::g_ePressedMods &= ~MOD_STATE::SHIFT;
        break;

        case KEY_LEFTALT:
        if (bDown) control::g_ePressedMods |= MOD_STATE::ALT;
        else control::g_ePressedMods &= ~MOD_STATE::ALT;
        break;

        case KEY_LEFTCTRL:
        if (bDown) control::g_ePressedMods |= MOD_STATE::CTRL;
        else control::g_ePressedMods &= ~MOD_STATE::CTRL;
        break;
    }
}

void
Window::procKey(WPARAM keyCode, bool bDown)
{
    ADT_ASSERT(keyCode < utils::size(s_aAsciiToLinuxKeyCodes), " ");
    control::g_abPressed[ s_aAsciiToLinuxKeyCodes[keyCode] ] = bDown;
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

} /* namespace platform::win32 */
