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
}

void
Client::keyboardLeave(wl_keyboard* pKeyboard, uint32_t serial, wl_surface* pSurface)
{
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

void
Client::pointerEnter(
    wl_pointer* pPointer, uint32_t serial, wl_surface* pSurface, wl_fixed_t surfaceX,
    wl_fixed_t surfaceY
)
{
}

void
Client::pointerLeave(wl_pointer* pPointer, uint32_t serial, wl_surface* pSurface)
{
}

void
Client::pointerMotion(wl_pointer* pPointer, uint32_t time, wl_fixed_t surfaceX, wl_fixed_t surfaceY)
{
}

void
Client::pointerButton(wl_pointer* pPointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
}

void
Client::pointerAxis(wl_pointer* pPointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
}

void
Client::pointerFrame(wl_pointer* pPointer)
{
}

void
Client::pointerAxisSource(wl_pointer* pPointer, uint32_t axisSource)
{
}

void
Client::pointerAxisStop(wl_pointer* pPointer, uint32_t time, uint32_t axis)
{
}

void
Client::pointerAxisDiscrete(wl_pointer* pPointer, uint32_t axis, int32_t discrete)
{
}

void
Client::pointerAxisValue120(wl_pointer* pPointer, uint32_t axis, int32_t value120)
{
}

void
Client::pointerAxisRelativeDirection(wl_pointer* pPointer, uint32_t axis, uint32_t direction)
{
}

} /* namespace platform::wayland */
