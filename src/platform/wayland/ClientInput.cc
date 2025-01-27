#include "Client.hh"

#include "app.hh"
#include "control.hh"

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
    if (key < utils::size(control::g_aPressed))
        control::g_aPressed[key] = state;
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
    auto& win = app::window();
    if (!win.m_bPointerRelativeMode)
    {
        win.m_pointerSurfaceX = static_cast<f32>(wl_fixed_to_double(surfaceX));
        win.m_pointerSurfaceY = static_cast<f32>(win.m_winHeight) - static_cast<f32>(wl_fixed_to_double(surfaceY));
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
    if (button < utils::size(control::g_aPressed))
        control::g_aPressed[button] = state;
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

void
Client::relativePointerMotion(
    [[maybe_unused]] zwp_relative_pointer_v1* pRelPointerV1,
    [[maybe_unused]] uint32_t utimeHi,
    [[maybe_unused]] uint32_t utimeLo,
    [[maybe_unused]] wl_fixed_t dx,
    [[maybe_unused]] wl_fixed_t dy,
    [[maybe_unused]] wl_fixed_t dxUnaccel,
    [[maybe_unused]] wl_fixed_t dyUnaccel
)
{
    if (m_bPointerRelativeMode)
    {
        m_relMotionX = static_cast<f32>(wl_fixed_to_double(dxUnaccel));
        m_relMotionY = static_cast<f32>(wl_fixed_to_double(dyUnaccel));
    }
}

} /* namespace platform::wayland */
