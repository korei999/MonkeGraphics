/* quick and dirty ui widgets */

#pragma once

#include "colors.hh"

#include "adt/Pair.hh"
#include "adt/Arena.hh"
#include "adt/Pool.hh"
#include "adt/Vec.hh"
#include "adt/enum.hh"
#include "adt/mathDecl.hh"

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
        void (*pfn)(Entry* pSelf, void* pArg) {};
        void* pArg {};
    };

    struct Menu
    {
        adt::StringFixed<32> sfName {};
        adt::Vec<Entry> vEntries {};
        adt::math::V4 selColor = adt::math::V4From(colors::GREEN, 1.0f);
        adt::math::V4 color = adt::math::V4From(colors::WHITESMOKE, 1.0f);
        adt::ssize selectedI = -1;
        Action onUpdate {};
        Action onClick {};
    };

    struct ArrowList
    {
        adt::StringFixed<32> sfName {};
        adt::Vec<Entry> vEntries {};
        adt::ssize selectedI = 0;
        adt::ssize prevSelectedI = 0;
        adt::math::V4 color = adt::math::V4From(colors::WHITESMOKE, 1.0f);
        adt::math::V4 arrowColor = adt::math::V4From(colors::CYAN, 1.0f);
        Action onUpdate {};
    };

    struct Text
    {
        adt::StringFixed<32> sfName {};
        adt::math::V4 color = adt::math::V4From(colors::WHITESMOKE, 1.0f);
    };

    /* */

    union
    {
        Menu m_menu;
        ArrowList m_arrowList;
        Text m_text;
    };
    TYPE m_eType {};

    /* */

    [[nodiscard]] int entryHeight() const;
};

struct Widget
{
    enum class FLAGS : adt::u8
    {
        NO_DRAW = 1, /* skip on drawing */
        TITLE = 1 << 1, /* show title */
        DRAG = 1 << 2, /* allow dragging */
    };

    static constexpr adt::f32 AUTO_SIZE = -1.0f;

    /* */

    adt::Arena arena {};

    adt::StringFixed<32> sfTitle {};

    adt::Vec<Entry> vEntries {};

    adt::f32 x {};
    adt::f32 y {};
    adt::f32 width = AUTO_SIZE; /* (AUTO_SIZE)-1 to expand automatically */
    adt::f32 height = AUTO_SIZE; /* (AUTO_SIZE)-1 to expand automatically */
    adt::f32 border {};

    adt::math::V4 bgColor = adt::math::V4From(colors::BLACK, 0.5f);

    FLAGS eFlags {};

    /* recalculates each update */
    struct
    {
        adt::f32 grabWidth {};
        adt::f32 grabHeight {};
    } priv {};
};
ADT_ENUM_BITWISE_OPERATORS(Widget::FLAGS);

struct Offset
{
    int x {};
    int y {};
};

void init();
void updateState();
void destroy();

extern adt::Pool<Widget, 64> g_poolWidgets;

} /* namespace ui */

namespace adt::print
{

static ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const ::ui::Offset x)
{
    return formatToContext(ctx, fmtArgs, Pair{x.x, x.y});
}

} /* namespace adt::print */
