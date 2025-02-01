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

static void cameraForward() { g_camera.m_lastMove += g_camera.getForwardVecNoY(); }
static void cameraBack() { g_camera.m_lastMove -= g_camera.getForwardVecNoY(); }
static void cameraRight() { g_camera.m_lastMove += g_camera.m_right; }
static void cameraLeft() { g_camera.m_lastMove -= g_camera.m_right; }
static void cameraUp() { g_camera.m_lastMove += CAMERA_UP; }
static void cameraDown() { g_camera.m_lastMove -= CAMERA_UP; }

Camera g_camera {.m_pos {0, 0, -3}, .m_lastMove {}, .m_sens = 0.05f, .m_speed = 4.0f};
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

    g_mouse.rel.x = win.m_relMotionX;
    g_mouse.rel.y = win.m_relMotionY;

    V2 delta = (g_mouse.rel - g_mouse.prevRel) * g_camera.m_sens;
    g_mouse.prevRel = g_mouse.rel;

    g_camera.m_yaw -= delta.x;
    g_camera.m_pitch += delta.y;

    if (g_camera.m_pitch > 89.9f)
        g_camera.m_pitch = 89.9f;
    if (g_camera.m_pitch < -89.9f)
        g_camera.m_pitch = -89.9f;

    const M4 yawTrm = M4RotFrom(0, toRad(g_camera.m_yaw), 0);
    const M4 pitchTrm = M4RotFrom(toRad(g_camera.m_pitch), 0, 0);
    const M4 axisTrm = yawTrm * pitchTrm;

    const V3 right = V3Norm((axisTrm * V4From(CAMERA_RIGHT, 0.0f)).xyz);
    const V3 up = V3Norm((axisTrm * V4From(CAMERA_UP, 0.0f)).xyz);
    const V3 lookAt = V3Norm((axisTrm * V4From(CAMERA_FRONT, 0.0f)).xyz);

    const M4 viewTrm {
        right.x, up.x, lookAt.x, 0,
        right.y, up.y, lookAt.y, 0,
        right.z, up.z, lookAt.z, 0,
        0,       0,    0,        1,
    };

    g_camera.m_front = lookAt;
    g_camera.m_right = right;
    g_camera.m_trm = viewTrm * g_camera.procMoveTRM();
}

void
procInput()
{
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
