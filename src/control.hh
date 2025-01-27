#pragma once

#include "frame.hh"

#include "adt/Arr.hh"
#include "adt/math.hh"

namespace control
{

constexpr int MAX_KEYBINDS = 128;
constexpr int MAX_KEY_VALUE = 255;
constexpr adt::f64 SPEED = 0.5;

struct Camera
{
    constexpr static const adt::math::V3 front {0, 0, 1};
    constexpr static const adt::math::V3 right {1, 0, 0};
    constexpr static const adt::math::V3 up {0, 1, 0};

    adt::math::V3 pos {};
    adt::math::V3 lastMove {};

    adt::math::M4 getTRM()
    {
        adt::f32 len = adt::math::V3Length(lastMove);
        if (len > 0)
        {
            return adt::math::M4TranslationFrom(
                -(pos += (adt::math::V3Norm(lastMove, len)*frame::g_dt*SPEED))
            );
        }

        return adt::math::M4TranslationFrom(-pos);
    }
};

enum REPEAT_KEY : adt::u8 { ONCE, REPEAT };

struct Keybind
{
    REPEAT_KEY eRepeat;
    adt::u8 key;
    void (*pfn)();
};

extern Camera g_camera;
extern bool g_aPressed[MAX_KEY_VALUE];
extern adt::Arr<Keybind, MAX_KEYBINDS> g_aKeybinds;
extern adt::Arr<Keybind, MAX_KEYBINDS> g_aModbinds; /* exec after g_aKeybinds */

void procKeys();

} /* namespace control */
