#pragma once

#include "adt/Array.hh"
#include "adt/math.hh"

#include "frame.hh"
#include "keys.hh"

namespace control
{

constexpr int MAX_KEYBINDS = 128;
constexpr int MAX_KEY_VALUE = 500;

constexpr adt::math::V3 CAMERA_FRONT {0.0f, 0.0f, 1.0f}; /* left hand */
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
    adt::f32 m_lastBoost = 1.0f;

    adt::f32 m_fov = 60.0f;

    /* */

    void
    updatePos()
    {
        adt::f32 len = adt::math::V3Length(m_lastMove);
        if (len > 0)
        {
            m_trm = m_view * adt::math::M4TranslationFrom(
                -(m_pos += (adt::math::V3Norm(m_lastMove, len)*(frame::g_dt)*m_speed*m_lastBoost))
            );
        }

        m_trm = m_view * adt::math::M4TranslationFrom(-m_pos);
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

enum class REPEAT : adt::u8 { ONCE, WHILE_DOWN };
enum class EXEC_ON : adt::u8 { PRESS, RELEASE };

struct Keybind
{
    REPEAT eRepeat {};
    EXEC_ON eExecOn {};
    MOD_STATE eMod {};
    adt::u32 key {};
    void (*pfn)() {};
};

extern Camera g_camera;
extern Mouse g_mouse;
extern bool g_abPrevPressed[MAX_KEY_VALUE];
extern bool g_abPressed[MAX_KEY_VALUE];
extern MOD_STATE g_ePressedMods;

extern adt::Array<Keybind, MAX_KEYBINDS> g_aKeybinds;

void procInput();

} /* namespace control */
