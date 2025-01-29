#include "control.hh"

#include "keys.hh"
#include "app.hh"

#include "adt/defer.hh"

using namespace adt;

namespace control
{

static void toggleFullscreen() { app::window().toggleFullscreen(); }
static void quit() { app::window().m_bRunning = false; }
static void toggleRelativePointer() { app::window().togglePointerRelativeMode(); }

static void cameraForward() { g_camera.m_lastMove += CAMERA_FRONT; }
static void cameraBack() { g_camera.m_lastMove -= CAMERA_FRONT; }
static void cameraRight() { g_camera.m_lastMove += CAMERA_RIGHT; }
static void cameraLeft() { g_camera.m_lastMove -= CAMERA_RIGHT; }
static void cameraUp() { g_camera.m_lastMove += CAMERA_UP; }
static void cameraDown() { g_camera.m_lastMove -= CAMERA_UP; }

Camera g_camera {.m_pos {0, 0, -3}, .m_lastMove {}};
Mouse g_mouse {};
bool g_aPrevPressed[MAX_KEY_VALUE] {};
bool g_aPressed[MAX_KEY_VALUE] {};

Arr<Keybind, MAX_KEYBINDS> g_aKeybinds {
    {ONCE, KEY_F, toggleFullscreen},
    {ONCE, KEY_R, toggleRelativePointer},
    {ONCE, KEY_Q, quit},
    {ONCE, KEY_ESC, quit},
    {REPEAT, KEY_W, cameraForward},
    {REPEAT, KEY_S, cameraBack},
    {REPEAT, KEY_A, cameraLeft},
    {REPEAT, KEY_D, cameraRight},
    {REPEAT, KEY_SPACE, cameraUp},
    {REPEAT, KEY_LEFTCTRL, cameraDown},
};

Arr<Keybind, MAX_KEYBINDS> g_aModbinds {
};

/* TODO: exec on press/release option */
static void
procKeybinds(Arr<bool, MAX_KEYBINDS>* paMap, const Arr<Keybind, MAX_KEYBINDS>& aCommands)
{
    for (auto& com : aCommands)
    {
        ssize idx = &com - &aCommands[0];

        if (g_aPressed[com.key])
        {
            if (com.eRepeat == REPEAT_KEY::REPEAT)
            {
                com.pfn();
            }
            else
            {
                if (!(*paMap)[idx])
                {
                    (*paMap)[idx] = true;
                    com.pfn();
                }
            }
        }
        else
        {
            (*paMap)[idx] = false;
        }
    }
}

static void
procMouse()
{
    using namespace adt::math;

    const auto& win = app::window();

    g_mouse.abs.x = win.m_pointerSurfaceX / static_cast<f32>(win.m_winWidth);
    g_mouse.abs.y = win.m_pointerSurfaceY / static_cast<f32>(win.m_winHeight);

    if (g_aPressed[BTN_LEFT])
    {
        if (!g_aPrevPressed[BTN_LEFT])
        {
            g_mouse.prevAbs = g_mouse.abs;
        }

        V2 delta = g_mouse.abs - g_mouse.prevAbs;

        g_camera.m_pitch += delta.y;
        g_camera.m_yaw += delta.x;

        g_mouse.prevAbs = g_mouse.abs;
    }

    M4 yawTrm = M4RotFrom(0, g_camera.m_yaw, 0);
    M4 pitchTrm = M4RotFrom(g_camera.m_pitch, 0, 0);
    M4 axisTrm = yawTrm * pitchTrm;

    V3 right = V3Norm((axisTrm * V4{1, 0, 0, 0}).xyz);
    V3 up = V3Norm((axisTrm * V4{0, 1, 0, 0}).xyz);
    V3 lookAt = V3Norm((axisTrm * V4{0, 0, 1, 0}).xyz);

    M4 viewTrm = M4Iden();
    viewTrm.v[0].x = right.x;
    viewTrm.v[1].x = right.y;
    viewTrm.v[2].x = right.z;

    viewTrm.v[0].y = up.x;
    viewTrm.v[1].y = up.y;
    viewTrm.v[2].y = up.z;

    viewTrm.v[0].z = lookAt.x;
    viewTrm.v[1].z = lookAt.y;
    viewTrm.v[2].z = lookAt.z;

    g_camera.m_trm = viewTrm * g_camera.getMoveTRM();
}

void
procKeys()
{
    using namespace adt::math;

    ADT_ASSERT(sizeof(g_aPrevPressed) == sizeof(g_aPressed),
        "must be same size: %llu, %llu",
        static_cast<u64>(sizeof(g_aPrevPressed)), static_cast<u64>(sizeof(g_aPressed))
    );

    defer( utils::copy(g_aPrevPressed, g_aPressed, utils::size(g_aPrevPressed)) );

    {
        static Arr<bool, MAX_KEYBINDS> aPressedKeysOnceMap(MAX_KEYBINDS);
        procKeybinds(&aPressedKeysOnceMap, g_aKeybinds);
    }

    {
        static Arr<bool, MAX_KEYBINDS> aPressedKeysOnceMap(MAX_KEYBINDS);
        procKeybinds(&aPressedKeysOnceMap, g_aModbinds);
    }

    procMouse();
}

} /* namespace control */
