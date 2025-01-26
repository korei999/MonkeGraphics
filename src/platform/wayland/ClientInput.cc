#include "Client.hh"

#include "adt/logs.hh"
#include "keys.hh"

namespace platform::wayland
{

void
Client::keyboardKeymap(
    [[maybe_unused]] wl_keyboard* pKeyboard,
    [[maybe_unused]] uint32_t format,
    [[maybe_unused]] int32_t fd,
    [[maybe_unused]] uint32_t size
)
{
}

void
Client::keyboardEnter(
    [[maybe_unused]] wl_keyboard* pKeyboard,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] wl_surface* pSurface,
    [[maybe_unused]] wl_array* pKeys
)
{
}

void
Client::keyboardLeave(
    [[maybe_unused]] wl_keyboard* pKeyboard,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] wl_surface* pSurface
)
{
}

void
Client::keyboardKey(
    [[maybe_unused]] wl_keyboard* pKeyboard,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] uint32_t time,
    [[maybe_unused]] uint32_t key,
    [[maybe_unused]] uint32_t state
)
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
    [[maybe_unused]] wl_keyboard* pKeyboard,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] uint32_t modsDepressed,
    [[maybe_unused]] uint32_t modsLatched,
    [[maybe_unused]] uint32_t modsLocked,
    [[maybe_unused]] uint32_t group
)
{
}

void
Client::keyboardRepeatInfo(
    [[maybe_unused]] wl_keyboard* pKeyboard,
    [[maybe_unused]] int32_t rate,
    [[maybe_unused]] int32_t delay
)
{
}

void
Client::pointerEnter(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] wl_surface* pSurface,
    [[maybe_unused]] wl_fixed_t surfaceX,
    [[maybe_unused]] wl_fixed_t surfaceY
)
{
}

void
Client::pointerLeave(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] wl_surface* pSurface
)
{
}

void
Client::pointerMotion(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t time,
    [[maybe_unused]] wl_fixed_t surfaceX,
    [[maybe_unused]] wl_fixed_t surfaceY
)
{
}

void
Client::pointerButton(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] uint32_t time,
    [[maybe_unused]] uint32_t button,
    [[maybe_unused]] uint32_t state
)
{
}

void
Client::pointerAxis(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t time,
    [[maybe_unused]] uint32_t axis,
    [[maybe_unused]] wl_fixed_t value
)
{
}

void
Client::pointerFrame(
    [[maybe_unused]] wl_pointer* pPointer
)
{
}

void
Client::pointerAxisSource(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t axisSource
)
{
}

void
Client::pointerAxisStop(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t time,
    [[maybe_unused]] uint32_t axis
)
{
}

void
Client::pointerAxisDiscrete(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t axis,
    [[maybe_unused]] int32_t discrete
)
{
}

void
Client::pointerAxisValue120(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t axis,
    [[maybe_unused]] int32_t value120
)
{
}

void
Client::pointerAxisRelativeDirection(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t axis,
    [[maybe_unused]] uint32_t direction
)
{
}

} /* namespace platform::wayland */
