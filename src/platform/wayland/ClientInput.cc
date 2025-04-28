#include "Client.hh"

#include "app.hh"
#include "control.hh"

#include "adt/logs.hh"

using namespace adt;

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
    m_bKeyboardUnfocused = false;

    if (m_bRestoreLockedPointer)
    {
        m_bRestoreLockedPointer = false;
        enableRelativeMode();
    }
}

void
Client::keyboardLeave(
    [[maybe_unused]] wl_keyboard* pKeyboard,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] wl_surface* pSurface
)
{
    m_bKeyboardUnfocused = true;

    if (m_bPointerUnfocused)
        memset(control::g_abPressed, 0, sizeof(control::g_abPressed));

    if (m_bPointerRelativeMode)
    {
        m_bRestoreLockedPointer = true;
        disableRelativeMode();
    }
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
    if (key < utils::size(control::g_abPressed))
        control::g_abPressed[key] = state;
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
    control::g_ePressedMods = static_cast<MOD_STATE>(modsDepressed);
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
    m_bPointerUnfocused = false;
    m_lastPointerEnterSerial = serial;

    if (m_bPointerRelativeMode)
    {
        hideCursor(true);
    }
}

void
Client::pointerLeave(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t serial,
    [[maybe_unused]] wl_surface* pSurface
)
{
    m_bPointerUnfocused = true;

    /*if (m_bKeyboardUnfocused)*/
    /*    memset(control::g_abPressed, 0, sizeof(control::g_abPressed));*/
}

void
Client::pointerMotion(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t time,
    [[maybe_unused]] wl_fixed_t surfaceX,
    [[maybe_unused]] wl_fixed_t surfaceY
)
{
    auto& win = app::windowInst();

    if (!win.m_bPointerRelativeMode)
    {
        win.m_pointerSurfaceX = static_cast<f32>(wl_fixed_to_double(surfaceX));
        win.m_pointerSurfaceY = static_cast<f32>(wl_fixed_to_double(surfaceY));
    }
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
    if (button < utils::size(control::g_abPressed))
        control::g_abPressed[button] = state;
}

void
Client::pointerAxis(
    [[maybe_unused]] wl_pointer* pPointer,
    [[maybe_unused]] uint32_t time,
    [[maybe_unused]] uint32_t axis,
    [[maybe_unused]] wl_fixed_t value
)
{
    if (axis == WL_POINTER_AXIS_SOURCE_WHEEL)
    {
        m_atomVertWheel.store(int(wl_fixed_to_double(value)), atomic::ORDER::RELEASE);
    }
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

void
Client::relativePointerMotion(
    [[maybe_unused]] zwp_relative_pointer_v1* pRelPointerV1,
    [[maybe_unused]] uint32_t utimeHi,
    [[maybe_unused]] uint32_t utimeLo,
    [[maybe_unused]] wl_fixed_t dx,
    [[maybe_unused]] wl_fixed_t dy,
    wl_fixed_t dxUnaccel,
    wl_fixed_t dyUnaccel
)
{
    if (m_bPointerRelativeMode)
    {
        m_relMotionX += static_cast<f32>(wl_fixed_to_double(dxUnaccel));
        m_relMotionY += static_cast<f32>(wl_fixed_to_double(dyUnaccel));
    }
}

void
Client::decorationConfigure(
    [[maybe_unused]] zxdg_toplevel_decoration_v1* pZXdgToplevelDecorationV1,
    [[maybe_unused]] uint32_t mode
)
{
    LOG_WARN("decorationConfigure()\n");
}

} /* namespace platform::wayland */
