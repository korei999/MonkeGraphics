/* quick and dirty ui widgets */

#pragma once

#include "colors.hh"

#include "adt/Pair.hh"
#include "adt/Arena.hh"
#include "adt/Pool.hh"
#include "adt/Vec.hh"
#include "adt/enum.hh"
#include "adt/math.hh"

namespace ui
{

constexpr adt::f32 WIDTH = 120.0f;
constexpr adt::f32 HEIGHT = 42.0f;

struct Widget;

struct Entry
{
    enum class TYPE : adt::u8
    {
        TEXT, ARROW_LIST, MENU,
    };

    struct Menu;
    struct ArrowList;

    template<typename T>
    struct Action
    {
        void (*pfn)(T* pSelf, void* pArg) {};
        void* pArg {};
    };

    struct MenuClickAction
    {
        /* TODO: why even bother with pArg? */
        void (*pfn)(Menu* pSelf, adt::i16 clickedI, void* pArg) {};
        void* pArg {};
    };

    struct Menu
    {
        adt::StringFixed<32> sfName {};
        adt::Vec<Entry> vEntries {};
        adt::math::V4 selColor = adt::math::V4From(colors::GREEN, 1.0f);
        adt::math::V4 color = adt::math::V4From(colors::WHITESMOKE, 1.0f);
        adt::i16 selectedI = -1;
        Action<Menu> onUpdate {};
        MenuClickAction onClick {};
    };

    struct ArrowList
    {
        adt::StringFixed<32> sfName {};
        adt::Vec<Entry> vEntries {};
        adt::isize selectedI = 0;
        adt::isize prevSelectedI = 0;
        adt::math::V4 color = adt::math::V4From(colors::WHITESMOKE, 1.0f);
        adt::math::V4 arrowColor = adt::math::V4From(colors::CYAN, 1.0f);
        Action<ArrowList> onUpdate {};
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

    [[nodiscard]] int height() const;
    void dispatchOnUpdateActions();
    adt::isize pushEntry(adt::Arena* pArena, const Entry& entry);

    /* */

    static Entry makeMenu(const Menu& menu);
    static Entry makeArrowList(const ArrowList& arrowList);
    static Entry makeText(const Text& text);
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

using WidgetPool = adt::Pool<Widget, 64>;

extern WidgetPool g_poolWidgets;

} /* namespace ui */

namespace adt::print
{

inline isize
formatToContext(Context ctx, FormatArgs fmtArgs, const ::ui::Offset x)
{
    return formatToContext(ctx, fmtArgs, Pair{x.x, x.y});
}

} /* namespace adt::print */
