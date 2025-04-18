#include "ui.hh"

#include "app.hh"
#include "common.hh"
#include "control.hh"
#include "game/game.hh"
#include "Model.hh"

using namespace adt;

namespace ui
{

struct ClickResult
{
    enum class FLAG : u8 { UNHANDLED, HANDLED, GRAB };

    /* */

    Offset off {};
    FLAG eFlag {};
};

static void dispatchAllOnUpdateActions();

static ClickResult clickArrowList(
    Widget* pWidget,
    Entry* pEntry,
    const f32 px,
    const f32 py,
    const Offset off
);

Pool<Widget, 64> g_poolWidgets;

static bool s_bGrabbed = false;
static Widget* s_pGrabbedWidget {};
static Widget s_dummyWidget {};
static bool s_bPressed = false;

int
Entry::height() const
{
    int height = 0;

    switch (m_eType)
    {
        case TYPE::TEXT:
        {
            height += 1;
        }
        break;

        case TYPE::ARROW_LIST:
        {
            int maxHeight = 0;
            for (const auto& ch : m_arrowList.vEntries)
                maxHeight = utils::max(ch.height(), maxHeight);
            height += maxHeight + 1; /* +1 for list name */
        };
        break;

        case TYPE::MENU:
        {
            for (const auto& ch : m_menu.vEntries)
                height += ch.height();
            ++height; /* skip menu name */
        }
        break;
    }

    return height;
}

void
Entry::dispatchOnUpdateActions()
{
    switch (m_eType)
    {
        case Entry::TYPE::ARROW_LIST:
        {
            auto& list = m_arrowList;
            for (auto& entry : list.vEntries) entry.dispatchOnUpdateActions();

            if (list.onUpdate.pfn) list.onUpdate.pfn(this, list.onUpdate.pArg);
        }
        break;

        case Entry::TYPE::MENU:
        {
            auto& menu = m_menu;
            if (menu.onUpdate.pfn) menu.onUpdate.pfn(this, menu.onUpdate.pArg);

            for (auto& child : menu.vEntries) child.dispatchOnUpdateActions();
        }
        break;

        case Entry::TYPE::TEXT: break;
    }
}

ssize
Entry::pushEntry(Arena* pArena, const Entry& entry)
{
    switch (m_eType)
    {
        case TYPE::MENU:
        return m_menu.vEntries.push(pArena, entry);
        break;

        case TYPE::ARROW_LIST:
        return m_arrowList.vEntries.push(pArena, entry);
        break;

        case TYPE::TEXT:
        ADT_ASSERT(false, "TEXT can't have entry Vec");
        break;
    }

    ADT_ASSERT(false, "bad path");

    return -1;
}

Entry
Entry::makeMenu(const Menu& menu)
{
    return Entry {
        .m_menu = menu,
        .m_eType = TYPE::MENU,
    };
}

Entry
Entry::makeArrowList(const ArrowList& arrowList)
{
    return Entry {
        .m_arrowList = arrowList,
        .m_eType = TYPE::ARROW_LIST,
    };
}

Entry
Entry::makeText(const Text& text)
{
    return Entry {
        .m_text = text,
        .m_eType = TYPE::TEXT,
    };
}

void
init()
{
    const auto& win = app::windowInst();

    /* FIXME: grabWidth and grabHeight restrict the clickable area. */
    {
        Widget widget {
            .arena = Arena {SIZE_8K},
            .sfTitle = "Entities",
            .x = WIDTH - 30.0f,
            .y = 1.0f,
            .width = Widget::AUTO_SIZE,
            .height = Widget::AUTO_SIZE,
            .border = 0.5f,
            .bgColor = math::V4From(colors::BLACK, 0.5f),
            .eFlags = Widget::FLAGS::TITLE | Widget::FLAGS::DRAG,
        };

        Vec<Entry> vListMenus {&widget.arena};

        {
            ssize entityI = 0;
            for (auto ent : game::g_poolEntities)
            {
                defer( ++entityI );

                if (ent.bNoDraw) continue;

                Entry entityListEntry = Entry::makeMenu({.sfName = ent.sfName});
                defer( vListMenus.push(&widget.arena, entityListEntry) );

                {
                    Entry animationsMenu = Entry::makeMenu({
                        .sfName = "Animations",
                        .onClick {
                            .pfn = +[](Entry* pSelf, i16 clickedI, void* pArg) -> void
                            {
                                auto entity = game::g_poolEntities[{int(reinterpret_cast<ssize>(pArg))}];
                                auto& rModel = Model::g_poolModels[{entity.modelI}];

                                if (control::g_abPressed[BTN_LEFT])
                                {
                                    rModel.m_animationUsedI = clickedI;
                                    pSelf->m_menu.selectedI = clickedI;
                                }
                                else if (control::g_abPressed[BTN_RIGHT])
                                {
                                    rModel.m_animationUsedI = -1;
                                    pSelf->m_menu.selectedI = -1;
                                }
                            },
                            .pArg = reinterpret_cast<void*>(entityI),
                        },
                    });
                    defer( entityListEntry.pushEntry(&widget.arena, animationsMenu) );

                    for (auto& animations : Model::g_poolModels[{ent.modelI}].m_vAnimations)
                    {
                        Entry animationText = Entry::makeText({.sfName = animations.sName});
                        animationsMenu.pushEntry(&widget.arena, animationText);
                    }
                }

                {
                    Entry positionsMenu = Entry::makeMenu({
                        .sfName = "Positions",
                        .onUpdate {
                            .pfn = +[](Entry* pSelf, void* pArg) -> void
                            {
                                auto entity = game::g_poolEntities[{int(reinterpret_cast<ssize>(pArg))}];

                                auto clPrint = [&](const StringView svFmt, auto arg, auto& rSfName) -> void
                                {
                                    char aBuff[64] {};
                                    const ssize n = print::toSpan(aBuff, svFmt, arg);
                                    rSfName = StringView{aBuff, n};
                                };

                                clPrint("x: {:.3}", entity.pos.x, pSelf->m_menu.vEntries[0].m_text.sfName);
                                clPrint("y: {:.3}", entity.pos.y, pSelf->m_menu.vEntries[1].m_text.sfName);
                                clPrint("z: {:.3}", entity.pos.z, pSelf->m_menu.vEntries[2].m_text.sfName);
                            },
                            .pArg = reinterpret_cast<void*>(entityI),
                        },
                    });

                    positionsMenu.pushEntry(&widget.arena, Entry::makeText({}));
                    positionsMenu.pushEntry(&widget.arena, Entry::makeText({}));
                    positionsMenu.pushEntry(&widget.arena, Entry::makeText({}));

                    entityListEntry.pushEntry(&widget.arena, positionsMenu);
                }
            }
        }

        Entry entityList = Entry::makeArrowList({
            .vEntries = vListMenus,
            .onUpdate {
                .pfn = +[](Entry* pSelf, void* pArg)
                {
                    auto clSetOutline = [&](ssize idx, const Opt<math::V4>& oColor) -> void
                    {
                        auto& rSel = pSelf->m_arrowList.vEntries[idx];
                        const ssize entityI = reinterpret_cast<ssize>(rSel.m_menu.vEntries[0].m_menu.onClick.pArg);
                        auto entity = game::g_poolEntities[{int(entityI)}];
                        auto& rModel = Model::g_poolModels[{entity.modelI}];
                        rModel.m_oOutlineColor = oColor;
                    };

                    clSetOutline(pSelf->m_arrowList.prevSelectedI, {});
                    clSetOutline(pSelf->m_arrowList.selectedI, math::V4From(colors::WHITESMOKE, 0.75f));
                },
                .pArg = nullptr,
            },
        });

        widget.vEntries.push(&widget.arena, entityList);

        [[maybe_unused]] auto h = g_poolWidgets.make(widget);
    }

    dispatchAllOnUpdateActions();
}

static ClickResult
clickMenu(
    Widget* pWidget,
    Entry* pEntry,
    const f32 px,
    const f32 py,
    const Offset off
)
{
    ADT_ASSERT(pEntry->m_eType == Entry::TYPE::MENU, " ");

    ClickResult ret {};

    Offset thisOff {0, 1};

    auto& menu = pEntry->m_menu;

    for (auto& child : menu.vEntries)
    {
        const int childHeight = child.height();
        defer( thisOff.y += childHeight );

        switch (child.m_eType)
        {
            case Entry::TYPE::TEXT:
            {
                if (common::AABB(px, py,
                        pWidget->x + off.x + thisOff.x,
                        pWidget->y + off.y + thisOff.y,
                        child.m_text.sfName.size(), 1
                    )
                )
                {
                    const ssize idx = menu.vEntries.idx(&child);

                    if (menu.onClick.pfn)
                    {
                        menu.onClick.pfn(
                            pEntry,
                            static_cast<i16>(idx),
                            menu.onClick.pArg
                        );
                    }

                    ret.eFlag = ClickResult::FLAG::HANDLED;
                    goto GOTO_done;
                }
            }
            break;

            case Entry::TYPE::ARROW_LIST:
            {
                if (py < pWidget->y + off.y + thisOff.y + childHeight &&
                    py >= pWidget->y + off.y + thisOff.y &&
                    px >= pWidget->x + off.x + thisOff.x
                )
                {
                    ClickResult res = clickArrowList(
                        pWidget, &child, px, py, {off.x + thisOff.x, off.y + thisOff.y}
                    );
                    ret.eFlag = res.eFlag;
                    goto GOTO_done;
                }
            }
            break;

            case Entry::TYPE::MENU:
            {
                if (py < pWidget->y + off.y + thisOff.y + childHeight + 1 &&
                    py >= pWidget->y + off.y + thisOff.y &&
                    px >= pWidget->x + off.x + thisOff.x
                )
                {
                    ClickResult res = clickMenu(
                        pWidget, &child, px, py, {thisOff.x + off.x + 2, thisOff.y + off.y}
                    );
                    ret.eFlag = res.eFlag;

                    if (ret.eFlag == ClickResult::FLAG::HANDLED)
                        goto GOTO_done;
                }
            }
            break;

            default: ADT_ASSERT(false, "invalid path");
        }
    }

GOTO_done:

    ret.off = thisOff;
    return ret;
}

static ClickResult
clickArrowList(Widget* pWidget, Entry* pEntry, const f32 px, const f32 py, const Offset off)
{
    ADT_ASSERT(pEntry, " ");
    ADT_ASSERT(pEntry->m_eType == Entry::TYPE::ARROW_LIST, " ");

    ClickResult ret {};
    Offset thisOff {0, 0};

    auto& list = pEntry->m_arrowList;

    if (common::AABB(px, py, pWidget->x + off.x, pWidget->y + off.y, 2, 1))
    {
        list.prevSelectedI = list.selectedI;

        if (control::g_abPressed[BTN_LEFT])
            list.selectedI = utils::cycleForward(list.selectedI, list.vEntries.size());
        else if (control::g_abPressed[BTN_RIGHT])
            list.selectedI = utils::cycleBackward(list.selectedI, list.vEntries.size());

        ret.eFlag = ClickResult::FLAG::HANDLED;

        return ret;
    }

    switch (list.vEntries[list.selectedI].m_eType)
    {
        case Entry::TYPE::MENU:
        {
            ++thisOff.y;
            ClickResult res = clickMenu(
                pWidget, &list.vEntries[list.selectedI], px, py, {thisOff.x + off.x + 2, thisOff.y + off.y}
            );
            thisOff.y += res.off.y;
            ret = res;
        }
        break;

        case Entry::TYPE::ARROW_LIST:
        {
        }
        break;

        case Entry::TYPE::TEXT:
        {
        }
        break;
    }

    return ret;
}

static ClickResult
clickWidget(Widget* pWidget, const f32 px, const f32 py, const Offset off)
{
    ADT_ASSERT(pWidget, " ");

    bool bHandled = false;
    ClickResult ret {};
    Offset thisOff {0, 0};

    if (px + pWidget->border >= pWidget->x && px - pWidget->border < pWidget->x + pWidget->priv.grabWidth &&
        py + pWidget->border >= pWidget->y && py - pWidget->border < pWidget->y + pWidget->priv.grabHeight
    )
    {
        if (bool(pWidget->eFlags & Widget::FLAGS::TITLE))
            ++thisOff.y;

        for (Entry& entry : pWidget->vEntries)
        {
            switch (entry.m_eType)
            {
                case Entry::TYPE::ARROW_LIST:
                {
                    if (py < pWidget->y + (pWidget->priv.grabHeight - off.y) + 1)
                    {
                        s_bPressed = true;

                        ClickResult res = clickArrowList(
                            pWidget, &entry, px, py, {off.x + thisOff.x, off.y + thisOff.y}
                        );

                        bHandled = res.eFlag == ClickResult::FLAG::HANDLED;

                        thisOff.y += res.off.y;
                    }
                }
                break;

                case Entry::TYPE::MENU:
                {
                    s_bPressed = true;

                    ClickResult res = clickMenu(
                        pWidget, &entry, px, py, {off.x + thisOff.x + 2, off.y + thisOff.y}
                    );

                    bHandled = res.eFlag == ClickResult::FLAG::HANDLED;

                    thisOff.y += res.off.y;
                }
                break;

                case Entry::TYPE::TEXT: break;
            }

            if (bHandled) break;
        }

        if (!bHandled && bool(pWidget->eFlags & Widget::FLAGS::DRAG))
        {
            s_pGrabbedWidget = pWidget;
            s_bGrabbed = true;
        }
    }

    if (!bHandled)
    {
        /* Prevent from hover activations, when first click was into nothing. */
        s_bPressed = true;
    }

    ret.off = off;
    return ret;
}

static void
dispatchAllOnUpdateActions()
{
    for (Widget& widget : g_poolWidgets)
    {
        for (auto& entry : widget.vEntries)
            entry.dispatchOnUpdateActions();
    }
}

void
updateState()
{
    dispatchAllOnUpdateActions();

    const auto& win = app::windowInst();
    auto& mouse = control::g_mouse;

    if (win.m_bPointerRelativeMode) return;

    mouse.abs = math::V2{win.m_pointerSurfaceX, win.m_pointerSurfaceY};

    const math::V2 delta = mouse.abs - mouse.prevAbs;
    mouse.prevAbs = mouse.abs;

    bool bNoClick =
        !(control::g_abPressed[BTN_LEFT] ||
        control::g_abPressed[BTN_RIGHT] ||
        control::g_abPressed[BTN_MIDDLE]
    );

    if (bNoClick)
    {
        s_bPressed = false;
        s_bGrabbed = false;
        s_pGrabbedWidget = &s_dummyWidget;
        return;
    }

    if (s_bPressed && !s_bGrabbed) return;

    const f32 widthFactor = 1.0f/(win.m_winWidth * (1.0f/WIDTH));
    const f32 heightFactor = 1.0f/(win.m_winHeight * (1.0f/HEIGHT));

    const f32 px = mouse.abs.x * widthFactor;
    const f32 py = mouse.abs.y * heightFactor;

    const f32 dx = delta.x * widthFactor;
    const f32 dy = delta.y * heightFactor;

    if (s_bGrabbed)
    {
        s_pGrabbedWidget->x += dx;
        s_pGrabbedWidget->y += dy;
        return;
    }

    /* TODO: rework into a rooted tree. */
    for (Widget& widget : g_poolWidgets)
    {
        ClickResult res = clickWidget(&widget, px, py, {0, 0});
        if (res.eFlag == ClickResult::FLAG::GRAB)
            break;
    }
}

void
destroy()
{
    for (auto& widget : g_poolWidgets)
        widget.arena.freeAll();
}

} /* namespace ui */
