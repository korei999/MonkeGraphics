#pragma once

#include "adt/String.hh"
#include "adt/Pool.hh"
#include "adt/enum.hh"

namespace ui
{

constexpr adt::f32 WIDTH = 120.0f;
constexpr adt::f32 HEIGHT = 42.0f;

struct Entry
{
    enum class TYPE : adt::u8
    {
        TEXT, ARROW_LIST, MENU,
    };

    /* */

    union
    {
        struct
        {
            adt::StringFixed<32> sfName {};
            adt::Vec<Entry> vEntries {};
            adt::ssize selectedI {};
        } menu;
        struct
        {
            adt::Vec<Entry> vEntries {};
            adt::ssize selectedI {};
            adt::math::V4 arrowColor {};
        } arrowList;
        struct
        {
            adt::StringFixed<32> sfText {};
        } text;
    };
    adt::math::V4 fgColor {};
    TYPE eType {};
};

struct Widget
{
    enum class FLAGS : adt::u8
    {
        NO_DRAW = 1,
        TITLE = 1 << 1,
        DRAG = 1 << 2,
    };

    /* */

    adt::Arena arena {};

    adt::StringFixed<16> sfName {};
    adt::StringFixed<32> sfTitle {};

    adt::Vec<Entry> vEntries {};

    adt::f32 x {};
    adt::f32 y {};
    adt::f32 width {}; /* -1 to expand automatically */
    adt::f32 height {}; /* -1 to expand automatically */

    adt::math::V4 bgColor {};

    FLAGS eFlags {};
};
ADT_ENUM_BITWISE_OPERATORS(Widget::FLAGS);

void init();
void updateState();
void destroy();

extern adt::Pool<Widget, 64> g_poolWidgets;
extern adt::MapManaged<adt::StringFixed<16>, adt::PoolHandle<Widget>> g_mapStringsToWidgetHandles;

} /* namespace ui */
