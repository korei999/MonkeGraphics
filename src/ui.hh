#pragma once

#include "adt/String.hh"
#include "adt/Pool.hh"
#include "adt/enum.hh"

namespace ui
{

struct Rect
{
    enum class FLAGS : adt::u8
    {
        DRAW = 1,
        TITLE = 1 << 1,
    };

    /* */

    adt::StringFixed<16> sfName {};
    adt::String sTitle {};

    adt::f32 x {};
    adt::f32 y {};
    int width {};
    int height {};

    adt::math::V4 color {};

    FLAGS eFlags {};
};
ADT_ENUM_BITWISE_OPERATORS(Rect::FLAGS);

void init();

extern adt::Pool<Rect, 64> g_poolRects;
extern adt::MapManaged<adt::StringFixed<16>, adt::PoolHandle<Rect>> g_mapStringsToRectHandles;

} /* namespace ui */
