#pragma once

#include "adt/Arena.hh"
#include "adt/Pool.hh"
#include "adt/Vec.hh"
#include "adt/enum.hh"
#include "adt/mathDecl.hh"
#include "adt/Map.hh"

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

    struct Action
    {
        void (*pfn)(Entry* self, void* pArg) {};
        void* pArg {};
    };

    /* */

    union
    {
        struct
        {
            adt::StringFixed<32> sfName {};
            adt::Vec<Entry> vEntries {};
            adt::math::V4 selColor {};
            adt::math::V4 color {};
            adt::ssize selectedI {};
            Action onUpdate;
            Action onClick;
        } menu;
        struct
        {
            adt::Vec<Entry> vEntries {};
            adt::ssize selectedI {};
            adt::ssize prevSelectedI {};
            adt::math::V4 color {};
            adt::math::V4 arrowColor {};
            Action onUpdate {};
        } arrowList;
        struct
        {
            adt::StringFixed<32> sfText {};
            adt::math::V4 color {};
        } text;
    };
    TYPE eType {};
};

struct Widget
{
    enum class FLAGS : adt::u8
    {
        NO_DRAW = 1, /* skip on drawing */
        TITLE = 1 << 1, /* show title */
        DRAG = 1 << 2, /* allow dragging */
    };

    static constexpr adt::f32 AUTO_POS = -1.0f;

    /* */

    adt::Arena arena {};

    adt::StringFixed<16> sfName {};
    adt::StringFixed<32> sfTitle {};

    adt::Vec<Entry> vEntries {};

    adt::f32 x {};
    adt::f32 y {};
    adt::f32 width {}; /* -1 to expand automatically */
    adt::f32 height {}; /* -1 to expand automatically */
    adt::f32 grabWidth {}; /* recalculates each update */
    adt::f32 grabHeight {}; /* recalculates each update */

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
