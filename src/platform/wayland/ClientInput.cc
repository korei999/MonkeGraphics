#include "Client.hh"

#include "adt/logs.hh"
#include "keys.hh"

namespace platform::wayland
{
void
Client::keyboardKeymap(wl_keyboard* pKeyboard, uint32_t format, int32_t fd, uint32_t size)
{
}

void
Client::keyboardEnter(wl_keyboard* pKeyboard, uint32_t serial, wl_surface* pSurface, wl_array* pKeys)
{
    LOG("keyboardEnter()\n");
}

void
Client::keyboardLeave(wl_keyboard* pKeyboard, uint32_t serial, wl_surface* pSurface)
{
    LOG("keyboardLeave()\n");
}

void
Client::keyboardKey(wl_keyboard* pKeyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    if (key == KEY_Q)
    {
        m_bRunning = false;
    }
    else if (key == KEY_F && state == WL_KEYBOARD_KEY_STATE_PRESSED)
    {
        toggleFullscreen();
    }
}

void
Client::keyboardModifiers(
    wl_keyboard* pKeyboard,
    uint32_t serial,
    uint32_t modsDepressed,
    uint32_t modsLatched,
    uint32_t modsLocked,
    uint32_t group
)
{
}

void
Client::keyboardRepeatInfo(wl_keyboard* pKeyboard, int32_t rate, int32_t delay)
{
}

} /* namespace platform::wayland */
