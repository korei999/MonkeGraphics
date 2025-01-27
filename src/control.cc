#include "control.hh"

#include "frame.hh"
#include "keys.hh"
#include "app.hh"

#include "adt/logs.hh"

using namespace adt;

namespace control
{

static void toggleFullscreen() { app::window().toggleFullscreen(); }
static void quit() { app::window().m_bRunning = false; }
static void cameraForward() { g_camera.lastMove += Camera::front*frame::g_dt*SPEED; }
static void cameraBack() { g_camera.lastMove -= Camera::front*frame::g_dt*SPEED; }
static void cameraRight() { g_camera.lastMove += Camera::right*frame::g_dt*SPEED; }
static void cameraLeft() { g_camera.lastMove -= Camera::right*frame::g_dt*SPEED; }
static void cameraUp() { g_camera.lastMove += Camera::up*frame::g_dt*SPEED; }
static void cameraDown() { g_camera.lastMove -= Camera::up*frame::g_dt*SPEED; }

Camera g_camera {.pos {0, 0, -3}, .lastMove {}};
bool g_aPressed[MAX_KEY_VALUE] {};

Arr<Keybind, MAX_KEYBINDS> g_aKeybinds {
    {ONCE, KEY_F, toggleFullscreen},
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
        if (idx >= paMap->getSize())
        {
            LOG_BAD("command array size is bigger than {}, skipping the rest\n", paMap->getSize());
            return;
        }

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

void
procKeys()
{
    g_camera.lastMove = {};

    {
        static Arr<bool, MAX_KEYBINDS> aPressedKeysOnceMap(MAX_KEYBINDS);
        procKeybinds(&aPressedKeysOnceMap, g_aKeybinds);
    }

    {
        static Arr<bool, MAX_KEYBINDS> aPressedKeysOnceMap(MAX_KEYBINDS);
        procKeybinds(&aPressedKeysOnceMap, g_aModbinds);
    }
}

} /* namespace control */
