#pragma once

#include "adt/defer.hh"
#include "frame.hh"

#include "adt/Arr.hh"
#include "adt/math.hh"

namespace control
{

constexpr int MAX_KEYBINDS = 128;
constexpr int MAX_KEY_VALUE = 500;

constexpr adt::math::V3 CAMERA_FRONT {0.0f, 0.0f, 1.0f};
constexpr adt::math::V3 CAMERA_RIGHT {1.0f, 0.0f, 0.0f};
constexpr adt::math::V3 CAMERA_UP {0.0f, 1.0f, 0.0f};

struct Camera
{
    adt::math::M4 m_trm {};
    adt::math::M4 m_view {};

    adt::math::V3 m_front {};
    adt::math::V3 m_right {};

    adt::math::V3 m_pos {};
    adt::math::V3 m_lastMove {};

    adt::f32 m_yaw {};
    adt::f32 m_pitch {};
    adt::f32 m_roll {};

    adt::f32 m_sens {};
    adt::f32 m_speed {};

    /* */

    [[nodiscard]] adt::math::M4
    procMoveTRM()
    {
        defer( m_lastMove = {} );

        adt::f32 len = adt::math::V3Length(m_lastMove);
        if (len > 0)
        {
            return adt::math::M4TranslationFrom(
                -(m_pos += (adt::math::V3Norm(m_lastMove, len)*frame::g_dt*m_speed))
            );
        }

        return adt::math::M4TranslationFrom(-m_pos);
    }

    adt::math::V3
    getForwardVecNoY() const
    {
        return {m_front.x, 0.0f, m_front.z};
    }
};

/* mouse buttons are in the `g_aPressed` as `BTN_*` */
struct Mouse
{
    adt::math::V2 abs {};
    adt::math::V2 prevAbs {};

    adt::math::V2 rel {};
    adt::math::V2 prevRel {};
};

enum REPEAT_KEY : adt::u8 { ONCE, REPEAT };

struct Keybind
{
    REPEAT_KEY eRepeat;
    adt::u32 key;
    void (*pfn)();
};

extern Camera g_camera;
extern Mouse g_mouse;
extern bool g_aPrevPressed[MAX_KEY_VALUE];
extern bool g_aPressed[MAX_KEY_VALUE];
extern adt::Arr<Keybind, MAX_KEYBINDS> g_aKeybinds;
extern adt::Arr<Keybind, MAX_KEYBINDS> g_aModbinds; /* exec after g_aKeybinds */

void procInput();

} /* namespace control */
