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
static void toggleVSync() { app::window().toggleVSync(); }

static void cameraForward() { g_camera.m_lastMove += g_camera.getForwardVecNoY(); }
static void cameraBack() { g_camera.m_lastMove -= g_camera.getForwardVecNoY(); }
static void cameraRight() { g_camera.m_lastMove += g_camera.m_right; }
static void cameraLeft() { g_camera.m_lastMove -= g_camera.m_right; }
static void cameraUp() { g_camera.m_lastMove += CAMERA_UP; }
static void cameraDown() { g_camera.m_lastMove -= CAMERA_UP; }
static void cameraBoost() { g_camera.m_lastBoost *= 2.0f; }
static void cameraDeboost() { g_camera.m_lastBoost *= 0.5f; }

Camera g_camera {.m_pos {0, 0, -3}, .m_lastMove {}, .m_sens = 0.05f, .m_speed = 4.0f};
Mouse g_mouse {};
bool g_abPrevPressed[MAX_KEY_VALUE] {};
bool g_abPressed[MAX_KEY_VALUE] {};
MOD_STATE g_ePressedMods {};

Arr<Keybind, MAX_KEYBINDS> g_aKeybinds {
    {REPEAT::ONCE,       EXEC_ON::PRESS,   MOD_STATE::NONE,  KEY_F,        toggleFullscreen     },
    {REPEAT::ONCE,       EXEC_ON::PRESS,   MOD_STATE::NONE,  KEY_R,        toggleRelativePointer},
    {REPEAT::ONCE,       EXEC_ON::PRESS,   MOD_STATE::NONE,  KEY_V,        toggleVSync          },
    {REPEAT::ONCE,       EXEC_ON::RELEASE, MOD_STATE::NONE,  KEY_Q,        quit                 },
    {REPEAT::ONCE,       EXEC_ON::PRESS,   MOD_STATE::NONE,  KEY_ESC,      quit                 },

    {REPEAT::WHILE_DOWN, EXEC_ON::PRESS,   MOD_STATE::NONE,  KEY_W,        cameraForward        },
    {REPEAT::WHILE_DOWN, EXEC_ON::PRESS,   MOD_STATE::SHIFT, 0,            cameraBoost          },
    {REPEAT::WHILE_DOWN, EXEC_ON::PRESS,   MOD_STATE::ALT,   0,            cameraDeboost        },
    {REPEAT::WHILE_DOWN, EXEC_ON::PRESS,   MOD_STATE::NONE,  KEY_S,        cameraBack           },
    {REPEAT::WHILE_DOWN, EXEC_ON::PRESS,   MOD_STATE::NONE,  KEY_A,        cameraLeft           },
    {REPEAT::WHILE_DOWN, EXEC_ON::PRESS,   MOD_STATE::NONE,  KEY_D,        cameraRight          },
    {REPEAT::WHILE_DOWN, EXEC_ON::PRESS,   MOD_STATE::NONE,  KEY_SPACE,    cameraUp             },
    {REPEAT::WHILE_DOWN, EXEC_ON::PRESS,   MOD_STATE::NONE,  KEY_LEFTCTRL, cameraDown           },
};

static void
procKeybinds(Arr<bool, MAX_KEYBINDS>* paPressOnceMap, const Arr<Keybind, MAX_KEYBINDS>& aCommands)
{
    for (auto& com : aCommands)
    {
        ssize idx = &com - &aCommands[0];

        bool bKey {};
        bool bMod {};

        if (com.eExecOn == EXEC_ON::PRESS)
        {
            bKey = com.key == 0 ? true : g_abPressed[com.key];
            bMod = com.eMod == MOD_STATE::NONE ? true : static_cast<bool>(com.eMod & g_ePressedMods);
        }
        else
        {
            bKey = g_abPrevPressed[com.key] && !g_abPressed[com.key];
            /* NOTE: not using `ePrevMods` */
            bMod = com.eMod == MOD_STATE::NONE ? true : com.eMod == g_ePressedMods;
        }

        if (bKey && bMod)
        {
            if (com.eRepeat == REPEAT::WHILE_DOWN)
            {
                com.pfn();
            }
            else
            {
                if (!paPressOnceMap->operator[](idx))
                {
                    paPressOnceMap->operator[](idx) = true;
                    com.pfn();
                }
            }
        }
        else
        {
            paPressOnceMap->operator[](idx) = false;
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

    g_camera.m_yaw += delta.x;
    g_camera.m_pitch += delta.y;

    if (g_camera.m_pitch > 89.9f)
        g_camera.m_pitch = 89.9f;
    if (g_camera.m_pitch < -89.9f)
        g_camera.m_pitch = -89.9f;

    const M4 yawTrm = M4RotFrom(0, toRad(-g_camera.m_yaw), 0);
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
    g_camera.m_view = viewTrm;
}

void
procInput()
{
    g_camera.m_lastMove = {};
    g_camera.m_lastBoost = 1.0f;

    ADT_ASSERT(sizeof(g_abPrevPressed) == sizeof(g_abPressed),
        "must be same size: %llu, %llu",
        static_cast<u64>(sizeof(g_abPrevPressed)), static_cast<u64>(sizeof(g_abPressed))
    );

    defer( utils::copy(g_abPrevPressed, g_abPressed, utils::size(g_abPrevPressed)) );

    static Arr<bool, MAX_KEYBINDS> s_aPressedKeysOnceMap(MAX_KEYBINDS);
    procKeybinds(&s_aPressedKeysOnceMap, g_aKeybinds);

    procMouse();
}

} /* namespace control */
